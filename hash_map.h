#ifndef __TFIDF_H_MAP
#define __TFIDF_H_MAP

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena.h"

#define HASH_MAP_DEFAULT_BUCKET_COUNT 512
#define HASH_MAP_THRESHOLD 0.75

// hash map for key:char* and value:u32
struct linked_list_node_cu32 {
  char *key;
  uint32_t value;
  struct linked_list_node_cu32 *next;
};

struct hash_map_bucket_cu32 {
  struct linked_list_node_cu32 *root;
};

struct hash_map_cu32 {
  struct hash_map_bucket_cu32 *buckets;
  size_t entries;
  size_t n_buckets;
};

struct hash_map_cu32 hash_map_cu32_new();
struct hash_map_cu32 hash_map_cu32_new_with_cap(size_t cap);
void hash_map_cu32_insert(struct hash_map_cu32 *self, char *key, uint32_t value,
                          size_t key_size);
uint32_t *hash_map_cu32_get(struct hash_map_cu32 *self, char *key);
uint32_t *hash_map_cu32_get_or_insert(struct hash_map_cu32 *self, char *key,
                                      uint32_t value, size_t key_size);
bool hash_map_cu32_contains(struct hash_map_cu32 *self, char *key);
void hash_map_cu32_free(struct hash_map_cu32 *self);

struct hash_map_bucket_node_u32f {
  uint32_t key;
  float value;
  struct hash_map_bucket_node_u32f *next;
};

struct hash_map_bucket_u32f {
  size_t n_nodes;
  struct hash_map_bucket_node_u32f *root;
};

struct hash_map_u32f {
  struct hash_map_bucket_u32f *buckets;
  struct arena a;
  size_t entries;
  size_t n_buckets;
};

struct hash_map_u32f hash_map_u32f_new();
struct hash_map_u32f hash_map_u32f_new_with_cap(size_t cap);
void hash_map_u32f_insert(struct hash_map_u32f *self, uint32_t key,
                          float value);
float *hash_map_u32f_get(struct hash_map_u32f *self, uint32_t key);
float *hash_map_u32f_get_or_insert(struct hash_map_u32f *self, uint32_t key,
                                   float value);
bool hash_map_u32f_contains(struct hash_map_u32f *self, uint32_t key);
void hash_map_u32f_free(struct hash_map_u32f *self);
void hash_map_u32f_shrink(struct hash_map_u32f *self);

#endif // __TFIDF_H_MAP
