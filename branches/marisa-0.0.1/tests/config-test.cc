#include <cassert>

#include <marisa/config.h>

int main() {
  {
    marisa::Config config(0, false, true, true);
    assert(config.is_last_trie(0));
    assert(config.max_num_tries() == 0);
    assert(config.patricia() == false);
    assert(config.tail() == true);
    assert(config.weight_order() == true);
  }

  {
    marisa::Config config(1, true, false, true);
    assert(config.is_last_trie(0));
    assert(config.max_num_tries() == 1);
    assert(config.patricia() == true);
    assert(config.tail() == false);
    assert(config.weight_order() == true);
  }

  {
    marisa::Config config(2, true, true, false);
    assert(!config.is_last_trie(0));
    assert(config.is_last_trie(1));
    assert(config.max_num_tries() == 2);
    assert(config.patricia() == true);
    assert(config.tail() == true);
    assert(config.weight_order() == false);
  }

  return 0;
}
