#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "skvs.h"

struct skvs_reader skvs_reader_new(char *path) {
  struct skvs_reader reader = {.file = fopen(path, "rb")};
  return reader;
}

void skvs_reader_free(struct skvs_reader *self) { fclose(self->file); }

// TODO: use two buffers for reading to reduce calls to malloc
void skvs_reader_next_pair(struct skvs_reader *self, struct skvs_pair *pair,
                           bool null_term) {
  pair->key_len = 0;
  pair->key = NULL;
  pair->value_len = 0;
  pair->value = NULL;

  char lenbuf[4];
  if (fread(&lenbuf, 1, 4, self->file) < 4)
    return;
  pair->key_len = ntohl(*(uint32_t *)lenbuf);

  pair->key = malloc_checked(pair->key_len + (null_term ? 1 : 0));
  if (fread(pair->key, 1, pair->key_len, self->file) < pair->key_len)
    return;
  if (null_term)
    pair->key[pair->key_len] = '\0';

  if (fread(&lenbuf, 1, 4, self->file) < 4)
    return;
  pair->value_len = ntohl(*(uint32_t *)lenbuf);

  pair->value = malloc_checked(pair->value_len + (null_term ? 1 : 0));
  if (fread(pair->value, 1, pair->value_len, self->file) < pair->value_len)
    return;
  if (null_term)
    pair->value[pair->value_len] = '\0';
}

bool skvs_pair_ok(struct skvs_pair *self) {
  return self->key_len != 0 && self->key != NULL && self->value_len != 0 &&
         self->value != NULL;
}

void skvs_pair_free(struct skvs_pair *self) {
  self->key_len = 0;
  self->value_len = 0;
  if (self->key)
    free(self->key);
  if (self->value)
    free(self->value);
}
