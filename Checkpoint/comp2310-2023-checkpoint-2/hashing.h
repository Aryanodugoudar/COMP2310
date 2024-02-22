#include <sys/types.h>

#define NUM_BUCKETS 8191

/*
   A single bucket in the hash table. You are allowed to modify this struct,
   especially if you think you can come up with a more efficient way of storing
   the necessary information in the hash table.
*/
typedef struct bucket {
  char *word;
  off_t offset;
  int used;
} bucket;

typedef struct hash_table {
  int num_buckets; // number of buckets the hash table contains
  bucket *buckets; // an array of size num_buckets
} hash_table;

int lookup_insert(hash_table *ht, char *word);
int lookup_find(hash_table *ht, char *word);
