#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "arena.h"
#include "mem.h"

struct arena arena_new(size_t element_size, size_t cap) {
  struct arena self;
  self.ptr = malloc_checked(element_size * cap);
  self.element_size = element_size;
  self.cap = cap;
  self.idx = 0;
  return self;
}

void arena_free(struct arena *self) {
  if (self->ptr) {
    free(self->ptr);
    self->ptr = NULL;
  }
}

void *arena_alloc(struct arena *self) {
  assert(self->idx < self->cap && "Arena is FULL");
  return self->ptr + self->idx++ * self->element_size;
}

void arena_shrink(struct arena *self) {
  self->ptr = realloc_checked(self->ptr, self->idx * self->element_size);
  self->cap = self->idx;
}
