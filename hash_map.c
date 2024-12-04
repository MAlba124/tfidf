#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "hash_map.h"
#include "mem.h"

// ##################################################
//               key:char* value:float
// ##################################################

static inline void hash_map_cf_insert_no_copy(struct hash_map_cf *self,
                                              char *key, float value);

static inline struct linked_list_node_cf *linked_list_cf_new(char *key,
                                                             float value) {
  struct linked_list_node_cf *node =
      malloc_checked(sizeof(struct linked_list_node_cf));
  node->key = key;
  node->value = value;
  node->next = NULL;
  return node;
}

static inline void linked_list_cf_free(struct linked_list_node_cf *root) {
  struct linked_list_node_cf *nod;
  while (root != NULL) {
    nod = root;
    root = root->next;
    free(nod->key);
    free(nod);
  }
}

static inline void
linked_list_cf_shallow_free(struct linked_list_node_cf *root) {
  struct linked_list_node_cf *nod;
  while (root != NULL) {
    nod = root;
    root = root->next;
    free(nod);
  }
}

static inline struct hash_map_bucket_cf *new_buckets_cf(size_t n) {
  struct hash_map_bucket_cf *buckets =
      malloc_checked(sizeof(struct hash_map_bucket_cf) * n);
  for (size_t i = 0; i < n; i++)
    buckets[i].root = NULL;
  return buckets;
}

static inline void free_buckets_cf(struct hash_map_bucket_cf *buckets,
                                   size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL)
      linked_list_cf_free(buckets[i].root);
  free(buckets);
}

static inline void free_buckets_cf_shallow(struct hash_map_bucket_cf *buckets,
                                           size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL)
      linked_list_cf_shallow_free(buckets[i].root);
  free(buckets);
}

static inline void hash_map_cf_rehash(struct hash_map_cf *self,
                                      size_t increase) {
  struct hash_map_bucket_cf *old = self->buckets;
  size_t old_n_buckets = self->n_buckets;
  self->buckets = new_buckets_cf(self->n_buckets + increase);
  self->n_buckets += increase;
  self->entries = 0;

  for (size_t i = 0; i < old_n_buckets; i++) {
    struct linked_list_node_cf *node = old[i].root;
    while (node != NULL) {
      hash_map_cf_insert_no_copy(self, node->key, node->value);
      node = node->next;
    }
  }

  free_buckets_cf_shallow(old, old_n_buckets);
}

struct hash_map_cf hash_map_cf_new_with_cap(size_t cap) {
  struct hash_map_cf map;
  map.buckets = new_buckets_cf(cap);
  map.n_buckets = cap;
  map.entries = 0;
  return map;
}

struct hash_map_cf hash_map_cf_new() {
  return hash_map_cf_new_with_cap(HASH_MAP_DEFAULT_BUCKET_COUNT);
}

static inline void hash_map_cf_insert_no_copy(struct hash_map_cf *self,
                                              char *key, float value) {
  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cf *bucket = &self->buckets[idx];

  struct linked_list_node_cf *node = bucket->root;
  if (node == NULL) {
    struct linked_list_node_cf *new_node = linked_list_cf_new(key, value);
    bucket->root = new_node;
    self->entries++;
    return;
  }

  while (true) {
    if (strcmp(node->key, key) == 0)
      return;
    else if (node->next == NULL) {
      struct linked_list_node_cf *new_node = linked_list_cf_new(key, value);
      node->next = new_node;
      self->entries++;
      break;
    }
    node = node->next;
  }
}

void hash_map_cf_insert(struct hash_map_cf *self, char *key, float value,
                        size_t key_size) {
  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cf *bucket = &self->buckets[idx];

  struct linked_list_node_cf *node = bucket->root;
  if (node == NULL) {
    char *key_data = malloc_checked(key_size);
    memcpy(key_data, key, key_size);
    struct linked_list_node_cf *new_node = linked_list_cf_new(key_data, value);
    bucket->root = new_node;
    self->entries++;

    double load_factor = (double)self->entries / (double)self->n_buckets;
    if (load_factor > HASH_MAP_THRESHOLD) {
      hash_map_cf_rehash(self, self->n_buckets);
    }
    return;
  }

  while (true) {
    if (strcmp(node->key, key) == 0) {
      return;
    } else if (node->next == NULL) {
      char *key_data = malloc_checked(key_size);
      memcpy(key_data, key, key_size);
      struct linked_list_node_cf *new_node =
          linked_list_cf_new(key_data, value);
      node->next = new_node;
      self->entries++;

      double load_factor = (double)self->entries / (double)self->n_buckets;
      if (load_factor > HASH_MAP_THRESHOLD) {
        hash_map_cf_rehash(self, self->n_buckets);
      }
      break;
    }
    node = node->next;
  }
}

float *hash_map_cf_get(struct hash_map_cf *self, char *key) {
  if (self->entries == 0 || self->n_buckets == 0)
    return NULL;

  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cf bucket = self->buckets[idx];
  struct linked_list_node_cf *node = bucket.root;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return &node->value;
    node = node->next;
  }

  return NULL;
}

float *hash_map_cf_get_or_insert(struct hash_map_cf *self, char *key,
                                 float value, size_t key_size) {
  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cf *bucket = &self->buckets[idx];
  struct linked_list_node_cf *prev_node = bucket->root;
  struct linked_list_node_cf *node = bucket->root;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return &node->value;

    prev_node = node;
    node = node->next;
  }

  char *key_data = malloc_checked(key_size);
  memcpy(key_data, key, key_size);
  struct linked_list_node_cf *new_node = linked_list_cf_new(key_data, value);

  float *new_value = &new_node->value;

  if (prev_node == NULL)
    bucket->root = new_node;
  else
    prev_node->next = new_node;

  self->entries++;

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor > HASH_MAP_THRESHOLD) {
    hash_map_cf_rehash(self, self->n_buckets);
  }
  return new_value;
}

bool hash_map_cf_contains(struct hash_map_cf *self, char *key) {
  return hash_map_cf_get(self, key) != NULL;
}

void hash_map_cf_free(struct hash_map_cf *self) {
  free_buckets_cf(self->buckets, self->n_buckets);
}

// ##################################################
//               key:char* value:uint32
// ##################################################
static inline void hash_map_cu32_insert_no_copy(struct hash_map_cu32 *self,
                                                char *key, uint32_t value);

static inline struct linked_list_node_cu32 *
linked_list_cu32_new(char *key, uint32_t value) {
  struct linked_list_node_cu32 *node =
      malloc_checked(sizeof(struct linked_list_node_cu32));
  node->key = key;
  node->value = value;
  node->next = NULL;
  return node;
}

static inline void linked_list_cu32_free(struct linked_list_node_cu32 *root) {
  struct linked_list_node_cu32 *nod;
  while (root != NULL) {
    nod = root;
    root = root->next;
    free(nod->key);
    free(nod);
  }
}

static inline void
linked_list_cu32_shallow_free(struct linked_list_node_cu32 *root) {
  struct linked_list_node_cu32 *nod;
  while (root != NULL) {
    nod = root;
    root = root->next;
    free(nod);
  }
}

static inline struct hash_map_bucket_cu32 *new_buckets_cu32(size_t n) {
  struct hash_map_bucket_cu32 *buckets =
      malloc_checked(sizeof(struct hash_map_bucket_cu32) * n);
  for (size_t i = 0; i < n; i++)
    buckets[i].root = NULL;
  return buckets;
}

static inline void free_buckets_cu32(struct hash_map_bucket_cu32 *buckets,
                                     size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL)
      linked_list_cu32_free(buckets[i].root);
  free(buckets);
}

static inline void
free_buckets_cu32_shallow(struct hash_map_bucket_cu32 *buckets, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (buckets[i].root != NULL)
      linked_list_cu32_shallow_free(buckets[i].root);
  free(buckets);
}

static inline void hash_map_cu32_rehash(struct hash_map_cu32 *self,
                                        size_t increase) {
  struct hash_map_bucket_cu32 *old = self->buckets;
  size_t old_n_buckets = self->n_buckets;
  self->buckets = new_buckets_cu32(self->n_buckets + increase);
  self->n_buckets += increase;
  self->entries = 0;

  for (size_t i = 0; i < old_n_buckets; i++) {
    struct linked_list_node_cu32 *node = old[i].root;
    while (node != NULL) {
      hash_map_cu32_insert_no_copy(self, node->key, node->value);
      node = node->next;
    }
  }

  free_buckets_cu32_shallow(old, old_n_buckets);
}

struct hash_map_cu32 hash_map_cu32_new_with_cap(size_t cap) {
  struct hash_map_cu32 map;
  map.buckets = new_buckets_cu32(cap);
  map.n_buckets = cap;
  map.entries = 0;
  return map;
}

struct hash_map_cu32 hash_map_cu32_new() {
  return hash_map_cu32_new_with_cap(HASH_MAP_DEFAULT_BUCKET_COUNT);
}

static inline void hash_map_cu32_insert_no_copy(struct hash_map_cu32 *self,
                                                char *key, uint32_t value) {
  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cu32 *bucket = &self->buckets[idx];

  struct linked_list_node_cu32 *node = bucket->root;
  if (node == NULL) {
    struct linked_list_node_cu32 *new_node = linked_list_cu32_new(key, value);
    bucket->root = new_node;
    self->entries++;
    return;
  }

  while (true) {
    if (strcmp(node->key, key) == 0)
      return;
    else if (node->next == NULL) {
      struct linked_list_node_cu32 *new_node = linked_list_cu32_new(key, value);
      node->next = new_node;
      self->entries++;
      break;
    }
    node = node->next;
  }
}

void hash_map_cu32_insert(struct hash_map_cu32 *self, char *key, uint32_t value,
                          size_t key_size) {
  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cu32 *bucket = &self->buckets[idx];

  struct linked_list_node_cu32 *node = bucket->root;
  if (node == NULL) {
    char *key_data = malloc_checked(key_size);
    memcpy(key_data, key, key_size);
    struct linked_list_node_cu32 *new_node =
        linked_list_cu32_new(key_data, value);
    bucket->root = new_node;
    self->entries++;

    double load_factor = (double)self->entries / (double)self->n_buckets;
    if (load_factor > HASH_MAP_THRESHOLD) {
      hash_map_cu32_rehash(self, self->n_buckets);
    }
    return;
  }

  while (true) {
    if (strcmp(node->key, key) == 0) {
      return;
    } else if (node->next == NULL) {
      char *key_data = malloc_checked(key_size);
      memcpy(key_data, key, key_size);
      struct linked_list_node_cu32 *new_node =
          linked_list_cu32_new(key_data, value);
      node->next = new_node;
      self->entries++;

      double load_factor = (double)self->entries / (double)self->n_buckets;
      if (load_factor > HASH_MAP_THRESHOLD) {
        hash_map_cu32_rehash(self, self->n_buckets);
      }
      break;
    }
    node = node->next;
  }
}

uint32_t *hash_map_cu32_get(struct hash_map_cu32 *self, char *key) {
  if (self->entries == 0 || self->n_buckets == 0)
    return NULL;

  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cu32 bucket = self->buckets[idx];
  struct linked_list_node_cu32 *node = bucket.root;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return &node->value;
    node = node->next;
  }

  return NULL;
}

uint32_t *hash_map_cu32_get_or_insert(struct hash_map_cu32 *self, char *key,
                                      uint32_t value, size_t key_size) {
  uint32_t hash = 5381;
  char *str = key;
  int c;
  while ((c = *str++))
    hash = c + (hash << 6) + (hash << 16) - hash;

  uint32_t idx = hash % self->n_buckets;
  struct hash_map_bucket_cu32 *bucket = &self->buckets[idx];
  struct linked_list_node_cu32 *prev_node = bucket->root;
  struct linked_list_node_cu32 *node = bucket->root;
  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return &node->value;

    prev_node = node;
    node = node->next;
  }

  char *key_data = malloc_checked(key_size);
  memcpy(key_data, key, key_size);
  struct linked_list_node_cu32 *new_node =
      linked_list_cu32_new(key_data, value);

  uint32_t *new_value = &new_node->value;

  if (prev_node == NULL)
    bucket->root = new_node;
  else
    prev_node->next = new_node;

  self->entries++;

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor > HASH_MAP_THRESHOLD) {
    hash_map_cu32_rehash(self, self->n_buckets);
  }
  return new_value;
}

bool hash_map_cu32_contains(struct hash_map_cu32 *self, char *key) {
  return hash_map_cu32_get(self, key) != NULL;
}

void hash_map_cu32_free(struct hash_map_cu32 *self) {
  free_buckets_cu32(self->buckets, self->n_buckets);
}

// ##################################################
//               key:uint32_t value:float
// ##################################################
static inline struct hash_map_bucket_u32f *new_buckets_u32f(size_t n) {
  struct hash_map_bucket_u32f *buckets =
      malloc_checked(sizeof(struct hash_map_bucket_u32f) * n);
  for (size_t i = 0; i < n; i++)
    buckets[i].root = NULL;
  return buckets;
}

static inline void hash_map_u32f_rehash(struct hash_map_u32f *self,
                                        size_t new) {
  struct hash_map_bucket_u32f *old = self->buckets;
  size_t old_n_buckets = self->n_buckets;
  struct arena old_a = self->a;
  self->buckets = new_buckets_u32f(new);
  self->n_buckets = new;
  self->a =
      arena_new(sizeof(struct hash_map_bucket_node_u32f),
                1 + (size_t)ceilf((float)self->n_buckets * HASH_MAP_THRESHOLD));
  self->entries = 0;

  for (size_t i = 0; i < old_n_buckets; i++) {
    struct hash_map_bucket_node_u32f *node = old[i].root;
    while (node != NULL) {
      hash_map_u32f_insert(self, node->key, node->value);
      node = node->next;
    }
  }

  arena_free(&old_a);
  free(old);
}

struct hash_map_u32f hash_map_u32f_new_with_cap(size_t cap) {
  struct hash_map_u32f map;
  map.buckets = new_buckets_u32f(cap);
  map.n_buckets = cap;
  map.a = arena_new(sizeof(struct hash_map_bucket_node_u32f),
                    1 + (size_t)ceilf((float)cap * HASH_MAP_THRESHOLD));
  map.entries = 0;
  return map;
}

struct hash_map_u32f hash_map_u32f_new() {
  return hash_map_u32f_new_with_cap(HASH_MAP_DEFAULT_BUCKET_COUNT);
}

void hash_map_u32f_insert(struct hash_map_u32f *self, uint32_t key,
                          float value) {
  uint32_t idx = key % self->n_buckets;
  struct hash_map_bucket_u32f *bucket = &self->buckets[idx];

  struct hash_map_bucket_node_u32f *node = bucket->root;
  if (node == NULL) {
    struct hash_map_bucket_node_u32f *new_node = arena_alloc(&self->a);
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    bucket->root = new_node;
    self->entries++;

    double load_factor = (double)self->entries / (double)self->n_buckets;
    if (load_factor > HASH_MAP_THRESHOLD) {
      hash_map_u32f_rehash(self, self->n_buckets * 2);
    }
    return;
  }
  while (true) {
    if (node->key == key) {
      return;
    } else if (node->next == NULL) {
      struct hash_map_bucket_node_u32f *new_node = arena_alloc(&self->a);
      new_node->key = key;
      new_node->value = value;
      new_node->next = NULL;
      node->next = new_node;
      self->entries++;

      double load_factor = (double)self->entries / (double)self->n_buckets;
      if (load_factor > HASH_MAP_THRESHOLD) {
        hash_map_u32f_rehash(self, self->n_buckets * 2);
      }
      break;
    }
    node = node->next;
  }
}

float *hash_map_u32f_get(struct hash_map_u32f *self, uint32_t key) {
  if (self->entries == 0 || self->n_buckets == 0)
    return NULL;
  uint32_t idx = key % self->n_buckets;
  struct hash_map_bucket_node_u32f *node = self->buckets[idx].root;
  while (node != NULL) {
    if (node->key == key)
      return &node->value;
    node = node->next;
  }

  return NULL;
}

float *hash_map_u32f_get_or_insert(struct hash_map_u32f *self, uint32_t key,
                                   float value) {
  uint32_t idx = key % self->n_buckets;
  struct hash_map_bucket_u32f *bucket = &self->buckets[idx];
  struct hash_map_bucket_node_u32f *prev_node = bucket->root;
  struct hash_map_bucket_node_u32f *node = bucket->root;
  while (node != NULL) {
    if (node->key == key)
      return &node->value;

    prev_node = node;
    node = node->next;
  }

  struct hash_map_bucket_node_u32f *new_node = arena_alloc(&self->a);
  new_node->key = key;
  new_node->value = value;
  new_node->next = NULL;

  float *new_value = &new_node->value;

  if (prev_node == NULL)
    bucket->root = new_node;
  else
    prev_node->next = new_node;

  self->entries++;

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor > HASH_MAP_THRESHOLD) {
    hash_map_u32f_rehash(self, self->n_buckets * 2);
    uint32_t idx = key % self->n_buckets;
    struct hash_map_bucket_u32f *bucket = &self->buckets[idx];
    struct hash_map_bucket_node_u32f *node = bucket->root;
    while (node != NULL) {
      if (node->key == key)
        return &node->value;
      prev_node = node;
      node = node->next;
    }
  } else {
    return new_value;
  }

  assert(0 && "Reached unreachable code!");
}

void hash_map_u32f_shrink(struct hash_map_u32f *self) {
  if (self->entries == 0) {
    self->n_buckets = 0;
    arena_free(&self->a);
    free(self->buckets);
    self->buckets = NULL;
    return;
  }

  double load_factor = (double)self->entries / (double)self->n_buckets;
  if (load_factor < HASH_MAP_THRESHOLD) {
    size_t new = self->entries + 1 + (size_t)((float)self->entries * 1.0 - HASH_MAP_THRESHOLD);
    struct hash_map_bucket_u32f *old = self->buckets;
    size_t old_n_buckets = self->n_buckets;
    struct arena old_a = self->a;
    self->buckets = new_buckets_u32f(new);
    self->n_buckets = new;
    self->a = arena_new(sizeof(struct hash_map_bucket_node_u32f), self->entries);
    self->entries = 0;

    for (size_t i = 0; i < old_n_buckets; i++) {
        struct hash_map_bucket_node_u32f *node = old[i].root;
        while (node != NULL) {
        hash_map_u32f_insert(self, node->key, node->value);
        node = node->next;
        }
    }

    arena_free(&old_a);
    free(old);
    return;
  }

  char *old_ptr = self->a.ptr;
  arena_shrink(&self->a);
  for (size_t i = 0; i < self->n_buckets; i++) {
    if (self->buckets[i].root == NULL)
      continue;
    self->buckets[i].root =
        (struct hash_map_bucket_node_u32f *)((intptr_t)self->buckets[i].root -
                                             (intptr_t)old_ptr +
                                             (intptr_t)self->a.ptr);
    struct hash_map_bucket_node_u32f *node = self->buckets[i].root;
    while (node != NULL) {
      if (node->next != NULL) {
        node->next =
            (struct hash_map_bucket_node_u32f *)((intptr_t)node->next -
                                                 (intptr_t)old_ptr +
                                                 (intptr_t)self->a.ptr);
      }
      node = node->next;
    }
  }
}

bool hash_map_u32f_contains(struct hash_map_u32f *self, uint32_t key) {
  return hash_map_u32f_get(self, key) != NULL;
}

void hash_map_u32f_free(struct hash_map_u32f *self) {
  arena_free(&self->a);
  if (self->buckets)
    free(self->buckets);
}
