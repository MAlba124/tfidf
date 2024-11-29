#ifndef __TFIDF_H_TOKENIZER
#define __TFIDF_H_TOKENIZER

#include <stdint.h>

#include "hash_map.h"

struct tokenizer {
  struct hash_map_cu32 map;
  uint32_t counter;
};

struct tokenizer tokenizer_new();
void tokenizer_free(struct tokenizer *self);
uint32_t tokenizer_add(struct tokenizer *self, char *dat);
uint32_t *tokenizer_get(struct tokenizer *self, char *dat);
/* int tokenizer_save_to_file(struct tokenizer *self, char *dst_path); */

#endif // __TFIDF_H_TOKENIZER
