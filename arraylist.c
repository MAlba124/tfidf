#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "arraylist.h"
#include "hash_map.h"
#include "mem.h"
#include "skvs.h"

struct array_list array_list_new(size_t cap) {
  struct array_list list = {
      .data = malloc_checked(sizeof(struct array_list_pair) * cap),
      .cap = cap,
      .size = 0,
  };
  return list;
}

void array_list_free(struct array_list *self) {
  for (size_t i = 0; i < self->size; i++) {
    free(self->data[i].location);
    hash_map_free(&self->data[i].map);
  }
  free(self->data);
  self->size = self->cap = 0;
}

void array_list_push(struct array_list *self, struct array_list_pair *element) {
  if (self->size >= self->cap) {
    self->data = realloc_checked(self->data, sizeof(struct array_list_pair) * self->cap * 2);
    self->cap += ARRAYLIST_EXTEND_WITH;
  }

  memcpy(&self->data[self->size], element, sizeof(struct array_list_pair));
  self->size++;
}

void array_list_shrink_to_fit(struct array_list *self) {
  self->cap = self->size;
  self->data = realloc_checked(self->data, sizeof(struct array_list_pair) * self->cap);
}
