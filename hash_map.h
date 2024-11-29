#ifndef __TFIDF_H_MAP
#define __TFIDF_H_MAP

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HASH_MAP_DEFAULT_BUCKET_COUNT 512
#define HASH_MAP_THRESHOLD 0.75

// Is linked list okay?
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

// Hash map for key:char* and value:float
struct linked_list_node_cf {
  char *key;
  float value;
  struct linked_list_node_cf *next;
};

struct hash_map_bucket_cf {
  struct linked_list_node_cf *root;
};

struct hash_map_cf {
  struct hash_map_bucket_cf *buckets;
  size_t entries;
  size_t n_buckets;
};

struct hash_map_cf hash_map_cf_new();
struct hash_map_cf hash_map_cf_new_with_cap(size_t cap);
void hash_map_cf_insert(struct hash_map_cf *self, char *key, float value, size_t key_size);
float *hash_map_cf_get(struct hash_map_cf *self, char *key);
float *hash_map_cf_get_or_insert(struct hash_map_cf *self, char *key, float value, size_t key_size);
bool hash_map_cf_contains(struct hash_map_cf *self, char *key);
void hash_map_cf_free(struct hash_map_cf *self);

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
void hash_map_cu32_insert(struct hash_map_cu32 *self, char *key, uint32_t value, size_t key_size);
uint32_t *hash_map_cu32_get(struct hash_map_cu32 *self, char *key);
uint32_t *hash_map_cu32_get_or_insert(struct hash_map_cu32 *self, char *key, uint32_t value, size_t key_size);
bool hash_map_cu32_contains(struct hash_map_cu32 *self, char *key);
void hash_map_cu32_free(struct hash_map_cu32 *self);

// hash map for key:uint32_t and value:float
struct linked_list_node_u32f {
  uint32_t key;
  float value;
  struct linked_list_node_u32f *next;
};

struct hash_map_bucket_u32f {
  struct linked_list_node_u32f *root;
};

struct hash_map_u32f {
  struct hash_map_bucket_u32f *buckets;
  size_t entries;
  size_t n_buckets;
};

struct hash_map_u32f hash_map_u32f_new();
struct hash_map_u32f hash_map_u32f_new_with_cap(size_t cap);
void hash_map_u32f_insert(struct hash_map_u32f *self, uint32_t key, float value);
float *hash_map_u32f_get(struct hash_map_u32f *self, uint32_t key);
float *hash_map_u32f_get_or_insert(struct hash_map_u32f *self, uint32_t key, float value);
bool hash_map_u32f_contains(struct hash_map_u32f *self, uint32_t key);
void hash_map_u32f_free(struct hash_map_u32f *self);

#endif // __TFIDF_H_MAP
