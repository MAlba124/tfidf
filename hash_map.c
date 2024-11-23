#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash_map.h"
#include "mem.h"

static void hash_map_insert_no_copy(struct hash_map *self, void *key,
                                    void *value);

struct linked_list_node *linked_list_new(char *key, char *value) {
  struct linked_list_node *node =
      malloc_checked(sizeof(struct linked_list_node));
  node->key = key;
  node->value = value;
  node->next = NULL;
  return node;
}

void linked_list_free(struct linked_list_node *root) {
  if (root->next == NULL) {
    free(root->key);
    free(root->value);
    free(root);
    return;
  }

  linked_list_free(root->next);
  free(root->key);
  free(root->value);
  free(root);
}

void linked_list_shallow_free(struct linked_list_node *root) {
  if (root->next == NULL) {
    free(root);
    return;
  }

  linked_list_shallow_free(root->next);
  free(root);
}

static struct hash_map_bucket *new_buckets(size_t n) {
  struct hash_map_bucket *buckets =
      malloc_checked(sizeof(struct hash_map_bucket) * n);
  for (size_t i = 0; i < n; i++)
    buckets[i].root = NULL;
  return buckets;
}

static void free_buckets(struct hash_map_bucket *buckets, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL) {
      linked_list_free(buckets[i].root);
    }
  free(buckets);
}

static void free_buckets_shallow(struct hash_map_bucket *buckets, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL) {
      linked_list_shallow_free(buckets[i].root);
    }
  free(buckets);
}

static void hash_map_rehash(struct hash_map *self, size_t increase) {
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
hash_map_new_with_cap(size_t cap, uint64_t (*hash)(const void *key),
                      bool (*compare)(const void *lhs, const void *rhs)) {
  struct hash_map map = {.hash = hash, .compare = compare};
  map.buckets = new_buckets(cap);
  map.n_buckets = cap;
  map.entries = 0;
  return map;
}

struct hash_map hash_map_new(uint64_t (*hash)(const void *value),
                             bool (*compare)(const void *lhs,
                                             const void *rhs)) {
  return hash_map_new_with_cap(HASH_MAP_DEFAULT_BUCKET_COUNT, hash, compare);
}

static void hash_map_insert_no_copy(struct hash_map *self, void *key,
                                    void *value) {
  if (hash_map_contains(self, key))
    return;

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor > HASH_MAP_THRESHOLD) {
    hash_map_rehash(self, self->entries + HASH_MAP_DEFAULT_BUCKET_COUNT);
  }

  int64_t idx = (self->hash)(key) % self->n_buckets;
  struct hash_map_bucket bucket = self->buckets[idx];
  struct linked_list_node *new_node = linked_list_new(key, value);

  struct linked_list_node *node = bucket.root;
  if (node == NULL) {
    self->buckets[idx].root = new_node;
    self->entries++;
    return;
  }

  while (true) {
    if (node->next == NULL) {
      node->next = new_node;
      self->entries++;
      break;
    }
    node = node->next;
  }
}

void hash_map_insert(struct hash_map *self, void *key, void *value,
                     size_t key_size, size_t value_size) {
  if (hash_map_contains(self, key))
    return;

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor > HASH_MAP_THRESHOLD) {
    hash_map_rehash(self, self->entries + HASH_MAP_DEFAULT_BUCKET_COUNT);
  }

  int64_t idx = (self->hash)(key) % self->n_buckets;
  struct hash_map_bucket bucket = self->buckets[idx];
  char *key_data = malloc_checked(key_size);
  memcpy(key_data, key, key_size);
  char *value_data = malloc_checked(value_size);
  memcpy(value_data, value, value_size);
  struct linked_list_node *new_node = linked_list_new(key_data, value_data);

  struct linked_list_node *node = bucket.root;
  if (node == NULL) {
    self->buckets[idx].root = new_node;
    self->entries++;
    return;
  }

  while (true) {
    if (node->next == NULL) {
      node->next = new_node;
      self->entries++;
      break;
    }
    node = node->next;
  }
}

void *hash_map_get(struct hash_map *self, void *key) {
  if (self->entries == 0 || self->n_buckets == 0)
    return NULL;

  int64_t idx = (self->hash)(key) % self->n_buckets;
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

uint64_t hash_map_hash_char_star(const void *key) {
  const char *str = key;
  uint64_t sum = 0;
  for (size_t i = 0; i < strlen(key); i++) {
    sum += str[i];
  }
  return sum;
}

bool hash_map_compare_char_star(const void *vlhs, const void *vrhs) {
  const char *lhs = vlhs;
  const char *rhs = vrhs;
  const size_t lhs_len = strlen(lhs);
  const size_t rhs_len = strlen(rhs);

  if (lhs_len != rhs_len)
    return false;

  return strcmp(lhs, rhs) == 0;
}
