#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hash_map.h"

#define test(testfn)                                                           \
  testfn();                                                                    \
  printf("\033[32m[+]\033[0m %s \033[32mpassed\033[0m\n", #testfn);

void test_hash_map_cf_insert_get() {
  struct hash_map_cf map = hash_map_cf_new_with_cap(3);
  float a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
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

void test_hash_map_cu32_insert_get() {
  struct hash_map_cu32 map = hash_map_cu32_new_with_cap(3);
  uint32_t a = 1, b = 2, c = 3, d = 4, e = 5;
  hash_map_cu32_insert(&map, "key1", a, 5);
  hash_map_cu32_insert(&map, "key2", b, 5);
  hash_map_cu32_insert(&map, "key3", c, 5);
  hash_map_cu32_insert(&map, "key4", d, 5);
  hash_map_cu32_insert(&map, "key5", e, 5);

  assert(map.entries == 5);

  assert(hash_map_cu32_contains(&map, "key1"));
  assert(hash_map_cu32_contains(&map, "key2"));
  assert(hash_map_cu32_contains(&map, "key3"));
  assert(hash_map_cu32_contains(&map, "key4"));
  assert(hash_map_cu32_contains(&map, "key5"));

  assert(*hash_map_cu32_get(&map, "key1") == a);
  assert(*hash_map_cu32_get(&map, "key2") == b);
  assert(*hash_map_cu32_get(&map, "key3") == c);
  assert(*hash_map_cu32_get(&map, "key4") == d);
  assert(*hash_map_cu32_get(&map, "key5") == e);

  hash_map_cu32_free(&map);
}

void test_hash_map_cu32_insert_get_big() {
  struct hash_map_cu32 map = hash_map_cu32_new();

  char str[8];
  size_t upper = 1000000;
  for (size_t i = 0; i < upper; i++) {
    sprintf(str, "%li", i);
    hash_map_cu32_insert(&map, str, (uint32_t)(upper - i), strlen(str) + 1);
  }

  assert(map.entries == upper);

  for (size_t i = 0; i < upper; i++) {
    sprintf(str, "%li", i);
    assert(*hash_map_cu32_get(&map, str) == (uint32_t)(upper - i));
  }

  hash_map_cu32_free(&map);
}

void test_hash_map_u32f_insert_get() {
  struct hash_map_u32f map = hash_map_u32f_new_with_cap(3);
  float a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
  hash_map_u32f_insert(&map, 1, a);
  hash_map_u32f_insert(&map, 2, b);
  hash_map_u32f_insert(&map, 3, c);
  hash_map_u32f_insert(&map, 4, d);
  hash_map_u32f_insert(&map, 5, e);

  assert(map.entries == 5);

  assert(hash_map_u32f_contains(&map, 1));
  assert(hash_map_u32f_contains(&map, 2));
  assert(hash_map_u32f_contains(&map, 3));
  assert(hash_map_u32f_contains(&map, 4));
  assert(hash_map_u32f_contains(&map, 5));

  assert(*hash_map_u32f_get(&map, 1) == a);
  assert(*hash_map_u32f_get(&map, 2) == b);
  assert(*hash_map_u32f_get(&map, 3) == c);
  assert(*hash_map_u32f_get(&map, 4) == d);
  assert(*hash_map_u32f_get(&map, 5) == e);

  hash_map_u32f_free(&map);
}

void test_hash_map_u32f_insert_get_big() {
  struct hash_map_u32f map = hash_map_u32f_new();

  size_t upper = 1000000;
  for (size_t i = 0; i < upper; i++) {
    hash_map_u32f_insert(&map, (uint32_t)i, (float)(upper - i));
  }

  assert(map.entries == upper);

  for (size_t i = 0; i < upper; i++) {
    assert(*hash_map_u32f_get(&map, (uint32_t)i) == (float)(upper - i));
  }

  hash_map_u32f_free(&map);
}

int main() {
  test(test_hash_map_cf_insert_get);
  test(test_hash_map_cf_insert_get_big);
  test(test_hash_map_cu32_insert_get);
  test(test_hash_map_cu32_insert_get_big);
  test(test_hash_map_u32f_insert_get);
  test(test_hash_map_u32f_insert_get_big);
  return 0;
}
