#ifndef __TFIDF_H_MEM
#define __TFIDF_H_MEM

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

inline void *malloc_checked(size_t size) {
  void *ptr = malloc(size);
  assert(ptr);
  return ptr;
}

static inline void *realloc_checked(void *ptr, size_t size) {
  void *newptr = realloc(ptr, size);
  assert(newptr);
  return newptr;
}

#endif // __TFIDF_H_MEM
