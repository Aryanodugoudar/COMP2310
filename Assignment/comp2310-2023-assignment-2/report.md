# <p style="text-align: center;">COMP2310 Assignment 1</p> 
##### <p style="text-align: center;">Aryan Odugoudar - u7689173</p>

## Introduction

This report discusses my implementation of `COMP2310 Assignment 2 :Distributed Web Search` , The system comprises client and server nodes, where servers handle partitioned data and respond to queries.The assignment's specifications guided this implementation.

## 1. System Architecture:
- The system consists of client nodes generating search queries and server nodes consisting of partitioned sections of the inverted index. The parent process facilitates data distribution, and each node, be it a client or server, communicates via queries and responses. I used a very simple hashing scheme, utilizing the first letter of keys for data partitioning among nodes.

- The system consists of 2 main fucntions to be implemented,namely 
    - `void request_partition(void)` 
    - `void node_serve(void)`
### &emsp;(1) `void request_partition(void)`

- The main aim of this function is to communicate with the parent node, establish a connection with it, request the partition data for the current node, receive the data, allocate memory for the partition, read the partition data, construct a hash table for the partition, and finally close the connection with the parent node. 

- This process ensures that each node in the distributed system gets the relevant portion of the database to build and operate on.

### &emsp;(2) `void node_serve(void)`

- The main aim of this function is to continuously listen for client connections, read and processes client requests, construct responses based on the query results, and send the responses back to the clients. 

- It also handles scenarios where single or multiple query terms are involved, ensuring appropriate responses are generated and sent back to the clients.


## 2. Implementation

- The implementation begin with the `request_partition(void)` opening a connection to a parent node specified by `HOSTNAME` and `PARENT_PORT` using the fucntion 
```
Open_clientfd(HOSTNAME, port)
```
- Once the connection is established the system sends Node ID to parent and receive partition information usign differnet functions like 
```
Rio_writen(parent_connfd, request_line, strlen(request_line))

Rio_readlineb(&rio, response_line, MAXLINE)

Rio_readnb(&rio, partition.m_ptr, partition_size)
``` 
- After the partition information is received the function `build_hash_table()` is called to pass the partition data to construct an in-memory hash table for efficient data retrieval.

- After the hash table is built we move to the `node_serve(void)` where the incoming client connections are accepted and a new socket file descriptor `connfd` is returned for communication with the client. 

- The request line from the client is read and tokenised into extract query terms `key1` and `key2`. Then the local hash tables are checked to find the corresponding entries and Node ID's.

- The function then constructs a response based on the query results and writes the response back to the client using
```
Rio_writen(connfd, response, strlen(response))
```


## 3. Single Node
- The implementation started with a single node server capable of processing queries sequentially. During the ingest phase, each node received the share of the database, constructing an in-memory inverted index.

-  The server node first checks for the presence of `Key1` and `Key2` and then its index values `index1` and `index2` using `find_node()`. If only one key is present the server uses its index elements to construct a response and sends it back to the client.

- If both the keys are present then the server finds the common elements in both the index using `get_intersection()` and constructs a response using both the keys and intersected index and sends it back to the client.

## 4. Multi Node
- After the implementation of single node we extended our system to accomodate multiple nodes.The parent process managed the partitioning of data, ensuring equitable distribution among nodes.

- During the ingest phase the server node determines whether the incoming queries are within its partition. If they are then the same function is performed as mentioned in single node, if not then the Node ID of the request is found using `find_node()` and the queries are forwarded to their respective node to perform the same operations as on the single node.



## 5. Implementation Challenges and Key Observations


- One challenge I faced was implementing the `node_serve()` function, for some reason the client request was looping and printing twice where it was blank on the first go and gave a proper answer on the 2nd. So I had to use a flag to implement the function only when it is on the 2nd loop.


- During testing of the code for `single_node_4` I saw that my key was printing some garbage value. Did a lot of manual testing to find out that I was doing a lookup on intersection queries aswell, I changed that and solved my issue.

- As I moved forward I realised that in some cases multiple client requests are being processed by the function and my code was not handling it well. So I had to use a while loop to fix it.

- I faced a major problem with multi threading where for some reason the result was being printed twice in `multi_node_2`. 

## Conclusion
In conclusion, the implementation successfully met the demands of a distributed inverted index system. Through meticulous design decisions, innovative solutions to challenges, and rigorous testing,a robust and efficient system was created.


