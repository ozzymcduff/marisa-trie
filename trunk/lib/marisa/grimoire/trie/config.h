#ifndef MARISA_GRIMOIRE_TRIE_CONFIG_H_
#define MARISA_GRIMOIRE_TRIE_CONFIG_H_

#include "../../base.h"

namespace marisa {
namespace grimoire {
namespace trie {

class Config {
 public:
  Config()
      : num_tries_(MARISA_DEFAULT_NUM_TRIES), tail_mode_(MARISA_DEFAULT_TAIL),
        node_order_(MARISA_DEFAULT_ORDER) {}

  void parse(int config_flags) {
    Config temp;
    temp.parse_(config_flags);
    swap(temp);
  }

  int flags() const {
    return (int)num_tries_ | tail_mode_ | node_order_;
  }
  std::size_t num_tries() const {
    return num_tries_;
  }
  TailMode tail_mode() const {
    return tail_mode_;
  }
  NodeOrder node_order() const {
    return node_order_;
  }

  void clear() {
    Config().swap(*this);
  }
  void swap(Config &rhs) {
    marisa::swap(num_tries_, rhs.num_tries_);
    marisa::swap(tail_mode_, rhs.tail_mode_);
    marisa::swap(node_order_, rhs.node_order_);
  }

 private:
  std::size_t num_tries_;
  TailMode tail_mode_;
  NodeOrder node_order_;

  void parse_(int config_flags) {
    MARISA_THROW_IF((config_flags & ~MARISA_CONFIG_MASK) != 0,
        MARISA_CODE_ERROR);

    const int num_tries = config_flags & MARISA_NUM_TRIES_MASK;
    if (num_tries != 0) {
      num_tries_ = num_tries;
    }

    const int tail_mode = config_flags & MARISA_TAIL_MODE_MASK;
    switch (tail_mode) {
      case 0: {
        tail_mode_ = MARISA_DEFAULT_TAIL;
        break;
      }
      case MARISA_TEXT_TAIL: {
        tail_mode_ = MARISA_TEXT_TAIL;
        break;
      }
      case MARISA_BINARY_TAIL: {
        tail_mode_ = MARISA_BINARY_TAIL;
        break;
      }
      default: {
        MARISA_THROW(MARISA_CODE_ERROR, "undefined tail mode");
      }
    }

    const int node_order = config_flags & MARISA_NODE_ORDER_MASK;
    switch (node_order) {
      case 0: {
        node_order_ = MARISA_DEFAULT_ORDER;
        break;
      }
      case MARISA_LABEL_ORDER: {
        node_order_ = MARISA_LABEL_ORDER;
        break;
      }
      case MARISA_WEIGHT_ORDER: {
        node_order_ = MARISA_WEIGHT_ORDER;
        break;
      }
      default: {
        MARISA_THROW(MARISA_CODE_ERROR, "undefined node order");
      }
    }
  }

  // Disallows copy and assignment.
  Config(const Config &);
  Config &operator=(const Config &);
};

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_CONFIG_H_
