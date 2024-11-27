#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <readline/readline.h>

#include "arraylist.h"
#include "hash_map.h"
#include "mem.h"
#include "skvs.h"
#include "tokenizer.h"

void hash_map_print_uint32_float(struct hash_map *map) {
  printf("Entries in map: %li\n", map->entries);
  for (size_t i = 0; i < map->n_buckets; i++) {
    struct linked_list_node *node = map->buckets[i].root;
    while (node) {
      printf("\t(bucket) %li : (key) %i : (value) %f\n", i,
             *(uint32_t *)node->key, *(float *)node->value);
      node = node->next;
    }
  }
}

struct search_result {
  char *src;
  float score;
};

static uint32_t hash_uint32_t(const void *key) { return *(uint32_t *)key; }

static bool compare_uint32_t(const void *lhs, const void *rhs) {
  return *(uint32_t *)lhs == *(uint32_t *)rhs;
}

static inline char *parse_next_element_in_str(char *str) {
  if (*str == '\0')
    return NULL;

  char *write = str;
  char *read = str;
  bool done = false;

  while (!done && *read != '\0') {
    switch (*read) {
    case 'A' ... 'Z':
      *write++ = tolower(*read++);
      break;
    case '0' ... '9':
    case 'a' ... 'z':
      *write++ = *read++;
      break;
    default:
      *write = '\0';
      read++;
      done = true;
      break;
    }
  }

  return read;
}

int main() {
  struct skvs_reader corpus_reader =
      skvs_reader_new("reddit_btc_test/comments.skvs");
  if (corpus_reader.file == NULL) {
    printf("Could not open reddit_btc_test/comments.skvs\n");
    return 1;
  }

  printf("Opened corpus at reddit_btc_test/comments.skvs\n");

  struct array_list corpus = array_list_new(100000);
  struct tokenizer tokizer = tokenizer_new();
  struct hash_map idf = hash_map_new(hash_uint32_t, compare_uint32_t);
  struct skvs_pair pair;
  float entries = 0;

  size_t print_counter = 0;
  time_t start_time = time(NULL);
  float zero = 0.0;
  float one = 1.0;
  /* float max_tf = 1.0; */

  while (true) {
    skvs_reader_next_pair(&corpus_reader, &pair, true);
    if (!skvs_pair_ok(&pair))
      break;

    struct hash_map corpus_map = hash_map_new(hash_uint32_t, compare_uint32_t);

    char *term_element = pair.value;
    char *tmp;

    float corpus_length = 0;
    float most_freq = 0.0;
    while ((tmp = parse_next_element_in_str(term_element)) != NULL) {
      if (*term_element != '\0') {
        uint32_t tok = tokenizer_add(&tokizer, term_element);
        float *tok_tf = (float *)hash_map_get_or_insert(
            &corpus_map, &tok, &zero, sizeof(uint32_t), sizeof(float));
        if (*tok_tf == 0.0)
          corpus_length++;
        hash_map_insert(&idf, &tok, &one, sizeof(uint32_t), sizeof(float));
        (*tok_tf)++;
        if (*tok_tf > most_freq)
          most_freq = *tok_tf;
      }
      term_element = tmp;
    }

    free(pair.value);

    // Calculate TF
    for (size_t i = 0; i < corpus_map.n_buckets; i++) {
      struct linked_list_node *nod = corpus_map.buckets[i].root;
      while (nod) {
        /* *(float *)nod->value = 1.0 + logf(*(float *)nod->value); */
        /* *(float *)nod->value = 0.5 + 0.5 * (*(float *)nod->value /
         * most_freq); */
        *(float *)nod->value /= corpus_length;
        nod = nod->next;
      }
    }

    struct array_list_pair corpus_all = {.location = pair.key,
                                         .map = corpus_map};

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

  // Calculate idf
  for (size_t i = 0; i < idf.n_buckets; i++) {
    struct linked_list_node *node = idf.buckets[i].root;
    while (node) {
      float df = *(float *)node->value;
      *(float *)node->value = log10f(entries / df);
      node = node->next;
    }
  }

  array_list_shrink_to_fit(&corpus);

  while (true) {
    char *query = readline("query > ");
    if (query == NULL)
      break;

    uint32_t *tokens = malloc_checked(0);
    struct hash_map query_tokens =
        hash_map_new(hash_uint32_t, compare_uint32_t);
    size_t n_tokens = 0;

    char *term_element = query;
    char *tmp;
    while ((tmp = parse_next_element_in_str(term_element)) != NULL) {
      if (*term_element != '\0') {
        uint32_t *tok = tokenizer_get(&tokizer, term_element);
        if (tok != NULL) {
          float *tok_tf = (float *)hash_map_get_or_insert(
              &query_tokens, tok, &zero, sizeof(uint32_t), sizeof(float));
          if (*tok_tf == 0.0) {
            n_tokens++;
            tokens = realloc_checked(tokens, sizeof(uint32_t) * n_tokens);
            tokens[n_tokens - 1] = *tok;
          }
          (*tok_tf)++;
        }
      }

      term_element = tmp;
    }

    for (size_t j = 0; j < query_tokens.n_buckets; j++) {
      struct linked_list_node *node = query_tokens.buckets[j].root;
      while (node) {
        /* *(float *)node->value = logf(1.0 + *(float *)node->value); */
        *(float *)node->value /= (float)n_tokens;
        node = node->next;
      }
    }

    if (n_tokens == 0) {
      printf("No results\n");
      goto clean;
    }

    struct search_result top_10[10];
    for (size_t i = 0; i < 10; i++) {
      top_10[i].score = 0.0;
      top_10[i].src = NULL;
    }

    bool found_matching = false;

    // TF-IDF for document
    for (size_t i = 0; i < corpus.size; i++) {
      float document_vector[n_tokens];
      float query_vector[n_tokens];
      size_t vector_idx = 0;

      struct array_list_pair *doc = &corpus.data[i];
      for (size_t j = 0; j < n_tokens; j++) {
        void *tf = hash_map_get(&doc->map, &tokens[j]);
        void *query_tf = hash_map_get(&query_tokens, &tokens[j]);
        void *idfa = hash_map_get(&idf, &tokens[j]);
        if (tf != NULL && idfa != NULL && query_tf != NULL) {
          document_vector[vector_idx] = *(float *)tf * *(float *)idfa;
          query_vector[vector_idx++] = *(float *)query_tf * *(float *)idfa;
          found_matching = true;
        }
      }

      if (vector_idx == 0)
        continue;

      // Cosine similarity
      float score = 0.0;
      float dot_prod = 0.0;
      float magn_prod = 0.0;
      float doc_norm = 0.0;
      float query_norm = 0.0;
      for (size_t j = 0; j < vector_idx; j++) {
        dot_prod += document_vector[j] * query_vector[j];
        query_norm += query_vector[j] * query_vector[j];
        doc_norm += document_vector[j] * document_vector[j];
      }
      query_norm = sqrtf(query_norm);
      doc_norm = sqrtf(doc_norm);
      magn_prod = doc_norm * query_norm;
      if (magn_prod != 0.0) {
        score = dot_prod / magn_prod;
        score *= (float)vector_idx / (float)n_tokens;
      }

      // Insert the found score to the top10 list if it belongs there
      size_t lowest_idx = 0;
      for (size_t j = 1; j < 10; j++)
        if (top_10[j].score < top_10[lowest_idx].score)
          lowest_idx = j;
      if (top_10[lowest_idx].score < score) {
        struct search_result da_result = {.src = doc->location, .score = score};
        top_10[lowest_idx] = da_result;
      }
    }

    if (!found_matching) {
      printf("No results\n");
      goto clean;
    }

    bool swapped = true;
    size_t n = 10;
    // Sort the results
    while (swapped) {
      swapped = false;
      for (size_t i = 1; i < n; i++) {
        if (top_10[i - 1].score > top_10[i].score) {
          struct search_result swapee = top_10[i - 1];
          top_10[i - 1] = top_10[i];
          top_10[i] = swapee;
          swapped = true;
        }
      }
      n--;
    }

    for (size_t i = 10; i > 0; i--) {
      float s = top_10[i - 1].score;
      if (top_10[i - 1].score > 0.0)
        printf("%2ld. | Score: %s%.0f%%\033[0m URL: %s\n", 10 - i + 1,
               s >= 0.75   ? "\033[32m"
               : s >= 0.25 ? "\033[33m"
                           : "\033[31m",
               s * 100.0, top_10[i - 1].src);
    }

  clean:
    hash_map_free(&query_tokens);
    free(tokens);
    free(query);
  }

  hash_map_free(&idf);
  array_list_free(&corpus);
  tokenizer_free(&tokizer);

  return 0;
}
