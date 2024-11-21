#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "map.h"

uint64_t hash(const void *key) {
  const char *str = key;
  uint64_t sum = 0;
  for (size_t i = 0; i < strlen(key); i++) {
    sum += str[i];
  }
  return sum;
}

bool compare(const void *vlhs, const void *vrhs) {
  const char *lhs = vlhs;
  const char *rhs = vrhs;
  const size_t lhs_len = strlen(lhs);
  const size_t rhs_len = strlen(rhs);

  if (lhs_len != rhs_len)
    return false;

  return strcmp(lhs, rhs) == 0;
}

int main() {
  struct hash_map map = hash_map_new_with_cap(3, hash, compare);
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

  return 0;
}
