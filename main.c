// TODO: use arenas for all the maps :===)))))
// TODO: test against real life tf-idf and cosine similarity tests

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

struct search_result {
  char *src;
  float score;
};

struct arguments {
  char *src_file;
  bool interactive;
  char *query;
  size_t n_results;
};

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

static inline struct hash_map_u32f
create_tf_map(struct tokenizer *tokizer, struct hash_map_u32f *idf, char *src) {
  struct hash_map_u32f corpus_map = hash_map_u32f_new_with_cap(32);

  char *term_element = src;
  char *tmp;

  float corpus_length = 0;
  float most_freq = 0.0;
  while ((tmp = parse_next_element_in_str(term_element)) != NULL) {
    if (*term_element != '\0') {
      uint32_t tok = tokenizer_add(tokizer, term_element);
      float *tok_tf = hash_map_u32f_get_or_insert(&corpus_map, tok, 0.0);
      hash_map_u32f_insert(idf, tok, 1.0);
      (*tok_tf)++;
      if (*tok_tf > most_freq)
        most_freq = *tok_tf;
      corpus_length++;
    }
    term_element = tmp;
  }

  // Calculate TF
  for (size_t i = 0; i < corpus_map.n_buckets; i++) {
    struct hash_map_bucket_node_u32f *nod = corpus_map.buckets[i].root;
    while (nod) {
      nod->value /= corpus_length;
      nod = nod->next;
    }
  }

  return corpus_map;
}

static inline void sort_and_print_results(struct search_result *results,
                                          size_t count) {
  bool swapped = true;
  size_t n = count;
  // Sort the results
  while (swapped) {
    swapped = false;
    for (size_t i = 1; i < n; i++) {
      if (results[i - 1].score > results[i].score) {
        struct search_result swapee = results[i - 1];
        results[i - 1] = results[i];
        results[i] = swapee;
        swapped = true;
      }
    }
    n--;
  }

  for (size_t i = count; i > 0; i--) {
    float s = results[i - 1].score;
    if (s > 0.0)
      printf("%2ld. | Score: %s%.0f%%\033[0m Title: %s\n", count - i + 1,
             s >= 0.75   ? "\033[32m"
             : s >= 0.25 ? "\033[33m"
                         : "\033[31m",
             s * 100.0, results[i - 1].src);
  }
}

struct arguments parse_args(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s [OPTION] -s [FILE]\n", argv[0]);
    exit(1);
  }

  struct arguments args = {
      .src_file = NULL, .interactive = false, .query = NULL, .n_results = 10};

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0) {
      if (args.query != NULL) {
        printf("%s: Incompatible options '-i' and '-q'\n", argv[0]);
        exit(1);
      }
      args.interactive = true;
    } else if (strcmp(argv[i], "--help") == 0) {
      printf("soon\n");
      exit(0);
    } else if (strcmp(argv[i], "-s") == 0) {
      if (i >= argc - 1) {
        printf("%s: Option '-s' requires an argument\n", argv[0]);
        exit(1);
      }
      args.src_file = argv[++i];
    } else if (strcmp(argv[i], "-q") == 0) {
      if (i >= argc - 1) {
        printf("%s: Option '-q' requires an argument\n", argv[0]);
        exit(1);
      }
      if (args.interactive) {
        printf("%s: Incompatible options '-q' and '-i'\n", argv[0]);
        exit(1);
      }
      args.query = argv[++i];
    } else if (strcmp(argv[i], "-n") == 0) {
      if (i >= argc - 1) {
        printf("%s: Option '-n' requires an argument\n", argv[0]);
        exit(1);
      }
      char *eptr = NULL;
      long n = strtol(argv[++i], &eptr, 10);
      if (*eptr != '\0') {
        printf("%s: Could not convert number of results\n", argv[0]);
        exit(1);
      }
      args.n_results = (size_t)n;
    } else {
      printf("%s: Invalid option '%s'\n", argv[0], argv[i]);
      exit(1);
    }
  }

  if (!args.src_file) {
    printf("%s: Missing corpus\n", argv[0]);
    exit(1);
  }

  if (!args.n_results) {
    exit(0);
  }

  if (!(!args.interactive ^ !args.query)) {
    printf("%s: Missing mode of operation('-i' or '-q')\n", argv[0]);
    exit(1);
  }

  return args;
}

static void single_search(struct array_list *corpus, struct tokenizer *tokizer,
                          struct hash_map_u32f *idf, char *query,
                          size_t n_results) {
  struct hash_map_u32f query_tokens = create_tf_map(tokizer, idf, query);
  size_t n_tokens = query_tokens.entries;
  uint32_t *tokens = malloc_checked(sizeof(uint32_t) * n_tokens);
  size_t tokens_i = 0;
  for (size_t i = 0; i < query_tokens.n_buckets; i++) {
    struct hash_map_bucket_node_u32f *node = query_tokens.buckets[i].root;
    while (node != NULL) {
      tokens[tokens_i++] = node->key;
      node = node->next;
    }
  }

  struct search_result top_10[n_results];

  if (n_tokens == 0) {
    printf("No results\n");
    goto clean;
  }

  for (size_t i = 0; i < n_results; i++) {
    top_10[i].score = 0.0;
    top_10[i].src = NULL;
  }

  bool found_matching = false;

  for (size_t i = 0; i < corpus->size; i++) {
    float document_vector[n_tokens];
    float query_vector[n_tokens];
    size_t vector_idx = 0;

    struct array_list_pair *doc = &corpus->data[i];
    for (size_t j = 0; j < n_tokens; j++) {
      float *tf_idf = hash_map_u32f_get(&doc->map, tokens[j]);
      void *query_tf = hash_map_u32f_get(&query_tokens, tokens[j]);
      void *idfa = hash_map_u32f_get(idf, tokens[j]);
      // Save elements if they intersect
      if (tf_idf != NULL && idfa != NULL && query_tf != NULL) {
        document_vector[vector_idx] = *(float *)tf_idf;
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
      // Multiply by the percentage of the intersected elements
      score = (dot_prod / magn_prod) * ((float)vector_idx / (float)n_tokens);
    }

    // Insert the found score to the top10 list if it belongs there
    size_t lowest_idx = 0;
    for (size_t j = 1; j < n_results; j++)
      if (top_10[j].score < top_10[lowest_idx].score)
        lowest_idx = j;
    if (top_10[lowest_idx].score < score)
      top_10[lowest_idx] =
          (struct search_result){.src = doc->location, .score = score};
  }

  if (!found_matching) {
    printf("No results\n");
    goto clean;
  }

  sort_and_print_results((struct search_result *)&top_10, n_results);

clean:
  hash_map_u32f_free(&query_tokens);
  free(tokens);
}

static void interactive_search(struct array_list *corpus,
                               struct tokenizer *tokizer,
                               struct hash_map_u32f *idf, size_t n_results) {
  while (true) {
    char *query = readline("query > ");
    if (query == NULL)
      break;

    single_search(corpus, tokizer, idf, query, n_results);

    free(query);
  }
}

int main(int argc, char **argv) {
  struct arguments args = parse_args(argc, argv);

  struct skvs_reader corpus_reader = skvs_reader_new(args.src_file);
  if (corpus_reader.file == NULL) {
    printf("Could not open '%s'\n", args.src_file);
    return 1;
  }

  struct array_list corpus = array_list_new(100000);
  struct tokenizer tokizer = tokenizer_new();
  struct hash_map_u32f idf = hash_map_u32f_new();
  float entries = 0;

  size_t print_counter = 0;
  time_t start_time = time(NULL);
  while (true) {
    struct skvs_pair pair;
    skvs_reader_next_pair(&corpus_reader, &pair, true);
    if (!skvs_pair_ok(&pair))
      break;

    // TODO: Might want to shrink the maps arena. This involves realloc()ing so
    // only required memory is used + calculating the offsets for the node
    // addresses (should be a simple offset = old_ptr - new_ptr, then update the
    // *nodes and *next)

    array_list_push(&corpus,
                    (struct array_list_pair){
                        .location = pair.key,
                        .map = create_tf_map(&tokizer, &idf, pair.value)});
    free(pair.value);

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
  array_list_shrink_to_fit(&corpus);

  // Calculate idf
  for (size_t i = 0; i < idf.n_buckets; i++) {
    struct hash_map_bucket_node_u32f *node = idf.buckets[i].root;
    while (node) {
      float df = node->value;
      node->value = log10f(entries / df);
      node = node->next;
    }
  }

  // Calculate tf-idf
  for (size_t i = 0; i < corpus.size; i++) {
    struct array_list_pair *doc = &corpus.data[i];
    for (size_t j = 0; j < doc->map.n_buckets; j++) {
      struct hash_map_bucket_node_u32f *node = doc->map.buckets[j].root;
      while (node != NULL) {
        float *__idf =
            hash_map_u32f_get(&idf, node->key); // Assumes the idf exist
        node->value *= *__idf;
        node = node->next;
      }
    }
  }

  if (args.interactive)
    interactive_search(&corpus, &tokizer, &idf, args.n_results);
  else
    single_search(&corpus, &tokizer, &idf, args.query, args.n_results);

  hash_map_u32f_free(&idf);
  array_list_free(&corpus);
  tokenizer_free(&tokizer);

  return 0;
}
