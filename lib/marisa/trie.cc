#include <algorithm>

#include "./trie.h"

namespace marisa {
namespace {

class PredictCallback {
 public:
  PredictCallback(std::vector<marisa::UInt32> *key_ids,
      std::vector<std::string> *keys)
      : key_ids_(key_ids), keys_(keys) {}
  PredictCallback(const PredictCallback &callback)
      : key_ids_(callback.key_ids_), keys_(callback.keys_) {}

  bool operator()(marisa::UInt32 key_id, const std::string &key) const {
    if (key_ids_ != NULL) {
      key_ids_->push_back(key_id);
    }
    if (keys_ != NULL) {
      keys_->push_back(key);
    }
    return true;
  }

 private:
  std::vector<marisa::UInt32> *key_ids_;
  std::vector<std::string> *keys_;

  // Disallows assignment.
  PredictCallback &operator=(const PredictCallback &);
};

}  // namespace

Trie::Trie()
    : louds_(), labels_(), terminal_flags_(), link_flags_(), links_(),
      trie_(), tail_(), num_keys_(0) {}

bool Trie::mmap(Mapper *mapper, const char *filename,
    long offset, int whence) {
  Mapper temp_mapper;
  if (!temp_mapper.open(filename, offset, whence) || !map(&temp_mapper)) {
    return false;
  }
  temp_mapper.swap(mapper);
  return true;
}

bool Trie::map(const void *ptr) {
  Mapper mapper(ptr);
  return map(&mapper);
}

bool Trie::map(const void *ptr, std::size_t size) {
  Mapper mapper(ptr, size);
  return map(&mapper);
}

bool Trie::map(Mapper *mapper) {
  Trie temp;
  if (!temp.louds_.map(mapper) ||
      !temp.labels_.map(mapper) ||
      !temp.terminal_flags_.map(mapper) ||
      !temp.link_flags_.map(mapper) ||
      !temp.links_.map(mapper) ||
      !temp.tail_.map(mapper) ||
      !mapper->map(&temp.num_keys_)) {
    return false;
  }
  if (temp.has_link() && !temp.has_tail()) {
    temp.trie_.reset(new Trie);
    if (!temp.trie_->map(mapper)) {
      return false;
    }
  }
  temp.swap(this);
  return true;
}

bool Trie::load(const char *filename, long offset, int whence) {
  Reader reader;
  if (!reader.open(filename, offset, whence)) {
    return false;
  }
  return read(&reader);
}

bool Trie::read(int fd) {
  Reader reader(fd);
  return read(&reader);
}

bool Trie::read(::FILE *file) {
  Reader reader(file);
  return read(&reader);
}

bool Trie::read(std::istream *stream) {
  Reader reader(stream);
  return read(&reader);
}

bool Trie::read(Reader *reader) {
  Trie temp;
  if (!temp.louds_.read(reader) ||
      !temp.labels_.read(reader) ||
      !temp.terminal_flags_.read(reader) ||
      !temp.link_flags_.read(reader) ||
      !temp.links_.read(reader) ||
      !temp.tail_.read(reader) ||
      !reader->read(&temp.num_keys_)) {
    return false;
  }
  if (temp.has_link() && !temp.has_tail()) {
    temp.trie_.reset(new Trie);
    if (!temp.trie_->read(reader)) {
      return false;
    }
  }
  temp.swap(this);
  return true;
}

bool Trie::save(const char *filename, bool trunc_flag,
    long offset, int whence) const {
  Writer writer;
  if (!writer.open(filename, trunc_flag, offset, whence)) {
    return false;
  }
  return write(&writer);
}

bool Trie::write(int fd) const {
  Writer writer(fd);
  return write(&writer);
}

bool Trie::write(::FILE *file) const {
  Writer writer(file);
  return write(&writer);
}

bool Trie::write(std::ostream *stream) const {
  Writer writer(stream);
  return write(&writer);
}

bool Trie::write(Writer *writer) const {
  if (!louds_.write(writer) ||
      !labels_.write(writer) ||
      !terminal_flags_.write(writer) ||
      !link_flags_.write(writer) ||
      !links_.write(writer) ||
      !tail_.write(writer) ||
      !writer->write(num_keys_)) {
    return false;
  }
  return has_trie() ? trie_->write(writer) : true;
}

UInt32 Trie::num_tries() const {
  return has_trie() ? (trie_->num_tries() + 1) : (louds_.empty() ? 0 : 1);
}

UInt32 Trie::num_nodes() const {
  return labels_.num_objs() + (has_trie() ? trie_->num_nodes() : 0);
}

std::size_t Trie::size() const {
  return louds_.size() + labels_.size() + terminal_flags_.size()
      + link_flags_.size() + links_.size()
      + (has_trie() ? trie_->size() : 0) + tail_.size() + sizeof(num_keys_);
}

void Trie::clear() {
  Trie().swap(this);
}

void Trie::swap(Trie *rhs) {
  louds_.swap(&rhs->louds_);
  labels_.swap(&rhs->labels_);
  terminal_flags_.swap(&rhs->terminal_flags_);
  link_flags_.swap(&rhs->link_flags_);
  links_.swap(&rhs->links_);
  std::swap(trie_, rhs->trie_);
  tail_.swap(&rhs->tail_);
  std::swap(num_keys_, rhs->num_keys_);
}

void Trie::restore_(UInt32 key_id, std::string *key) const {
  std::size_t src_pos = key->length();
  UInt32 node = key_id_to_node(key_id);
  while (node != 0) {
    if (has_link(node)) {
      std::size_t pos = key->length();
      if (has_trie()) {
        trie_->trie_restore(get_link(node), key);
      } else {
        tail_restore(get_link(node), key);
      }
      std::reverse(key->begin() + pos, key->end());
    } else {
      *key += labels_[node];
    }
    node = get_parent(node);
  }
  std::reverse(key->begin() + src_pos, key->end());
}

void Trie::trie_restore(UInt32 node, std::string *key) const {
  do {
    if (has_link(node)) {
      if (has_trie()) {
        trie_->trie_restore(get_link(node), key);
      } else {
        tail_restore(get_link(node), key);
      }
    } else {
      *key += labels_[node];
    }
    node = get_parent(node);
  } while (node != 0);
}

void Trie::tail_restore(UInt32 offset, std::string *key) const {
  key->append(reinterpret_cast<const char *>(tail_[offset]));
}

template <typename T>
UInt32 Trie::lookup_(T query) const {
  UInt32 node = 0;
  std::size_t pos = 0;
  while (!query.ends_at(pos)) {
    if (!find_child<T>(node, query, pos)) {
      return notfound();
    }
  }
  return terminal_flags_[node] ? node_to_key_id(node) : notfound();
}

template UInt32 Trie::lookup_(CQuery query) const;
template UInt32 Trie::lookup_(const Query &query) const;

template <typename T>
UInt32 Trie::find_(T query, std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths) const {
  UInt32 count = 0;
  UInt32 node = 0;
  std::size_t pos = 0;
  do {
    if (terminal_flags_[node]) {
      if (key_ids != NULL) {
        key_ids->push_back(node_to_key_id(node));
      }
      if (key_lengths != NULL) {
        key_lengths->push_back(pos);
      }
      ++count;
    }
  } while (!query.ends_at(pos) && find_child<T>(node, query, pos));
  return count;
}

template UInt32 Trie::find_(CQuery query,
    std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths) const;
template UInt32 Trie::find_(const Query &query,
    std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths) const;

template <typename T>
UInt32 Trie::find_first_(T query, std::size_t *key_length) const {
  UInt32 node = 0;
  std::size_t pos = 0;
  do {
    if (terminal_flags_[node]) {
      if (key_length != NULL) {
        *key_length = pos;
      }
      return node_to_key_id(node);
    }
  } while (!query.ends_at(pos) && find_child<T>(node, query, pos));
  return notfound();
}

template UInt32 Trie::find_first_(CQuery query,
    std::size_t *key_length) const;
template UInt32 Trie::find_first_(const Query &query,
    std::size_t *key_length) const;

template <typename T>
UInt32 Trie::find_last_(T query, std::size_t *key_length) const {
  UInt32 node_found = notfound();
  std::size_t pos_found = mismatch();
  UInt32 node = 0;
  std::size_t pos = 0;
  do {
    if (terminal_flags_[node]) {
      node_found = node;
      pos_found = pos;
    }
  } while (!query.ends_at(pos) && find_child<T>(node, query, pos));
  if (node_found != notfound()) {
    if (key_length != NULL) {
      *key_length = pos_found;
    }
    return node_to_key_id(node_found);
  }
  return notfound();
}

template UInt32 Trie::find_last_(CQuery query,
    std::size_t *key_length) const;
template UInt32 Trie::find_last_(const Query &query,
    std::size_t *key_length) const;

template <typename T>
std::size_t Trie::trie_match(UInt32 node, T query, std::size_t pos) const {
  if (has_link(node)) {
    std::size_t next_pos;
    if (has_trie()) {
      next_pos = trie_->trie_match<T>(get_link(node), query, pos);
    } else {
      next_pos = tail_match<T>(get_link(node), query, pos);
    }
    if ((next_pos == mismatch()) || (next_pos == pos)) {
      return next_pos;
    }
    pos = next_pos;
  } else if (labels_[node] != query[pos]) {
    return pos;
  } else {
    ++pos;
  }
  node = get_parent(node);
  while (node != 0) {
    if (query.ends_at(pos)) {
      return mismatch();
    }
    if (has_link(node)) {
      std::size_t next_pos;
      if (has_trie()) {
        next_pos = trie_->trie_match<T>(get_link(node), query, pos);
      } else {
        next_pos = tail_match<T>(get_link(node), query, pos);
      }
      if ((next_pos == mismatch()) || (next_pos == pos)) {
        return mismatch();
      }
      pos = next_pos;
    } else if (labels_[node] != query[pos]) {
      return mismatch();
    } else {
      ++pos;
    }
    node = get_parent(node);
  }
  return pos;
}

template std::size_t Trie::trie_match(UInt32 node,
    CQuery query, std::size_t pos) const;
template std::size_t Trie::trie_match(UInt32 node,
    const Query &query, std::size_t pos) const;

template <typename T>
UInt32 Trie::predict_(T query, std::vector<UInt32> *key_ids) const {
  UInt32 node = 0;
  std::size_t pos = 0;
  while (!query.ends_at(pos)) {
    if (!predict_child<T>(node, query, pos)) {
      return 0;
    }
  }
  UInt32 count = 0;
  if (terminal_flags_[node]) {
    if (key_ids != NULL) {
      key_ids->push_back(node_to_key_id(node));
    }
    ++count;
  }
  UInt32 louds_pos = get_child(node);
  if (!louds_[louds_pos]) {
    return count;
  }
  UInt32 node_begin = louds_pos_to_node(louds_pos);
  UInt32 node_end = louds_pos_to_node(get_child(node + 1));
  while (node_begin < node_end) {
    UInt32 key_id_begin = node_to_key_id(node_begin);
    UInt32 key_id_end = node_to_key_id(node_end);
    if (key_ids != NULL) {
      for (UInt32 key_id = key_id_begin; key_id < key_id_end; ++key_id) {
        key_ids->push_back(key_id);
      }
    }
    count += key_id_end - key_id_begin;
    node_begin = louds_pos_to_node(get_child(node_begin));
    node_end = louds_pos_to_node(get_child(node_end));
  }
  return count;
}

template UInt32 Trie::predict_(CQuery query,
    std::vector<UInt32> *key_ids) const;
template UInt32 Trie::predict_(const Query &query,
    std::vector<UInt32> *key_ids) const;

template <typename T>
UInt32 Trie::predict_(T query, std::vector<UInt32> *key_ids,
    std::vector<std::string> *keys) const {
  PredictCallback callback(key_ids, keys);
  return predict_callback_(query, callback);
}

template UInt32 Trie::predict_(CQuery query,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const;
template UInt32 Trie::predict_(const Query &query,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const;

template <typename T>
bool Trie::predict_child(UInt32 &node, T query, std::size_t &pos,
    std::string *key) const {
  UInt32 louds_pos = get_child(node);
  if (!louds_[louds_pos]) {
    return false;
  }
  node = louds_pos_to_node(louds_pos);
  do {
    if (has_link(node)) {
      std::size_t next_pos = has_trie() ?
          trie_->trie_prefix_match<T>(get_link(node), query, pos, key) :
          tail_prefix_match<T>(get_link(node), query, pos, key);
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

template bool Trie::predict_child(UInt32 &node,
    CQuery query, std::size_t &pos, std::string *key) const;
template bool Trie::predict_child(UInt32 &node,
    const Query &query, std::size_t &pos, std::string *key) const;

template <typename T>
std::size_t Trie::trie_prefix_match(UInt32 node, T query,
    std::size_t pos, std::string *key) const {
  if (has_link(node)) {
    std::size_t next_pos;
    if (has_trie()) {
      next_pos = trie_->trie_prefix_match<T>(get_link(node), query, pos, key);
    } else {
      next_pos = tail_prefix_match<T>(get_link(node), query, pos, key);
    }
    if ((next_pos == mismatch()) || (next_pos == pos)) {
      return next_pos;
    }
    pos = next_pos;
  } else if (labels_[node] != query[pos]) {
    return pos;
  } else {
    ++pos;
  }
  node = get_parent(node);
  while (node != 0) {
    if (query.ends_at(pos)) {
      if (key != NULL) {
        trie_restore(node, key);
      }
      return pos;
    }
    if (has_link(node)) {
      std::size_t next_pos;
      if (has_trie()) {
        next_pos = trie_->trie_prefix_match<T>(
            get_link(node), query, pos, key);
      } else {
        next_pos = tail_prefix_match<T>(get_link(node), query, pos, key);
      }
      if ((next_pos == mismatch()) || (next_pos == pos)) {
        return next_pos;
      }
      pos = next_pos;
    } else if (labels_[node] != query[pos]) {
      return mismatch();
    } else {
      ++pos;
    }
    node = get_parent(node);
  }
  return pos;
}

template <typename T>
std::size_t Trie::tail_prefix_match(UInt32 offset, T query,
    std::size_t pos, std::string *key) const {
  const UInt8 *p = tail_[offset];
  if (*p != query[pos]) {
    return pos;
  }
  for (++p, ++pos; *p != '\0'; ++p, ++pos) {
    if (query.ends_at(pos)) {
      if (key != NULL) {
        key->append(reinterpret_cast<const char *>(p));
      }
      return pos;
    } else if (*p != query[pos]) {
      return mismatch();
    }
  }
  return pos;
}

}  // namespace marisa
