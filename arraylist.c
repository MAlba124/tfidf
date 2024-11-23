#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "arraylist.h"
#include "hash_map.h"
#include "mem.h"
#include "skvs.h"

struct array_list array_list_new(size_t cap) {
  struct array_list list = {
      /* .data = malloc_checked(sizeof(struct skvs_pair) * cap), */
      .data = malloc_checked(sizeof(struct array_list_pair) * cap),
      .cap = cap,
      .size = 0,
  };
  return list;
}

void array_list_free(struct array_list *self) {
  for (size_t i = 0; i < self->size; i++) {
    /* free(self->data[i].key); */
    /* free(self->data[i].value); */
    free(self->data[i].location);
    hash_map_free(&self->data[i].map);
  }
  free(self->data);
  self->size = self->cap = 0;
}

/* void array_list_push(struct array_list *self, struct skvs_pair *element) { */
void array_list_push(struct array_list *self, struct array_list_pair *element) {
  if (self->size >= self->cap) {
    /* self->data = realloc_checked(self->data, sizeof(struct skvs_pair) *
     * self->cap + sizeof(struct skvs_pair) * ARRAYLIST_EXTEND_WITH); */
    self->data = realloc_checked(
        self->data, sizeof(struct array_list_pair) * self->cap +
                        sizeof(struct array_list_pair) * ARRAYLIST_EXTEND_WITH);
    self->cap += ARRAYLIST_EXTEND_WITH;
  }

  /* memcpy(&self->data[self->size], element, sizeof(struct skvs_pair)); */
  memcpy(&self->data[self->size], element, sizeof(struct array_list_pair));
  self->size++;
}
