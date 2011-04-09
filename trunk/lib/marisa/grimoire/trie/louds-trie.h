#ifndef MARISA_GRIMOIRE_TRIE_LOUDS_TRIE_H_
#define MARISA_GRIMOIRE_TRIE_LOUDS_TRIE_H_

#include "../../keyset.h"
#include "../../agent.h"
#include "../vector.h"
#include "config.h"
#include "key.h"
#include "tail.h"

namespace marisa {
namespace grimoire {
namespace trie {

class LoudsTrie  {
 public:
  LoudsTrie();
  ~LoudsTrie();

  void build(Keyset &keyset, int flags);

  void map(Mapper &mapper);
  void read(Reader &reader);
  void write(Writer &writer) const;

  bool lookup(Agent &agent) const;
  void reverse_lookup(Agent &agent) const;
  bool common_prefix_search(Agent &agent) const;
  bool predictive_search(Agent &agent) const;

  std::size_t num_tries() const {
    return config_.num_tries();
  }
  std::size_t num_keys() const {
    return size();
  }
  std::size_t num_nodes() const {
    return (louds_.size() / 2) - 1;
  }

  TailMode tail_mode() const {
    return config_.tail_mode();
  }
  NodeOrder node_order() const {
    return config_.node_order();
  }

  bool empty() const {
    return size() == 0;
  }
  std::size_t size() const {
    return terminal_flags_.num_1s();
  }
  std::size_t total_size() const;
  std::size_t io_size() const;

  void clear();
  void swap(LoudsTrie &rhs);

 private:
  BitVector louds_;
  BitVector terminal_flags_;
  Vector<UInt8> labels_;
  BitVector link_flags_;
  FlatVector links_;
  Tail tail_;
  scoped_ptr<LoudsTrie> next_trie_;
  std::size_t num_l1_nodes_;
  Config config_;
  Mapper mapper_;

  void build_(Keyset &keyset, const Config &config);

  template <typename T>
  void build_trie(Vector<T> &keys,
      Vector<UInt32> *terminals, const Config &config, std::size_t trie_id);

  template <typename T>
  void build_current_trie(Vector<T> &keys,
      Vector<UInt32> *terminals, const Config &config, std::size_t trie_id);

  void build_next_trie(Vector<Key> &keys,
      Vector<UInt32> *terminals, const Config &config, std::size_t trie_id);
  void build_next_trie(Vector<ReverseKey> &keys,
      Vector<UInt32> *terminals, const Config &config, std::size_t trie_id);

  template <typename T>
  void build_terminals(const Vector<T> &keys,
      Vector<UInt32> *terminals) const;

  void map_(Mapper &mapper);
  void read_(Reader &reader);
  void write_(Writer &writer) const;

  inline bool find_child(Agent &agent) const;
  inline bool predictive_find_child(Agent &agent) const;

  void restore(Agent &agent, std::size_t node_id) const;
  bool match(Agent &agent, std::size_t node_id) const;
  bool prefix_match(Agent &agent, std::size_t node_id) const;

  // Disallows copy and assignment.
  LoudsTrie(const LoudsTrie &);
  LoudsTrie &operator=(const LoudsTrie &);
};

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_LOUDS_TRIE_H_
