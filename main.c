#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <readline/readline.h>

#include "arraylist.h"
#include "hash_map.h"
#include "mem.h"
#include "skvs.h"
#include "tokenizer.h"

static uint64_t hash_uint64_t(const void *key) {
  return *(uint64_t *)key;
}

static bool compare_uint64_t(const void *lhs, const void *rhs) {
  return *(uint64_t *)lhs == *(uint64_t *)rhs;
}

struct search_result {
  char *src;
  float score;
};

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

  size_t print_counter = 0;
  time_t start_time = time(NULL);

  while (1) {
    skvs_reader_next_pair(&corpus_reader, &pair, true);
    if (!skvs_pair_ok(&pair))
      break;

    struct hash_map corpus_map = hash_map_new(hash_uint64_t, compare_uint64_t);
    char *pair_value_ptr = pair.value;
    char *this_start = pair.value;
    // I think this includes space characters in some cases
    while (*pair.value != '\0') {
      if (*pair.value == ' ') {
        *pair.value = '\0';
        if (this_start != pair.value) {
          uint64_t tok = tokenizer_add(&tokizer, this_start);
          float one = 1.0;
          void *tok_tf = hash_map_get(&corpus_map, &tok);
          if (tok_tf != NULL) {
            *(float *)tok_tf += 1.0;
            void *tok_idf = hash_map_get(&idf, &tok);
            if (tok_idf != NULL)
                *(float *)tok_idf += 1.0;
            else
                hash_map_insert(&idf, &tok, &one, sizeof(uint64_t), sizeof(float));
          }
          else
            hash_map_insert(&corpus_map, &tok, &one, sizeof(uint64_t), sizeof(float));
        }
        pair.value++;
        while (*pair.value == ' ' && *pair.value != '\0') {
          pair.value++;
        }
        this_start = pair.value;
      } else {
        pair.value++;
      }
    }

    if (this_start != pair.value) {
      uint64_t tok = tokenizer_add(&tokizer, this_start);
      float one = 1.0;
      void *tok_tf = hash_map_get(&corpus_map, &tok);
      if (tok_tf != NULL) {
        *(float *)tok_tf += 1.0;
        void *tok_idf = hash_map_get(&idf, &tok);
        if (tok_idf != NULL)
          *(float *)tok_idf += 1.0;
        else
          hash_map_insert(&idf, &tok, &one, sizeof(uint64_t), sizeof(float));
      }
      else
      hash_map_insert(&corpus_map, &tok, &one, sizeof(uint64_t), sizeof(float));
    }

    free(pair_value_ptr);

    struct array_list_pair corpus_all = {
      .location = pair.key,
      .map = corpus_map
    };

    array_list_push(&corpus, &corpus_all);

    entries++;
    print_counter++;
    if (print_counter == 10000) {
      print_counter = 0;
      time_t elapsed = time(NULL) - start_time;
      printf("\r%.0f entries loaded in %lis", entries, elapsed);
      fflush(stdout);
    }
  }

  time_t elapsed = time(NULL) - start_time;
  printf("\r%.0f entries loaded in %lis\n", entries, elapsed);

  skvs_reader_free(&corpus_reader);

  for (size_t i = 0; i < idf.n_buckets; i++) {
    struct linked_list_node *node = idf.buckets[i].root;
    while (node) {
      float df = *(float *)node->value;
      *(float *)node->value = logf(entries / df);
      node = node->next;
    }
  }

  while (true) {
    char *query = readline("query > ");
    if (query == NULL)
      break;

    uint64_t *tokens = malloc_checked(0);
    size_t n_tokens = 0;

    char *rolling_query = query;
    char *this_start = rolling_query;
    while (*rolling_query != '\0') {
      if (*rolling_query == ' ') {
        *rolling_query = '\0';
        if (this_start != rolling_query) {
          uint64_t *tok = tokenizer_get(&tokizer, this_start);
          if (tok != NULL) {
            n_tokens++;
            tokens = realloc_checked(tokens, sizeof(uint64_t) * n_tokens);
            tokens[n_tokens - 1] = *tok;
          }
        }
        rolling_query++;
        while (*rolling_query == ' ' && *rolling_query != '\0') {
          rolling_query++;
        }
        this_start = rolling_query;
      } else {
        rolling_query++;
      }
    }

    if (this_start != rolling_query) {
      uint64_t *tok = tokenizer_get(&tokizer, this_start);
      if (tok != NULL) {
        n_tokens++;
        tokens = realloc_checked(tokens, sizeof(uint64_t) * n_tokens);
        tokens[n_tokens - 1] = *tok;
      }
    }

    if (n_tokens == 0) {
      printf("No results\n");
      goto clean;
    }

    // TODO: no need to malloc
    struct search_result *top_10 =
        malloc_checked(sizeof(struct search_result) * 10);
    for (size_t i = 0; i < 10; i++) {
      top_10[i].score = 0.0;
    }

    bool found_matching = false;

    for (size_t i = 0; i < corpus.size; i++) {
      struct array_list_pair *doc = &corpus.data[i];
      float score = 0.0;
      for (size_t j = 0; j < n_tokens; j++) {
        void *tf = hash_map_get(&doc->map, &tokens[j]);
        void *idfa = hash_map_get(&idf, &tokens[j]);
        if (tf != NULL && idfa != NULL) {
          score += *(float *)tf * *(float *)idfa;
          found_matching = true;
        }
      }
      size_t lowest_idx = 0;
      for (size_t j = 1; j < 10; j++)
        if (top_10[j].score < top_10[lowest_idx].score)
          lowest_idx = j;
      if (top_10[lowest_idx].score < score) {
        struct search_result da_result = {
          .src = doc->location,
          .score = score
        };
        memcpy(&top_10[lowest_idx], &da_result, sizeof(struct search_result));
      }
    }

    if (!found_matching) {
      printf("No results\n");
      free(top_10);
      goto clean;
    }

    // TODO: sort top_10 before printing

    for (size_t i = 0; i < 10; i++) {
      if (top_10[i].score > 0.0)
        printf("Score: %.3f URL: %s\n", top_10[i].score, top_10[i].src);
    }

    free(top_10);

  clean:
    free(tokens);
    free(query);
  }

  hash_map_free(&idf);
  array_list_free(&corpus);
  tokenizer_free(&tokizer);

  return 0;
}
