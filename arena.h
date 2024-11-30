#ifndef __TFIDF_H_ARENA
#define __TFIDF_H_ARENA

struct arena {
  size_t element_size;
  size_t cap;
  size_t idx;
  char *ptr;
};

struct arena arena_new(size_t element_size, size_t cap);
void arena_free(struct arena *self);
void *arena_alloc(struct arena *self);
/* void extend_and_reset(struct arena *self, size_t new_cap); */

#endif // __TFIIDF_H_ARENA
