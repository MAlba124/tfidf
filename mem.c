#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "mem.h"

void *malloc_checked(size_t size) {
  void *ptr = malloc(size);
  assert(ptr);
  return ptr;
}

void *realloc_checked(void *ptr, size_t size) {
  void *newptr = realloc(ptr, size);
  assert(newptr);
  return newptr;
}
