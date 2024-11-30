#ifndef __TFIDF_H_SKVS
#define __TFIDF_H_SKVS

// Simple key value store data format

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct skvs_reader {
  FILE *file;
};

struct skvs_pair {
  uint32_t key_len;
  char *key;
  uint32_t value_len;
  char *value;
};

// Caller is responsible to check that .file is valid
struct skvs_reader skvs_reader_new(char *path);
void skvs_reader_free(struct skvs_reader *self);
// Caller is responsible to free() pair.key and pair.value after use
void skvs_reader_next_pair(struct skvs_reader *self, struct skvs_pair *pair,
                           bool null_term);
bool skvs_pair_ok(struct skvs_pair *self);
void skvs_pair_free(struct skvs_pair *self);

#endif // __TFIDF_H_SKVS
