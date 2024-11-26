#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "hash_map.h"

#define test(testfn) testfn();                  \
    printf("[+] %s passed\n", #testfn);

void hash_map_print_char_star_int(struct hash_map *map) {
  printf("Entries in map: %li\n", map->entries);
  for (size_t i = 0; i < map->n_buckets; i++) {
    struct linked_list_node *node = map->buckets[i].root;
    while (node) {
      printf("\t(bucket) %li : (key) %s : (value) %i\n", i, node->key, *(int *)node->value);
      node = node->next;
    }
  }
}

void test_hash_map_basic_inser_get() {
  struct hash_map map = hash_map_new_with_cap(3, hash_map_hash_char_star, hash_map_compare_char_star);
  int a=1, b=2, c=3, d=4, e=5;
  hash_map_insert(&map, "key1", &a, 5, sizeof(int));
  hash_map_insert(&map, "key2", &b, 5, sizeof(int));
  hash_map_insert(&map, "key3", &c, 5, sizeof(int));
  hash_map_insert(&map, "key4", &d, 5, sizeof(int));
  hash_map_insert(&map, "key5", &e, 5, sizeof(int));

  assert(map.entries == 5);

  assert(hash_map_contains(&map, "key1"));
  assert(hash_map_contains(&map, "key2"));
  assert(hash_map_contains(&map, "key3"));
  assert(hash_map_contains(&map, "key4"));
  assert(hash_map_contains(&map, "key5"));

  assert(*(int *)hash_map_get(&map, "key1") == a);
  assert(*(int *)hash_map_get(&map, "key2") == b);
  assert(*(int *)hash_map_get(&map, "key3") == c);
  assert(*(int *)hash_map_get(&map, "key4") == d);
  assert(*(int *)hash_map_get(&map, "key5") == e);

  hash_map_free(&map);
}

int main() {
  test(test_hash_map_basic_inser_get);

  return 0;
}
