#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "arraylist.h"
#include "hash_map.h"
#include "skvs.h"
#include "tokenizer.h"

static uint64_t hash_uint64_t(const void *key) {
  return *(uint64_t *)key;
}

static bool compare_uint64_t(const void *lhs, const void *rhs) {
  return *(uint64_t *)lhs == *(uint64_t *)rhs;
}

int main() {
  struct skvs_reader corpus_reader = skvs_reader_new("reddit_btc_test/comments.skvs");
  if (corpus_reader.file == NULL) {
    printf("Could not open reddit_btc_test/comments.skvs\n");
    return 1;
  }

  printf("Opened corpus at reddit_btc_test/comments.skvs\n");

  struct array_list corpus = array_list_new(100000);
  struct tokenizer tokizer = tokenizer_new();
  struct hash_map idf = hash_map_new(hash_uint64_t, compare_uint64_t);
  struct skvs_pair pair;
  float entries = 0;

  while (1) {
    skvs_reader_next_pair(&corpus_reader, &pair, true);
    if (!skvs_pair_ok(&pair))
      break;

    // Create tokenizer, make hash map for each value of this pair with TF and make seprate hash map for IDF globally for all the bodies :==)
    // hash map for doc should be <uint64_t, float>

    struct hash_map corpus_map = hash_map_new(hash_uint64_t, compare_uint64_t);
    char *pair_value_ptr = pair.value;
    char *this_start = pair.value;
    while (*pair.value != '\0') {
      if (*pair.value == ' ') {
        *pair.value = '\0';
        if (this_start != pair.value) {
          uint64_t tok = tokenizer_add(&tokizer, this_start);
          float zero = 0.0;
          void *tok_tf = hash_map_get(&corpus_map, &tok);
          if (tok_tf != NULL)
            *(float *)tok_tf += 1.0;
          else
            hash_map_insert(&corpus_map, &tok, &zero, sizeof(uint64_t), sizeof(float));

          void *tok_idf = hash_map_get(&idf, &tok);
          if (tok_idf != NULL)
            *(float *)tok_idf += 1.0;
          else
            hash_map_insert(&idf, &tok, &zero, sizeof(uint64_t), sizeof(float));
        }
        pair.value++;
        while (*pair.value == ' ' && *pair.value != '\0') {
          pair.value++;
        }
        this_start = pair.value;
        continue;
      }
      pair.value++;
    }

    free(pair_value_ptr);

    struct array_list_pair corpus_all = {
      .location = pair.key,
      .map = corpus_map
    };

    array_list_push(&corpus, &corpus_all);

    entries++;
  }

  printf("Entries: %f\n", entries);

  skvs_reader_free(&corpus_reader);

  /* printf("%s\n", corpus.data[0].key); */

  hash_map_free(&idf);
  array_list_free(&corpus);
  tokenizer_free(&tokizer);

  return 0;
}
