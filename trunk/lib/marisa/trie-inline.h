#ifndef MARISA_TRIE_INLINE_H_
#define MARISA_TRIE_INLINE_H_

#include "./cell.h"

namespace marisa {

inline std::string Trie::operator[](UInt32 key_id) const {
  std::string key;
  restore(key_id, &key);
  return key;
}

inline UInt32 Trie::operator[](const std::string &str) const {
  return lookup(str);
}

inline UInt32 Trie::operator[](const char *str) const {
  return lookup(str);
}

inline std::string Trie::restore(UInt32 key_id) const {
  std::string key;
  restore(key_id, &key);
  return key;
}

inline void Trie::restore(UInt32 key_id, std::string *key) const {
  restore_(key_id, key);
}

inline UInt32 Trie::lookup(const char *str) const {
  return lookup_<CQuery>(CQuery(str));
}

inline UInt32 Trie::lookup(const char *ptr, std::size_t length) const {
  return lookup_<const Query &>(Query(ptr, length));
}

inline UInt32 Trie::lookup(const std::string &str) const {
  return lookup_<const Query &>(Query(str));
}

inline UInt32 Trie::find(const char *str,
    std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths) const {
  return find_<CQuery>(CQuery(str), key_ids, key_lengths);
}

inline UInt32 Trie::find(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths) const {
  return find_<const Query &>(Query(ptr, length), key_ids, key_lengths);
}

inline UInt32 Trie::find(const std::string &str,
    std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths) const {
  return find_<const Query &>(Query(str), key_ids, key_lengths);
}

inline UInt32 Trie::find_first(const char *str,
    std::size_t *key_length) const {
  return find_first_<CQuery>(CQuery(str), key_length);
}

inline UInt32 Trie::find_first(const char *ptr, std::size_t length,
    std::size_t *key_length) const {
  return find_first_<const Query &>(Query(ptr, length), key_length);
}

inline UInt32 Trie::find_first(const std::string &str,
    std::size_t *key_length) const {
  return find_first_<const Query &>(Query(str), key_length);
}

inline UInt32 Trie::find_last(const char *str,
    std::size_t *key_length) const {
  return find_last_<CQuery>(CQuery(str), key_length);
}

inline UInt32 Trie::find_last(const char *ptr, std::size_t length,
    std::size_t *key_length) const {
  return find_last_<const Query &>(Query(ptr, length), key_length);
}

inline UInt32 Trie::find_last(const std::string &str,
    std::size_t *key_length) const {
  return find_last_<const Query &>(Query(str), key_length);
}

template <typename T>
inline UInt32 Trie::find_callback(const char *str, T callback) const {
  return find_callback_<CQuery>(CQuery(str), callback);
}

template <typename T>
inline UInt32 Trie::find_callback(const char *ptr, std::size_t length,
    T callback) const {
  return find_callback_<const Query &>(Query(ptr, length), callback);
}

template <typename T>
inline UInt32 Trie::find_callback(const std::string &str,
    T callback) const {
  return find_callback_<const Query &>(Query(str), callback);
}

inline UInt32 Trie::predict(const char *str,
    std::vector<UInt32> *key_ids) const {
  return predict_<CQuery>(CQuery(str), key_ids);
}

inline UInt32 Trie::predict(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids) const {
  return predict_<const Query &>(Query(ptr, length), key_ids);
}

inline UInt32 Trie::predict(const std::string &str,
    std::vector<UInt32> *key_ids) const {
  return predict_<const Query &>(Query(str), key_ids);
}

inline UInt32 Trie::predict(const char *str,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const {
  return predict_<CQuery>(CQuery(str), key_ids, keys);
}

inline UInt32 Trie::predict(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const {
  return predict_<const Query &>(Query(ptr, length), key_ids, keys);
}

inline UInt32 Trie::predict(const std::string &str,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const {
  return predict_<const Query &>(Query(str), key_ids, keys);
}

template <typename T>
inline UInt32 Trie::predict_callback(const char *str, T callback) const {
  return predict_callback_<CQuery>(CQuery(str), callback);
}

template <typename T>
inline UInt32 Trie::predict_callback(const char *ptr, std::size_t length,
    T callback) const {
  return predict_callback_<const Query &>(Query(ptr, length), callback);
}

template <typename T>
inline UInt32 Trie::predict_callback(const std::string &str,
    T callback) const {
  return predict_callback_<const Query &>(Query(str), callback);
}

inline UInt32 Trie::num_keys() const {
  return num_keys_;
}

inline UInt32 Trie::notfound() {
  return NOTFOUND;
}

inline std::size_t Trie::mismatch() {
  return MISMATCH;
}

template <typename T, typename U>
UInt32 Trie::find_callback_(T query, U callback) const {
  UInt32 count = 0;
  UInt32 node = 0;
  std::size_t pos = 0;
  do {
    if (terminal_flags_[node]) {
      ++count;
      if (!callback(node_to_key_id(node), static_cast<std::size_t>(pos))) {
        return count;
      }
    }
  } while (!query.ends_at(pos) && find_child<T>(node, query, pos));
  return count;
}

template <typename T>
inline bool Trie::find_child(UInt32 &node, T query,
    std::size_t &pos) const {
  UInt32 louds_pos = get_child(node);
  if (!louds_[louds_pos]) {
    return false;
  }
  node = louds_pos_to_node(louds_pos);
  do {
    if (has_link(node)) {
      std::size_t next_pos = has_trie() ?
          trie_->trie_match<T>(get_link(node), query, pos) :
          tail_match<T>(get_link(node), query, pos);
      if (next_pos == mismatch()) {
        return false;
      } else if (next_pos != pos) {
        pos = next_pos;
        return true;
      }
    } else if (labels_[node] == query[pos]) {
      ++pos;
      return true;
    }
    ++node;
    ++louds_pos;
  } while (louds_[louds_pos]);
  return false;
}

template <typename T>
std::size_t Trie::tail_match(UInt32 offset, T query,
    std::size_t pos) const {
  const UInt8 *p = tail_[offset];
  if (*p != query[pos]) {
    return pos;
  }
  for (++p, ++pos; *p != '\0'; ++p, ++pos) {
    if (query.ends_at(pos) || (*p != query[pos])) {
      return mismatch();
    }
  }
  return pos;
}

template <typename T, typename U>
UInt32 Trie::predict_callback_(T query, U callback) const {
  std::string key;
  UInt32 node = 0;
  std::size_t pos = 0;
  while (!query.ends_at(pos)) {
    if (!predict_child<T>(node, query, pos, &key)) {
      return 0;
    }
  }
  query.insert(&key);
  UInt32 count = 0;
  if (terminal_flags_[node]) {
    ++count;
    if (!callback(node_to_key_id(node),
        static_cast<const std::string &>(key))) {
      return count;
    }
  }
  Cell cell;
  cell.set_louds_pos(get_child(node));
  if (!louds_[cell.louds_pos()]) {
    return count;
  }
  cell.set_node(louds_pos_to_node(cell.louds_pos()));
  cell.set_key_id(node_to_key_id(cell.node()));
  cell.set_length(key.length());
  std::vector<Cell> stack;
  stack.push_back(cell);
  std::size_t stack_pos = 1;
  while (stack_pos != 0) {
    Cell &cur = stack[stack_pos - 1];
    if (!louds_[cur.louds_pos()]) {
      cur.set_louds_pos(cur.louds_pos() + 1);
      --stack_pos;
      continue;
    }
    cur.set_louds_pos(cur.louds_pos() + 1);
    key.resize(cur.length());
    if (has_link(cur.node())) {
      if (has_trie()) {
        trie_->trie_restore(get_link(cur.node()), &key);
      } else {
        tail_restore(get_link(cur.node()), &key);
      }
    } else {
      key += labels_[cur.node()];
    }
    if (terminal_flags_[cur.node()]) {
      ++count;
      if (!callback(cur.key_id(),
          static_cast<const std::string &>(key))) {
        return count;
      }
      cur.set_key_id(cur.key_id() + 1);
    }
    if (stack_pos == stack.size()) {
      cell.set_louds_pos(get_child(cur.node()));
      cell.set_node(louds_pos_to_node(cell.louds_pos()));
      cell.set_key_id(node_to_key_id(cell.node()));
      stack.push_back(cell);
    }
    stack[stack_pos].set_length(key.length());
    stack[stack_pos - 1].set_node(stack[stack_pos - 1].node() + 1);
    ++stack_pos;
  }
  return count;
}

inline UInt32 Trie::key_id_to_node(UInt32 key_id) const {
  return terminal_flags_.select1(key_id);
}

inline UInt32 Trie::node_to_key_id(UInt32 node) const {
  return terminal_flags_.rank1(node);
}

inline UInt32 Trie::louds_pos_to_node(UInt32 louds_pos) const {
  return louds_.rank1(louds_pos);
}

inline UInt32 Trie::get_child(UInt32 node) const {
  return louds_.select0(node) + 1;
}

inline UInt32 Trie::get_parent(UInt32 node) const {
  return louds_.rank0(louds_.select1(node)) - 1;
}

inline bool Trie::has_link(UInt32 node) const {
  return (link_flags_.empty()) ? false : link_flags_[node];
}

inline UInt32 Trie::get_link(UInt32 node) const {
  return (links_[link_flags_.rank1(node)] * 256) + labels_[node];
}

inline bool Trie::has_link() const {
  return !link_flags_.empty();
}

inline bool Trie::has_trie() const {
  return trie_.get() != NULL;
}

inline bool Trie::has_tail() const {
  return !tail_.empty();
}

}  // namespace marisa

#endif  // MARISA_TRIE_INLINE_H_
