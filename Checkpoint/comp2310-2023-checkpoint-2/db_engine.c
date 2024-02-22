#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "hashing.h"

// #define QDB_NAIVE
#define QDB_OPT

void create_db(const char *csv_file, const char *db_file);
void query_db(const char *db_file, const char *cmd_file);
void print_db(const char *db_file, const char *dst_file);

hash_table ht;

void create_db(const char *csv_file, const char *db_file)
{
  FILE *csv_fp = fopen(csv_file, "r");
  FILE *db_fp = fopen(db_file, "wb");
  if (csv_fp == NULL || db_fp == NULL)
  {
    fprintf(stderr, "fopen error: %s\n", strerror(errno));
    exit(1);
  }

  char line[1000];
  while (fgets(line, sizeof(line), csv_fp) != NULL)
  {
    line[strcspn(line, "\n")] = '\0';

    char *key = strtok(line, ",");
    int att = 0;
    int attri[1000];

    char *token = strtok(NULL, ",");
    while (token != NULL)
    {
      attri[att++] = atoi(token);
      token = strtok(NULL, ",");
    }

    fwrite(key, strlen(key), 1, db_fp);
    fputc('\0', db_fp);

    size_t padding = (4 - (strlen(key) + 1) % 4) % 4;
    for (size_t i = 0; i < padding; ++i)
    {
      fputc('\0', db_fp);
    }

    fwrite(&att, sizeof(int), 1, db_fp);
    fwrite(attri, sizeof(int), att, db_fp);
  }

  fclose(csv_fp);
  fclose(db_fp);
}

void query_db(const char *db_file, const char *cmd_file)
{
#ifdef QDB_OPT
  FILE *db_fp = fopen(db_file, "r");
  if (db_fp == NULL)
  {
    fprintf(stderr, "fopen error: %s\n", strerror(errno));
    exit(1);
  }

  int db_size;
  fseek(db_fp, 0, SEEK_END);
  db_size = ftell(db_fp);
  rewind(db_fp);

  void *db_ptr = mmap(NULL, db_size, PROT_READ, MAP_PRIVATE, fileno(db_fp), 0);
  if (db_ptr == MAP_FAILED)
  {
    fprintf(stderr, "MMap failed: %s\n", strerror(errno));
    exit(1);
  }

  hash_table table;
  table.num_buckets = NUM_BUCKETS;
  table.buckets = (bucket *)malloc(sizeof(bucket) * NUM_BUCKETS);

  int offset = 0;
  while (offset < db_size)
  {
    int indx;
    char *key = (char *)db_ptr + offset;
    size_t padding = (4 - (strlen(key) + 1) % 4) % 4;
    unsigned int *num_ids = (unsigned int *)(key + strlen(key) + 1 + padding);
    unsigned int *doc_ids = num_ids + 1;
    indx = lookup_insert(&table, key);
    table.buckets[indx].word = key;
    table.buckets[indx].offset = (off_t)(key);
    table.buckets[indx].used = 1;

    offset += (strlen(key) + 1 + padding + sizeof(int) + sizeof(int) * (*num_ids));
  }

  FILE *cmd_fp = fopen(cmd_file, "r");
  if (cmd_fp == NULL)
  {
    fprintf(stderr, "fopen error: %s\n", strerror(errno));
    exit(1);
  }

  char line[1000];
  while (fgets(line, sizeof(line), cmd_fp) != NULL)
  {
    line[strcspn(line, "\n")] = '\0';
    char *key1 = strtok(line, " ");
    char *key2 = strtok(NULL, " ");

    int index1 = lookup_find(&table, key1);
    int index2 = (key2 != NULL) ? lookup_find(&table, key2) : -1;

    if (index1 == -1)
    {
      printf("%s not found\n", key1);
    }
    else if (key2 == NULL)
    {
      size_t padding = (4 - (strlen(key1) + 1) % 4) % 4;
      int *doc_ids = (int *)(table.buckets[index1].offset + strlen(key1) + 1 + padding);
      printf("%s", key1);
      for (int i = 0; i < doc_ids[0]; i++)
      {
        printf(",%d", doc_ids[i + 1]);
      }
      printf("\n");
    }

    if (index2 == -1 && key2 != NULL)
    {
      printf("%s not found\n", key2);
    }
    else if ((key2 != NULL) && (index1 != -1))
    {
      size_t padding1 = (4 - (strlen(key1) + 1) % 4) % 4;
      size_t padding2 = (4 - (strlen(key2) + 1) % 4) % 4;
      int *doc_ids1 = (int *)(table.buckets[index1].offset + strlen(key1) + 1 + padding1);
      int *doc_ids2 = (int *)(table.buckets[index2].offset + strlen(key2) + 1 + padding2);

      int i = 1, j = 1;
      int common_docs = 0;

      // Count common doc_ids
      // int com[1000];
      int size1 = doc_ids1[0];

      int size2 = doc_ids2[0];

      int com[size1 + size2];
      int comIndex = 0;

      for (int i = 1; i <= size1; i++)
      {
        for (int j = 1; j <= size2; j++)
        {
          if (doc_ids1[i] == doc_ids2[j])
          {

            int isDuplicate = 0;
            for (int k = 0; k < comIndex; k++)
            {
              if (doc_ids1[i] == com[k])
              {
                isDuplicate = 1;
                break;
              }
            }

            if (!isDuplicate)
            {
              com[comIndex++] = doc_ids1[i];
            }
            break;
          }
        }
      }

      // Print the common doc_ids
      printf("%s,%s", key1, key2);
      for (int i = 0; i < comIndex; i++)
      {
        printf(",%d", com[i]);
      }
      printf("\n");
    }
  }

  munmap(db_ptr, db_size);
  fclose(db_fp);
  fclose(cmd_fp);
#endif
}

void print_db(const char *db_file, const char *csv_file)
{
  FILE *csv_fp = fopen(csv_file, "w");
  FILE *db_fp = fopen(db_file, "r");
  if (csv_fp == NULL || db_fp == NULL)
  {
    fprintf(stderr, "fopen error: %s\n", strerror(errno));
    exit(1);
  }

  int db_size;
  fseek(db_fp, 0, SEEK_END);
  db_size = ftell(db_fp);
  rewind(db_fp);

  void *db_ptr = mmap(NULL, db_size, PROT_READ, MAP_PRIVATE, fileno(db_fp), 0);
  if (db_ptr == MAP_FAILED)
  {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(1);
  }

  int offset = 0;
  while (offset < db_size)
  {
    char *key = (char *)db_ptr + offset;
    size_t padding = (4 - (strlen(key) + 1) % 4) % 4;
    unsigned int *num_ids = (unsigned int *)(key + strlen(key) + 1 + padding);
    unsigned int *doc_ids = num_ids + 1;

    fwrite(key, strlen(key), 1, csv_fp);
    for (int i = 0; i < *num_ids; i++)
    {
      fprintf(csv_fp, ",%u", doc_ids[i]);
    }
    fprintf(csv_fp, "\n");

    offset += (strlen(key) + 1 + padding + sizeof(int) + sizeof(int) * (*num_ids));
  }

  munmap(db_ptr, db_size);
  fclose(db_fp);
  fclose(csv_fp);
}

int main(int argc, char const *argv[])
{
  const char *command, *file1, *file2;
  if (argc != 4)
  {
    fprintf(stderr, "usage: %s [CREATE|QUERY|PRINT] [file1] [file2]\n", argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "CREATE") == 0)
  {
    create_db(argv[2], argv[3]);
  }
  else if (strcmp(argv[1], "QUERY") == 0)
  {
    query_db(argv[2], argv[3]);
  }
  else if (strcmp(argv[1], "PRINT") == 0)
  {
    print_db(argv[2], argv[3]);
  }
  else
  {
    fprintf(stderr, "invalid command given.\n");
    exit(1);
  }

  return 0;
}
