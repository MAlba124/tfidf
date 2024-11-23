#ifndef __TFIDF_H_ARRAYLIST
#define __TFIDF_H_ARRAYLIST

#include <stddef.h>

#include "hash_map.h"
#include "skvs.h"

#define ARRAYLIST_EXTEND_WITH 1024

struct array_list_pair {
  char *location;
  struct hash_map map;
};

struct array_list {
  struct array_list_pair *data;
  size_t cap;
  size_t size;
};

struct array_list array_list_new(size_t cap);
void array_list_free(struct array_list *self);
void array_list_push(struct array_list *self, struct array_list_pair *element);

#endif // __TFIDF_H_ARRAYLIST
