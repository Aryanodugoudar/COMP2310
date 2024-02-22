#include "hashing.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Find the index of a bucket that can be used to insert the word. It will not
 * populate the given bucket at the index, you must do that yourself.
 */
int lookup_insert(hash_table *ht, char *word) {
  int i;
  unsigned int h, o;
  unsigned int k = 0;

  for (i = 0; word[i]; i++)
    k = (k * 33) + word[i];

  h = k % (ht->num_buckets);
  o = 1 + (k % (ht->num_buckets - 1));
  for (i = 0; i < ht->num_buckets; i++) {
    if (ht->buckets[h].used == 0) {
      return h;
    }
    h += o;
    if (h >= (unsigned int)ht->num_buckets) {
      h = h - ht->num_buckets;
    }
  }
  fprintf(stderr, "pedsort: hash table full\n");
  exit(1);
}

/**
 * Find the bucket containing the word. Will return -1 if the given word is
 * not present in the hash table.
 */
int lookup_find(hash_table *ht, char *word) {
  int i;
  unsigned int h, o;
  unsigned int k = 0;

  for (i = 0; word[i]; i++)
    k = (k * 33) + word[i];

  h = k % (ht->num_buckets);
  o = 1 + (k % (ht->num_buckets - 1));
  for (i = 0; i < ht->num_buckets; i++) {
    if (ht->buckets[h].used && (strcmp(ht->buckets[h].word, word) == 0)) {
      return h;
    }
    h += o;
    if (h >= (unsigned int)ht->num_buckets)
      h = h - ht->num_buckets;
  }
  return -1;
}
