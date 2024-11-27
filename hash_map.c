#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hash_map.h"
#include "mem.h"

static inline void hash_map_insert_no_copy(struct hash_map *self, void *key,
                                    void *value);

static inline struct linked_list_node *linked_list_new(char *key, char *value) {
  struct linked_list_node *node =
      malloc_checked(sizeof(struct linked_list_node));
  node->key = key;
  node->value = value;
  node->next = NULL;
  return node;
}

static inline void linked_list_free(struct linked_list_node *root) {
  struct linked_list_node *nod;
  while (root != NULL) {
    nod = root;
    root = root->next;
    free(nod->key);
    free(nod->value);
    free(nod);
  }
}

static inline void linked_list_shallow_free(struct linked_list_node *root) {
  struct linked_list_node *nod;
  while (root != NULL) {
    nod = root;
    root = root->next;
    free(nod);
  }
}

static inline struct hash_map_bucket *new_buckets(size_t n) {
  struct hash_map_bucket *buckets =
      malloc_checked(sizeof(struct hash_map_bucket) * n);
  for (size_t i = 0; i < n; i++)
    buckets[i].root = NULL;
  return buckets;
}

static inline void free_buckets(struct hash_map_bucket *buckets, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL)
      linked_list_free(buckets[i].root);
  free(buckets);
}

static inline void free_buckets_shallow(struct hash_map_bucket *buckets, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL)
      linked_list_shallow_free(buckets[i].root);
  free(buckets);
}

static inline void hash_map_rehash(struct hash_map *self, size_t increase) {
  struct hash_map_bucket *old = self->buckets;
  size_t old_n_buckets = self->n_buckets;
  self->buckets = new_buckets(self->n_buckets + increase);
  self->n_buckets += increase;
  self->entries = 0;

  for (size_t i = 0; i < old_n_buckets; i++) {
    struct linked_list_node *node = old[i].root;
    while (node != NULL) {
      hash_map_insert_no_copy(self, node->key, node->value);
      node = node->next;
    }
  }

  free_buckets_shallow(old, old_n_buckets);
}

struct hash_map
hash_map_new_with_cap(size_t cap, uint32_t (*hash)(const void *key),
                      bool (*compare)(const void *lhs, const void *rhs)) {
  struct hash_map map = {.hash = hash, .compare = compare};
  map.buckets = new_buckets(cap);
  map.n_buckets = cap;
  map.entries = 0;
  return map;
}

struct hash_map hash_map_new(uint32_t (*hash)(const void *value),
                             bool (*compare)(const void *lhs,
                                             const void *rhs)) {
  return hash_map_new_with_cap(HASH_MAP_DEFAULT_BUCKET_COUNT, hash, compare);
}

static inline void hash_map_insert_no_copy(struct hash_map *self, void *key,
                                    void *value) {
  uint32_t idx = (self->hash)(key) % self->n_buckets;
  struct hash_map_bucket *bucket = &self->buckets[idx];

  struct linked_list_node *node = bucket->root;
  if (node == NULL) {
    struct linked_list_node *new_node = linked_list_new(key, value);
    bucket->root = new_node;
    self->entries++;
    return;
  }

  while (true) {
    if ((self->compare)(node->key, key))
      return;
    else if (node->next == NULL) {
      struct linked_list_node *new_node = linked_list_new(key, value);
      node->next = new_node;
      self->entries++;
      break;
    }
    node = node->next;
  }
}

void hash_map_insert(struct hash_map *self, void *key, void *value,
                     size_t key_size, size_t value_size) {
  uint32_t idx = (self->hash)(key) % self->n_buckets;
  struct hash_map_bucket *bucket = &self->buckets[idx];

  struct linked_list_node *node = bucket->root;
  if (node == NULL) {
    char *key_data = malloc_checked(key_size);
    memcpy(key_data, key, key_size);
    char *value_data = malloc_checked(value_size);
    memcpy(value_data, value, value_size);
    struct linked_list_node *new_node = linked_list_new(key_data, value_data);
    bucket->root = new_node;
    self->entries++;

    double load_factor = (double)self->entries / (double)self->n_buckets;
    if (load_factor > HASH_MAP_THRESHOLD) {
      hash_map_rehash(self, self->n_buckets);
    }
    return;
  }

  while (true) {
    if ((self->compare)(node->key, key)) {
      return;
    } else if (node->next == NULL) {
      char *key_data = malloc_checked(key_size);
      memcpy(key_data, key, key_size);
      char *value_data = malloc_checked(value_size);
      memcpy(value_data, value, value_size);
      struct linked_list_node *new_node = linked_list_new(key_data, value_data);
      node->next = new_node;
      self->entries++;

      double load_factor = (double)self->entries / (double)self->n_buckets;
      if (load_factor > HASH_MAP_THRESHOLD) {
        hash_map_rehash(self, self->n_buckets);
      }
      break;
    }
    node = node->next;
  }
}

void *hash_map_get_or_insert(struct hash_map *self, void *key, void *value, size_t key_size, size_t value_size) {
  uint32_t idx = (self->hash)(key) % self->n_buckets;
  struct hash_map_bucket *bucket = &self->buckets[idx];
  struct linked_list_node *prev_node = bucket->root;
  struct linked_list_node *node = bucket->root;
  while (node != NULL) {
    if (self->compare(key, (void *)node->key))
      return (void *)node->value;

    prev_node = node;
    node = node->next;
  }

  char *key_data = malloc_checked(key_size);
  memcpy(key_data, key, key_size);
  char *value_data = malloc_checked(value_size);
  memcpy(value_data, value, value_size);
  struct linked_list_node *new_node = linked_list_new(key_data, value_data);

  char *new_value = new_node->value;

  if (prev_node == NULL)
    bucket->root = new_node;
  else
    prev_node->next = new_node;

  self->entries++;

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor > HASH_MAP_THRESHOLD) {
    hash_map_rehash(self, self->n_buckets);
  }
  return new_value;
}

void *hash_map_get(struct hash_map *self, void *key) {
  if (self->entries == 0 || self->n_buckets == 0)
    return NULL;

  uint32_t idx = (self->hash)(key) % self->n_buckets;
  struct hash_map_bucket bucket = self->buckets[idx];
  struct linked_list_node *node = bucket.root;
  while (node != NULL) {
    if (self->compare(key, (void *)node->key))
      return (void *)node->value;
    node = node->next;
  }

  return NULL;
}

bool hash_map_contains(struct hash_map *self, void *key) {
  return hash_map_get(self, key) != NULL;
}

void hash_map_free(struct hash_map *self) {
  free_buckets(self->buckets, self->n_buckets);
}

// sdbm hash function see: http://www.cse.yorku.ca/~oz/hash.html
uint32_t hash_map_hash_char_star(const void *key) {
  const char *str = key;
  uint32_t hash = 5381;
  int c;

  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  return hash;
}

bool hash_map_compare_char_star(const void *lhs, const void *rhs) {
  return strcmp((char *)lhs, (char *)rhs) == 0;
}
