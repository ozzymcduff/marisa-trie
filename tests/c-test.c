#include <marisa.h>

#include "assert.h"

int main(void) {
  marisa_trie *trie = NULL;
  ASSERT(marisa_init(&trie) == MARISA_OK);
  const char *keys[] = { "apple", "and", "bad", "app" };
  ASSERT(marisa_build(trie, keys, 4, NULL, NULL, NULL, 0) == MARISA_OK);
  int num_tries;
  ASSERT(marisa_get_num_tries(trie, &num_tries) == MARISA_OK);
  marisa_uint32 num_keys;
  ASSERT(marisa_get_num_keys(trie, &num_keys) == MARISA_OK);
  marisa_uint32 num_nodes;
  ASSERT(marisa_get_num_nodes(trie, &num_nodes) == MARISA_OK);
  size_t total_size;
  ASSERT(marisa_get_total_size(trie, &total_size) == MARISA_OK);
  printf("%d, %u, %u, %zu\n", num_tries, num_keys, num_nodes, total_size);
  ASSERT(marisa_end(trie) == MARISA_OK);
  return 0;
}
