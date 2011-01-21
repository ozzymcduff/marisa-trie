#include <cassert>

#include <marisa/progress.h>

int main() {
  {
    marisa::Progress progress(0);
    assert(progress.is_valid());
    while ((progress.trie_id() + 1) < progress.num_tries()) {
      assert(!progress.is_last());
      ++progress;
    }
    assert(progress.is_last());

    assert(progress.flags() == MARISA_DEFAULT_FLAGS);
    assert(progress.trie_id() == progress.num_tries() - 1);
    assert(progress.total_size() == 0);

    progress.test_total_size(0);
    progress.test_total_size(1);
    try {
      progress.test_total_size(MARISA_UINT32_MAX);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_SIZE_ERROR);
    }
    progress.test_total_size(MARISA_UINT32_MAX - 1);
    progress.test_total_size(0);
    try {
      progress.test_total_size(1);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_SIZE_ERROR);
    }

    assert(progress.num_tries() == MARISA_DEFAULT_NUM_TRIES);
    assert(progress.trie() == MARISA_DEFAULT_TRIE);
    assert(progress.tail() == MARISA_DEFAULT_TAIL);
    assert(progress.order() == MARISA_DEFAULT_ORDER);
  }

  {
    marisa::Progress progress(MARISA_DEFAULT_FLAGS);
    assert(progress.is_valid());
    while ((progress.trie_id() + 1) < progress.num_tries()) {
      assert(!progress.is_last());
      ++progress;
    }
    assert(progress.is_last());
    assert(progress.num_tries() == MARISA_DEFAULT_NUM_TRIES);
    assert(progress.trie() == MARISA_DEFAULT_TRIE);
    assert(progress.tail() == MARISA_DEFAULT_TAIL);
    assert(progress.order() == MARISA_DEFAULT_ORDER);
  }

  {
    marisa::Progress progress(1 | MARISA_PREFIX_TRIE
        | MARISA_BINARY_TAIL | MARISA_LABEL_ORDER);
    assert(progress.is_valid());
    assert(progress.is_last());
    assert(progress.num_tries() == 1);
    assert(progress.trie() == MARISA_PREFIX_TRIE);
    assert(progress.tail() == MARISA_BINARY_TAIL);
    assert(progress.order() == MARISA_LABEL_ORDER);
  }

  return 0;
}
