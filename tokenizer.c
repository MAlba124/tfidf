#include <string.h>
#include <assert.h>

#include "tokenizer.h"
#include "hash_map.h"

struct tokenizer tokenizer_new() {
  struct tokenizer t = {
    .map = hash_map_cu32_new_with_cap(1024*2),
    .counter = 0,
  };
  return t;
}

void tokenizer_free(struct tokenizer *self) {
  hash_map_cu32_free(&self->map);
}

uint32_t tokenizer_add(struct tokenizer *self, char *dat) {
  void *possible_token = hash_map_cu32_get(&self->map, dat);
  if (possible_token != NULL)
    return *(uint32_t *)possible_token;

  uint32_t token = self->counter++;
  hash_map_cu32_insert(&self->map, dat, token, strlen(dat) + 1);
  return token;
}

uint32_t *tokenizer_get(struct tokenizer *self, char *dat) {
  uint32_t *token = hash_map_cu32_get(&self->map, dat);
  return token;
}
