#ifndef MARISA_CONFIG_H_
#define MARISA_CONFIG_H_

#include "./base.h"

namespace marisa {

class Config {
 public:
  Config(UInt32 max_num_tries, bool patricia, bool tail, bool weight_order)
      : max_num_tries_(max_num_tries),
        patricia_(patricia),
        tail_(tail),
        weight_order_(weight_order) {}

  bool is_last_trie(UInt32 trie_id) const {
    return (trie_id + 1) >= max_num_tries_;
  }

  UInt32 max_num_tries() const {
    return max_num_tries_;
  }
  bool patricia() const {
    return patricia_;
  }
  bool tail() const {
    return tail_;
  }
  bool weight_order() const {
    return weight_order_;
  }

 private:
  UInt32 max_num_tries_;
  bool patricia_;
  bool tail_;
  bool weight_order_;

  // Disallows copy and assignment.
};

}  // namespace marisa

#endif  // MARISA_CONFIG_H_
