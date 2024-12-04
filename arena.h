#ifndef __TFIDF_H_ARENA
#define __TFIDF_H_ARENA

#include <stdint.h>

struct arena {
  size_t element_size;
  size_t cap;
  size_t idx;
  char *ptr;
};

struct arena arena_new(size_t element_size, size_t cap);
void arena_free(struct arena *self);
void *arena_alloc(struct arena *self);
void arena_shrink(struct arena *self);

#endif // __TFIIDF_H_ARENA
