#ifndef __TFIDF_H_MAP
#define __TFIDF_H_MAP

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HASH_MAP_DEFAULT_BUCKET_COUNT 512
#define HASH_MAP_THRESHOLD 0.75

struct linked_list_node {
  char *key;
  char *value;
  struct linked_list_node *next;
};

struct hash_map_bucket {
  struct linked_list_node *root;
};

struct hash_map {
  uint32_t (*hash)(const void * value);
  bool (*compare)(const void *rhs, const void *lhs);
  struct hash_map_bucket *buckets;
  size_t entries;
  size_t n_buckets;
};

struct hash_map hash_map_new(uint32_t (*hash)(const void *key), bool (*compare)(const void *lhs, const void *rhs));
struct hash_map hash_map_new_with_cap(size_t cap, uint32_t (*hash)(const void *key), bool (*compare)(const void *lhs, const void *rhs));
void hash_map_insert(struct hash_map *self, void *key, void *value, size_t key_size, size_t value_size);
void *hash_map_get(struct hash_map *self, void *key);
void *hash_map_get_or_insert(struct hash_map *self, void *key, void *value, size_t key_size, size_t value_size);
bool hash_map_contains(struct hash_map *self, void *key);
void hash_map_free(struct hash_map *self);

uint32_t hash_map_hash_char_star(const void *key);
bool hash_map_compare_char_star(const void *lhs, const void *rhs);

#endif // __TFIDF_H_MAP
