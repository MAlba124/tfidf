#ifndef __TFIDF_H_TOKENIZER
#define __TFIDF_H_TOKENIZER

#include <stdint.h>

#include "hash_map.h"

struct tokenizer {
  struct hash_map map;
  uint64_t counter;
};

struct tokenizer tokenizer_new();
void tokenizer_free(struct tokenizer *self);
uint64_t tokenizer_add(struct tokenizer *self, char *dat);
uint64_t *tokenizer_get(struct tokenizer *self, char *dat);
/* int tokenizer_save_to_file(struct tokenizer *self, char *dst_path); */

#endif // __TFIDF_H_TOKENIZER
