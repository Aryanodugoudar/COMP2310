#include "csapp/csapp.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// You may assume that all requests sent to a node are less than this length
#define REQUESTLINELEN 128
#define HOSTNAME "localhost"

// Cache related constants
#define MAX_OBJECT_SIZE 512 // object here refers to the posting list or result list being cached
#define MAX_CACHE_SIZE MAX_OBJECT_SIZE * 128

/* This struct contains all information needed for each node */
typedef struct node_info
{
  int node_id;     // node number
  int port_number; // port number
  int listen_fd;   // file descriptor of socket the node is using
} node_info;

/* Variables that all nodes will share */

// Port number of the parent process. After each node has been spawned, it
// attempts to connect to this port to send the parent the request for it's own
// section of the database.
int PARENT_PORT = 0;

// Number of nodes that were created. Must be between 1 and 8 (inclusive).
int TOTAL_NODES = 8;

// A dynamically allocated array of TOTAL_NODES node_info structs.
// The parent process creates this and populates it's values so when it creates
// the nodes, they each know what port number the others are using.
node_info *NODES = NULL;

/* ------------  Variables specific to each child process / node ------------ */

// After forking each node (one child process) changes the value of this variable
// to their own node id.
// Note that node ids start at 0. If you're just implementing the single node
// server this will be set to 0.
int NODE_ID = -1;

// Each node will fill this struct in with it's own portion of the database.
database partition = {NULL, 0, NULL};

/** @brief Called by a child process (node) when it wants to request its partition
 *         of the database from the parent process. This will be called ONCE by
 *         each node in the "digest" phase.
 *
 *  @todo  Implement this function. This function will need to:
 *         - Connect to the parent process. HINT: the port number to use is
 *           stored as an int in PARENT_PORT.
 *         - Send a request line to the parent. The request needs to be a string
 *           of the form "<nodeid>\n" (the ID of the node followed by a newline)
 *         - Read the response of the parent process. The response will start
 *           with the size of the partition followed by a newline. After the
 *           newline character, the next size bytes of the response will be this
 *           node's partition of the database.
 *         - Set the global partition variable.
 */
void request_partition(void)
{
  int parent_connfd;
  char request_line[REQUESTLINELEN];
  char response_line[REQUESTLINELEN];
  size_t partition_size;
  rio_t rio;

  char port[REQUESTLINELEN];
  port_number_to_str(PARENT_PORT, port);
  parent_connfd = Open_clientfd(HOSTNAME, port);
  Rio_readinitb(&rio, parent_connfd);
  snprintf(request_line, REQUESTLINELEN, "%d\n", NODE_ID);

  Rio_writen(parent_connfd, request_line, strlen(request_line));
  Rio_readlineb(&rio, response_line, MAXLINE);
  sscanf(response_line, "%lu", &partition_size);

  partition.m_ptr = (char *)malloc(partition_size);

  partition.db_size = partition_size;

  Rio_readnb(&rio, partition.m_ptr, partition_size);

  build_hash_table(&partition);

  Close(parent_connfd);
}

/** @brief The main server loop for a node. This will be called by a node after
 *         it has finished the digest phase. The server will run indefinitely,
 *         responding to requests. Each request is a single line.
 *
 *  @note  The parent process creates the listening socket that the node should
 *         use to accept incoming connections. This file descriptor is stored in
 *         NODES[NODE_ID].listen_fd.
 */
void node_serve(void)
{
  int listenfd = NODES[NODE_ID].listen_fd;
  struct sockaddr_storage clientaddr2;

  int n;
  int m;

  while (1)
  {
    rio_t rio2;
    char request_line2[MAXLINE];
    socklen_t clientlen = sizeof(struct sockaddr_storage);

    int connfd = Accept(listenfd, (SA *)&clientaddr2, &clientlen);

    Rio_readinitb(&rio2, connfd);

    while (Rio_readlineb(&rio2, request_line2, MAXLINE) > 0)
    {

      if (strlen(request_line2) == 0)
        break;

      request_line2[strcspn(request_line2, "\n")] = '\0';
      char *key1 = strtok(request_line2, " ");
      char *key2 = strtok(NULL, " ");
      char response[MAXLINE];

      char *index1 = find_entry(&partition, key1);
      char *index2 = (key2 != NULL) ? find_entry(&partition, key2) : NULL;

      int node_id1 = find_node(key1, TOTAL_NODES);
      int node_id2 = key2 == NULL ? NODE_ID : find_node(key2, TOTAL_NODES);
      size_t n = 0;

      if (node_id1 != NODE_ID && key2 == NULL)
      {

        // remote lookup
        char port[PORT_STRLEN], request_line[REQUESTLINELEN];
        rio_t rio_3;
        port_number_to_str(NODES[node_id1].port_number, port);
        int connfd2 = Open_clientfd(HOSTNAME, port);
        snprintf(request_line, REQUESTLINELEN, "%s\n", key1);
        Rio_writen(connfd2, request_line, REQUESTLINELEN);

        Rio_readinitb(&rio_3, connfd2);
        n = Rio_readlineb(&rio_3, response, MAXLINE);

        Close(connfd2);
      }

      if (index1 == NULL && node_id1 == NODE_ID)
      {
        // local lookup has not found key
        n = snprintf(response, MAXLINE, "%s not found\n", key1);
      }

      if (index1 != NULL && key2 == NULL && node_id1 == NODE_ID)
      {
        // local lookup
        n = entry_to_str(index1, response, MAXLINE);
      }

      char mix[MAXLINE];

      if (key2 != NULL && index2 == NULL && node_id2 == NODE_ID)
      {
        // intersection failed since key2 is not found
        n += snprintf(response + n, MAXLINE, "%s not found\n", key2);
      }

      if (key2 != NULL && index1 != NULL && index2 != NULL && node_id1 == NODE_ID && node_id2 == NODE_ID)
      {
        // local interseciton
        value_array *intersect = get_intersection(get_value_array(index1), get_value_array(index2));

        value_array_to_str(intersect, mix, MAXLINE);
        n = snprintf(response, MAXLINE, "%s,%s%s", key1, key2, mix);
      }

      if (key2 != NULL && index1 != NULL && index2 != NULL && node_id1 != NODE_ID && node_id2 != NODE_ID)
      {
        // remote intersection
        char port[PORT_STRLEN], request_line[REQUESTLINELEN];
        value_array *va1, *va2, *va3;
        rio_t rio_3;
        int connfd2;

        // remote lookup for key1
        port_number_to_str(NODES[node_id1].port_number, port);
        connfd2 = Open_clientfd(HOSTNAME, port);
        snprintf(request_line, REQUESTLINELEN, "%s\n", key1);
        Rio_writen(connfd2, request_line, REQUESTLINELEN);
        Rio_readinitb(&rio_3, connfd2);
        n = Rio_readlineb(&rio_3, response, MAXLINE);
        va1 = create_value_array(response);
        Close(connfd2);

        // remote lookup for key2
        port_number_to_str(NODES[node_id2].port_number, port);
        connfd2 = Open_clientfd(HOSTNAME, port);
        snprintf(request_line, REQUESTLINELEN, "%s\n", key2);
        Rio_writen(connfd2, request_line, REQUESTLINELEN);
        Rio_readinitb(&rio_3, connfd2);
        n = Rio_readlineb(&rio_3, response, MAXLINE);
        va2 = create_value_array(response);
        Close(connfd2);

        if (va1 == NULL)
          n = snprintf(response, MAXLINE, "%s not found\n", key1);

        if (va2 == NULL)
          n += snprintf(response + n, MAXLINE, "%s not found\n", key2);

        if (va1 != NULL && va2 != NULL)
        {
          va3 = get_intersection(va1, va2);
          value_array_to_str(va3, mix, MAXLINE);
          snprintf(response, MAXLINE, "%s,%s%s", key1, key2, mix);
          free(va3);
        }

        if (va1 != NULL)
          free(va1);
        if (va2 != NULL)
          free(va2);
      }

      Rio_writen(connfd, response, strlen(response));
    }

    Close(connfd);
  }
}

/** @brief Called after a child process is forked. Initialises all information
 *         needed by an individual node. It then calls request_partition to get
 *         the database partition that belongs to this node (the digest phase).
 *         It then calls node_serve to begin responding to requests (the serve
 *         phase). Since the server is designed to run forever (unless forcibly
 *         terminated) this function should not return.
 *
 *  @param node_id Value between 0 and TOTAL_NODES-1 that represents which node
 *         number this is. The global NODE_ID variable will be set to this value
 */
void start_node(int node_id)
{
  NODE_ID = node_id;

  // close all listen_fds except the one that this node should use.
  for (int n = 0; n < TOTAL_NODES; n++)
  {
    if (n != NODE_ID)
      Close(NODES[n].listen_fd);
  }

  request_partition();
  node_serve();
}

/** ----------------------- PARENT PROCESS FUNCTIONS ----------------------- **/

/* The functions below here are for the initial parent process to use (spawning
 * child processes, partitioning the database, etc).
 * You do not need to modify this code.
 */

/** @brief  Tries to create a listening socket on the port that start_port
 *          points to. If it cannot use that port, it will subsequently try
 *          increasing port numbers until it successfully creates a listening
 *          socket, or it has run out of valid ports. The value at start_port is
 *          set to the port_number the listening socket was opened on. The file
 *          descriptor of the listening socket is returned.
 *
 *  @param  start_port The value that start_port points to is used as the first
 *          port to try. When the function returns, the value is updated to the
 *          port number that the listening socket can use.
 *  @return The file descriptor of the listening socket that was created, or -1
 *          if no listening socket has been created.
 */
int get_listenfd(int *start_port)
{
  char portstr[PORT_STRLEN];
  int port, connfd;
  for (port = *start_port; port < MAX_PORTNUM; port++)
  {
    port_number_to_str(port, portstr);
    connfd = open_listenfd(portstr);
    if (connfd != -1)
    { // found a port to use
      *start_port = port;
      return connfd;
    }
  }
  return -1;
}

/** @brief  Called by the parent to handle a single request from a node for its
 *          partition of the database.
 *
 *  @param  db The database that will be partitioned.
 *  @param  connfd The connected file descriptor to read the request (a node id)
 *          from. The partition of the database is written back in response.
 *  @return If there is an error in the request returns -1. Otherwise returns 0.
 */
int parent_handle_request(database *db, int connfd)
{
  char request[REQUESTLINELEN];
  char responseline[REQUESTLINELEN];
  char *response;
  int node_id;
  ssize_t rl;
  size_t partition_size = 0;
  if ((rl = read(connfd, request, REQUESTLINELEN)) < 0)
  {
    fprintf(stderr, "parent_handle_request read error: %s\n", strerror(errno));
    return -1;
  }
  sscanf(request, "%d", &node_id);
  if ((node_id < 0) || (node_id >= TOTAL_NODES))
  {
    response = "Invalid Request.\n";
    partition_size = strlen(response);
  }
  else
  {
    response = get_partition(db, TOTAL_NODES, node_id, &partition_size);
  }
  snprintf(responseline, REQUESTLINELEN, "%lu\n", partition_size);
  rl = write(connfd, responseline, strlen(responseline));
  rl = write(connfd, response, partition_size);
  return 0;
}

/** Called by the parent process to load in the database, and wait for the child
 *  nodes it created to send a message requesting their portion of the database.
 *  After it has received the same number of requests as nodes, it unmaps the
 *  database.
 *
 *  @param db_path path to the database file being loaded in. It is assumed that
 *         the entries contained in this file are already sorted in alphabetical
 *         order.
 */
void parent_serve(char *db_path, int parent_connfd)
{
  // The parent doesn't need to create/populate the hash table.
  database *db = load_database(db_path);
  struct sockaddr_storage clientaddr;
  socklen_t clientlen = sizeof(clientaddr);
  int connfd = 0;
  int requests = 0;

  while (requests < TOTAL_NODES)
  {
    connfd = accept(parent_connfd, (SA *)&clientaddr, &clientlen);
    parent_handle_request(db, connfd);
    Close(connfd);
    requests++;
  }
  // Parent has now finished it's job.
  Munmap(db->m_ptr, db->db_size);
}

/** @brief Called after the parent has finished sending each node its partition
 *         of the database. The parent waits in a loop for any child processes
 *         (nodes) to terminate, and prints to stderr information about why the
 *         child process terminated.
 */
void parent_end()
{
  int stat_loc;
  pid_t pid;
  while (1)
  {
    pid = wait(&stat_loc);
    if (pid < 0 && (errno == ECHILD))
      break;
    else
    {
      if (WIFEXITED(stat_loc))
        fprintf(stderr, "Process %d terminated with exit status %d\n", pid, WEXITSTATUS(stat_loc));
      else if (WIFSIGNALED(stat_loc))
        fprintf(stderr, "Process %d terminated by signal %d\n", pid, WTERMSIG(stat_loc));
    }
  }
}

int main(int argc, char const *argv[])
{
  int start_port;    // port to begin search
  int parent_connfd; // parent listens here to handle distributing database
  int n_connfd;
  pid_t pid;

  if (argc != 4)
  {
    fprintf(stderr, "usage: %s [num_nodes] [starting_port] [name_of_file]\n", argv[0]);
    exit(1);
  }

  sscanf(argv[1], "%d", &TOTAL_NODES);
  sscanf(argv[2], "%d", &start_port);

  if (TOTAL_NODES < 1 || (TOTAL_NODES > 8))
  {
    fprintf(stderr, "Invalid node number given.\n");
    exit(1);
  }
  else if ((start_port < 1024) || start_port >= (MAX_PORTNUM - TOTAL_NODES))
  {
    fprintf(stderr, "Invalid starting port given.\n");
    exit(1);
  }

  NODES = calloc(TOTAL_NODES, sizeof(node_info));
  parent_connfd = get_listenfd(&start_port);
  PARENT_PORT = start_port;

  for (int n = 0; n < TOTAL_NODES; n++)
  {
    start_port++; // start search at previously assigned port + 1
    n_connfd = get_listenfd(&start_port);
    if (n_connfd < 0)
    {
      fprintf(stderr, "get_listenfd error\n");
      exit(1);
    }
    NODES[n].listen_fd = n_connfd;
    NODES[n].node_id = n;
    NODES[n].port_number = start_port;
  }

  // Begin forking all child processes.
  for (int n = 0; n < TOTAL_NODES; n++)
  {
    if ((pid = Fork()) == 0)
    { // child process
      Close(parent_connfd);
      start_node(n);
      exit(1);
    }
    else
    {
      node_info node = NODES[n];
      fprintf(stderr, "NODE %d [PID: %d] listening on port %d\n", n, pid, node.port_number);
    }
  }

  // Parent closes all fd's that belong to it's children
  for (int n = 0; n < TOTAL_NODES; n++)
    Close(NODES[n].listen_fd);

  // Parent can now begin waiting for children to send messages to contact.
  parent_serve((char *)argv[3], parent_connfd);
  Close(parent_connfd);

  parent_end();

  return 0;
}