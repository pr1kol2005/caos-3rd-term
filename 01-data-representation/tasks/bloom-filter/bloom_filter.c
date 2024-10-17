#include "bloom_filter.h"

uint64_t calc_hash(const char* str, uint64_t modulus, uint64_t seed) {
  uint64_t hash = 0;
  uint64_t m = 1;

  for (int i = 0; str[i] != '\0'; ++i) {
    int x = (int)(str[i] - ' ' + 1);
    hash = (hash * seed + m * x) % modulus;
  }

  return hash;
}

void bloom_init(struct BloomFilter* bloom_filter, uint64_t set_size,
                hash_fn_t hash_fn, uint64_t hash_fn_count) {
  bloom_filter->set = calloc((set_size + 63) / 64, sizeof(uint64_t));
  bloom_filter->set_size = set_size;
  bloom_filter->hash_fn = hash_fn;
  bloom_filter->hash_fn_count = hash_fn_count;
}

void bloom_destroy(struct BloomFilter* bloom_filter) {
  free(bloom_filter->set);
  bloom_filter->set = NULL;
  bloom_filter->set_size = 0;
  bloom_filter->hash_fn = NULL;
  bloom_filter->hash_fn_count = 0;
}

void bloom_insert(struct BloomFilter* bloom_filter, Key key) {
  for (int i = 0; i < bloom_filter->hash_fn_count; ++i) {
    uint64_t hash = bloom_filter->hash_fn(key, bloom_filter->set_size, i);
    bloom_filter->set[hash / 64] |= (1ull << (hash % 64));
  }
}

bool bloom_check(struct BloomFilter* bloom_filter, Key key) {
  for (int i = 0; i < bloom_filter->hash_fn_count; ++i) {
    uint64_t hash = bloom_filter->hash_fn(key, bloom_filter->set_size, i);
    if (!(bloom_filter->set[hash / 64] & (1ull << (hash % 64)))) {
      return false;
    }
  }
  return true;
}
