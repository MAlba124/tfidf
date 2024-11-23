#include <string.h>
#include <assert.h>

#include "tokenizer.h"
#include "hash_map.h"

struct tokenizer tokenizer_new() {
  struct tokenizer t = {
    .map = hash_map_new(hash_map_hash_char_star, hash_map_compare_char_star),
    .counter = 0,
  };
  return t;
}

void tokenizer_free(struct tokenizer *self) {
  hash_map_free(&self->map);
}

uint64_t tokenizer_add(struct tokenizer *self, char *dat) {
  void *possible_token = hash_map_get(&self->map, dat);
  if (possible_token != NULL)
    return *(uint64_t *)possible_token;

  uint64_t token = self->counter;
  hash_map_insert(&self->map, dat, &token, strlen(dat) + 1, sizeof(uint64_t));
  self->counter++;
  return token;
}

uint64_t *tokenizer_get(struct tokenizer *self, char *dat) {
  void *token = hash_map_get(&self->map, dat);
  return (uint64_t *)token;
}
