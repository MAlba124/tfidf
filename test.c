#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "hash_map.h"

#define test(testfn) testfn();                  \
    printf("\033[32m[+]\033[0m %s \033[32mpassed\033[0m\n", #testfn);

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

void test_hash_map_basic_insert_get() {
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

void test_hash_map_basic_insert_get_big() {
  struct hash_map map = hash_map_new(hash_map_hash_char_star, hash_map_compare_char_star);

  char str[7];
  size_t upper = 100000;
  for (size_t i = 0; i < upper; i++) {
    sprintf(str, "%li", i);
    int val = upper - i;
    hash_map_insert(&map, str, &val, strlen(str) + 1, sizeof(int));
  }

  assert(map.entries == upper);

  for (size_t i = 0; i < upper; i++) {
    sprintf(str, "%li", i);
    assert(*(int *)hash_map_get(&map, str) == (int)(upper - i));
  }
  hash_map_free(&map);
}

void test_hash_map_cf_insert_get() {
  struct hash_map_cf map = hash_map_cf_new_with_cap(3);
  float a=1.1, b=2.2, c=3.3, d=4.4, e=5.5;
  hash_map_cf_insert(&map, "key1", a, 5);
  hash_map_cf_insert(&map, "key2", b, 5);
  hash_map_cf_insert(&map, "key3", c, 5);
  hash_map_cf_insert(&map, "key4", d, 5);
  hash_map_cf_insert(&map, "key5", e, 5);

  assert(map.entries == 5);

  assert(hash_map_cf_contains(&map, "key1"));
  assert(hash_map_cf_contains(&map, "key2"));
  assert(hash_map_cf_contains(&map, "key3"));
  assert(hash_map_cf_contains(&map, "key4"));
  assert(hash_map_cf_contains(&map, "key5"));

  assert(*hash_map_cf_get(&map, "key1") == a);
  assert(*hash_map_cf_get(&map, "key2") == b);
  assert(*hash_map_cf_get(&map, "key3") == c);
  assert(*hash_map_cf_get(&map, "key4") == d);
  assert(*hash_map_cf_get(&map, "key5") == e);

  hash_map_cf_free(&map);
}

void test_hash_map_cf_insert_get_big() {
  struct hash_map_cf map = hash_map_cf_new();

  char str[8];
  size_t upper = 1000000;
  for (size_t i = 0; i < upper; i++) {
    sprintf(str, "%li", i);
    hash_map_cf_insert(&map, str, (float)(upper - i), strlen(str) + 1);
  }

  assert(map.entries == upper);

  for (size_t i = 0; i < upper; i++) {
    sprintf(str, "%li", i);
    assert(*hash_map_cf_get(&map, str) == (float)(upper - i));
  }

  hash_map_cf_free(&map);
}

int main() {
  test(test_hash_map_basic_insert_get);
  test(test_hash_map_cf_insert_get);
  test(test_hash_map_basic_insert_get_big);
  test(test_hash_map_cf_insert_get_big);
  return 0;
}
