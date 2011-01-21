#include <algorithm>
#include <stdexcept>

#include "trie.h"

namespace marisa {
namespace {

class PredictCallback {
 public:
  PredictCallback(std::vector<marisa::UInt32> *key_ids,
      std::vector<std::string> *keys, std::size_t avail)
      : key_ids_(key_ids), keys_(keys), avail_(avail) {}
  PredictCallback(const PredictCallback &callback)
      : key_ids_(callback.key_ids_), keys_(callback.keys_),
        avail_(callback.avail_) {}

  bool operator()(marisa::UInt32 key_id,
      const char *key, std::size_t key_length) {
    if (key_ids_ != NULL) {
      key_ids_->push_back(key_id);
    }
    if (keys_ != NULL) {
      keys_->push_back(std::string(key, key_length));
    }
    return avail_-- > 1;
  }

 private:
  std::vector<marisa::UInt32> *key_ids_;
  std::vector<std::string> *keys_;
  std::size_t avail_;

  // Disallows assignment.
  PredictCallback &operator=(const PredictCallback &);
};

}  // namespace

Trie::Trie()
    : louds_(), labels_(), terminal_flags_(), link_flags_(), links_(),
      trie_(), tail_(), num_keys_(0) {}

void Trie::mmap(Mapper *mapper, const char *filename,
    long offset, int whence) {
  MARISA_THROW_IF(mapper == NULL, MARISA_PARAM_ERROR);
  Mapper temp_mapper;
  temp_mapper.open(filename, offset, whence);
  map(temp_mapper);
  temp_mapper.swap(mapper);
}

void Trie::map(const void *ptr, std::size_t size) {
  Mapper mapper(ptr, size);
  map(mapper);
}

void Trie::map(Mapper &mapper) {
  Trie temp;
  temp.louds_.map(mapper);
  temp.labels_.map(mapper);
  temp.terminal_flags_.map(mapper);
  temp.link_flags_.map(mapper);
  temp.links_.map(mapper);
  temp.tail_.map(mapper);
  mapper.map(&temp.num_keys_);

  if (temp.has_link() && !temp.has_tail()) {
    temp.trie_.reset(new (std::nothrow) Trie);
    MARISA_THROW_IF(!temp.has_trie(), MARISA_MEMORY_ERROR);
    temp.trie_->map(mapper);
  }
  temp.swap(this);
}

void Trie::load(const char *filename,
    long offset, int whence) {
  Reader reader;
  reader.open(filename, offset, whence);
  read(reader);
}

void Trie::fread(std::FILE *file) {
  Reader reader(file);
  read(reader);
}

void Trie::read(int fd) {
  Reader reader(fd);
  read(reader);
}

void Trie::read(std::istream &stream) {
  Reader reader(&stream);
  read(reader);
}

void Trie::read(Reader &reader) {
  Trie temp;
  temp.louds_.read(reader);
  temp.labels_.read(reader);
  temp.terminal_flags_.read(reader);
  temp.link_flags_.read(reader);
  temp.links_.read(reader);
  temp.tail_.read(reader);
  reader.read(&temp.num_keys_);

  if (temp.has_link() && !temp.has_tail()) {
    temp.trie_.reset(new (std::nothrow) Trie);
    MARISA_THROW_IF(!temp.has_trie(), MARISA_MEMORY_ERROR);
    temp.trie_->read(reader);
  }
  temp.swap(this);
}

void Trie::save(const char *filename, bool trunc_flag,
    long offset, int whence) const {
  Writer writer;
  writer.open(filename, trunc_flag, offset, whence);
  write(writer);
}

void Trie::fwrite(std::FILE *file) const {
  Writer writer(file);
  write(writer);
}

void Trie::write(int fd) const {
  Writer writer(fd);
  write(writer);
}

void Trie::write(std::ostream &stream) const {
  Writer writer(&stream);
  write(writer);
}

void Trie::write(Writer &writer) const {
  louds_.write(writer);
  labels_.write(writer);
  terminal_flags_.write(writer);
  link_flags_.write(writer);
  links_.write(writer);
  tail_.write(writer);
  writer.write(num_keys_);
  if (has_trie()) {
    trie_->write(writer);
  }
}

std::string Trie::restore(UInt32 key_id) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(key_id >= num_keys_, MARISA_PARAM_ERROR);
  std::string key;
  restore_(key_id, &key);
  return key;
}

void Trie::restore(UInt32 key_id, std::string *key) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((key_id >= num_keys_) || (key == NULL), MARISA_PARAM_ERROR);
  restore_(key_id, key);
}

void Trie::restore(UInt32 key_id, char *key_buf,
    std::size_t key_buf_size, std::size_t *key_length) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((key_id >= num_keys_) ||
      ((key_buf == NULL) && (key_buf_size != 0)), MARISA_PARAM_ERROR);
  restore_(key_id, key_buf, key_buf_size, key_length);
}

UInt32 Trie::lookup(const char *str) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return lookup_<CQuery>(CQuery(str));
}

UInt32 Trie::lookup(const char *ptr, std::size_t length) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return lookup_<const Query &>(Query(ptr, length));
}

std::size_t Trie::find(const char *str,
    std::vector<UInt32> *key_ids, std::vector<std::size_t> *key_lengths,
    std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return find_<CQuery>(CQuery(str), key_ids, key_lengths, max_count);
}

std::size_t Trie::find(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids, std::vector<std::size_t> *key_lengths,
    std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return find_<const Query &>(Query(ptr, length),
      key_ids, key_lengths, max_count);
}

UInt32 Trie::find_first(const char *str,
    std::size_t *key_length) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return find_first_<CQuery>(CQuery(str), key_length);
}

UInt32 Trie::find_first(const char *ptr, std::size_t length,
    std::size_t *key_length) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return find_first_<const Query &>(Query(ptr, length), key_length);
}

UInt32 Trie::find_last(const char *str,
    std::size_t *key_length) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return find_last_<CQuery>(CQuery(str), key_length);
}

UInt32 Trie::find_last(const char *ptr, std::size_t length,
    std::size_t *key_length) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return find_last_<const Query &>(Query(ptr, length), key_length);
}

std::size_t Trie::predict(const char *str,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys,
    std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return (keys == NULL) ?
      predict_breadth_first(str, key_ids, max_count) :
      predict_depth_first(str, key_ids, keys, max_count);
}

std::size_t Trie::predict(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids, std::vector<std::string> *keys,
    std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return (keys == NULL) ?
      predict_breadth_first(ptr, length, key_ids, max_count) :
      predict_depth_first(ptr, length, key_ids, keys, max_count);
}

std::size_t Trie::predict_breadth_first(const char *str,
    std::vector<UInt32> *key_ids, std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return predict_breadth_first_<CQuery>(CQuery(str), key_ids, max_count);
}

std::size_t Trie::predict_breadth_first(const char *ptr, std::size_t length,
    std::vector<UInt32> *key_ids, std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return predict_breadth_first_<const Query &>(
      Query(ptr, length), key_ids, max_count);
}

std::size_t Trie::predict_depth_first(
    const char *str, std::vector<UInt32> *key_ids,
    std::vector<std::string> *keys, std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_PARAM_ERROR);
  return predict_depth_first_<CQuery>(
      CQuery(str), key_ids, keys, max_count);
}

std::size_t Trie::predict_depth_first(
    const char *ptr, std::size_t length, std::vector<UInt32> *key_ids,
    std::vector<std::string> *keys, std::size_t max_count) const {
  MARISA_THROW_IF(empty(), MARISA_STATE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_PARAM_ERROR);
  return predict_depth_first_<const Query &>(
      Query(ptr, length), key_ids, keys, max_count);
}

// predict...

int Trie::num_tries() const {
  return has_trie() ? (trie_->num_tries() + 1) : (louds_.empty() ? 0 : 1);
}

std::size_t Trie::num_nodes() const {
  if (louds_.empty()) {
    return 0;
  }
  std::size_t num_nodes = (louds_.size() / 2) - 1;
  if (has_trie()) {
    num_nodes += trie_->num_nodes();
  }
  return num_nodes;
}

std::size_t Trie::total_size() const {
  return louds_.total_size() + labels_.total_size()
      + terminal_flags_.total_size() + link_flags_.total_size()
      + links_.total_size() + (has_trie() ? trie_->total_size() : 0)
      + tail_.total_size() + sizeof(num_keys_);
}

void Trie::clear() {
  Trie().swap(this);
}

void Trie::swap(Trie *rhs) {
  MARISA_THROW_IF(rhs == NULL, MARISA_PARAM_ERROR);
  louds_.swap(&rhs->louds_);
  labels_.swap(&rhs->labels_);
  terminal_flags_.swap(&rhs->terminal_flags_);
  link_flags_.swap(&rhs->link_flags_);
  links_.swap(&rhs->links_);
  Swap(&trie_, &rhs->trie_);
  tail_.swap(&rhs->tail_);
  Swap(&num_keys_, &rhs->num_keys_);
}

void Trie::restore_(UInt32 key_id, std::string *key) const {
  const std::size_t start_pos = key->length();
  try {
    UInt32 node = key_id_to_node(key_id);
    while (node != 0) {
      if (has_link(node)) {
        const std::size_t prev_pos = key->length();
        if (has_trie()) {
          trie_->trie_restore(get_link(node), key);
        } else {
          tail_restore(node, key);
        }
        std::reverse(key->begin() + prev_pos, key->end());
      } else {
        *key += labels_[node];
      }
      node = get_parent(node);
    }
    std::reverse(key->begin() + start_pos, key->end());
  } catch (const std::bad_alloc &) {
    key->resize(start_pos);
    MARISA_THROW(MARISA_MEMORY_ERROR);
  } catch (const std::length_error &) {
    key->resize(start_pos);
    MARISA_THROW(MARISA_SIZE_ERROR);
  }
}

void Trie::trie_restore(UInt32 node, std::string *key) const {
  do {
    if (has_link(node)) {
      if (has_trie()) {
        trie_->trie_restore(get_link(node), key);
      } else {
        tail_restore(node, key);
      }
    } else {
      *key += labels_[node];
    }
    node = get_parent(node);
  } while (node != 0);
}

void Trie::tail_restore(UInt32 node, std::string *key) const {
  if (tail_.mode() == MARISA_BINARY_TAIL) {
    UInt32 length;
    const UInt32 offset = get_link(node, &length);
    key->append(reinterpret_cast<const char *>(tail_[offset]), length);
  } else {
    key->append(reinterpret_cast<const char *>(tail_[get_link(node)]));
  }
}

void Trie::restore_(UInt32 key_id, char *key_buf,
    std::size_t key_buf_size, std::size_t *key_length) const {
  std::size_t pos = 0;
  UInt32 node = key_id_to_node(key_id);
  while (node != 0) {
    if (has_link(node)) {
      const std::size_t prev_pos = pos;
      if (has_trie()) {
        trie_->trie_restore(get_link(node), key_buf, key_buf_size, pos);
      } else {
        tail_restore(node, key_buf, key_buf_size, pos);
      }
      if (pos < key_buf_size) {
        std::reverse(key_buf + prev_pos, key_buf + pos);
      }
    } else {
      if (pos < key_buf_size) {
        key_buf[pos] = labels_[node];
      }
      ++pos;
    }
    node = get_parent(node);
  }
  if (key_length != NULL) {
    *key_length = pos;
  }
  if (pos < key_buf_size) {
    key_buf[pos] = '\0';
    std::reverse(key_buf, key_buf + pos);
  }
}

void Trie::trie_restore(UInt32 node, char *key_buf,
    std::size_t key_buf_size, std::size_t &pos) const {
  do {
    if (has_link(node)) {
      if (has_trie()) {
        trie_->trie_restore(get_link(node), key_buf, key_buf_size, pos);
      } else {
        tail_restore(node, key_buf, key_buf_size, pos);
      }
    } else {
      if (pos < key_buf_size) {
        key_buf[pos] = labels_[node];
      }
      ++pos;
    }
    node = get_parent(node);
  } while (node != 0);
}

void Trie::tail_restore(UInt32 node, char *key_buf,
    std::size_t key_buf_size, std::size_t &pos) const {
  if (tail_.mode() == MARISA_BINARY_TAIL) {
    UInt32 length;
    const UInt8 *ptr = tail_[get_link(node, &length)];
    for (UInt32 i = 0; i < length; ++i) {
      if (pos < key_buf_size) {
        key_buf[pos] = ptr[i];
      }
      ++pos;
    }
  } else {
    for (const UInt8 *str = tail_[get_link(node)]; *str != '\0'; ++str) {
      if (pos < key_buf_size) {
        key_buf[pos] = *str;
      }
      ++pos;
    }
  }
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

//template <typename T>
//bool Trie::find_child(UInt32 &node, T query,
//    std::size_t &pos) const {
//  UInt32 louds_pos = get_child(node);
//  if (!louds_[louds_pos]) {
//    return false;
//  }
//  node = louds_pos_to_node(louds_pos);
//  do {
//    if (has_link(node)) {
//      UInt32 next_pos = has_trie() ?
//          trie_->trie_match<T>(get_link(node), query, pos) :
//          tail_match<T>(node, query, pos);
//      if (next_pos == mismatch()) {
//        return false;
//      } else if (next_pos != pos) {
//        pos = next_pos;
//        return true;
//      }
//    } else if (labels_[node] == query[pos]) {
//      ++pos;
//      return true;
//    }
//    ++node;
//    ++louds_pos;
//  } while (louds_[louds_pos]);
//  return false;
//}

template <typename T>
std::size_t Trie::trie_match(UInt32 node, T query,
    std::size_t pos) const {
  if (has_link(node)) {
    UInt32 next_pos;
    if (has_trie()) {
      next_pos = trie_->trie_match<T>(get_link(node), query, pos);
    } else {
      next_pos = tail_match<T>(node, query, pos);
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
      UInt32 next_pos;
      if (has_trie()) {
        next_pos = trie_->trie_match<T>(get_link(node), query, pos);
      } else {
        next_pos = tail_match<T>(node, query, pos);
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

template <typename T>
std::size_t Trie::tail_match(UInt32 node, T query,
    std::size_t pos) const {
  const UInt32 link_id = link_flags_.rank1(node);
  const UInt32 offset = (links_[link_id] * 256) + labels_[node];
  const UInt8 *ptr = tail_[offset];
  if (*ptr != query[pos]) {
    return pos;
  } else if (tail_.mode() == MARISA_BINARY_TAIL) {
    const UInt32 length = (links_[link_id + 1] * 256)
        + labels_[link_flags_.select1(link_id + 1)] - offset;
    for (UInt32 i = 1; i < length; ++i) {
      if (query.ends_at(pos + i) || (ptr[i] != query[pos + i])) {
        return mismatch();
      }
    }
    return pos + length;
  } else {
    for (++ptr, ++pos; *ptr != '\0'; ++ptr, ++pos) {
      if (query.ends_at(pos) || (*ptr != query[pos])) {
        return mismatch();
      }
    }
    return pos;
  }
}

template <typename T>
std::size_t Trie::find_(T query, std::vector<UInt32> *key_ids,
    std::vector<std::size_t> *key_lengths,
    std::size_t max_count) const try {
  std::size_t count = 0;
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
      if (++count >= max_count) {
        return count;
      }
    }
  } while (!query.ends_at(pos) && find_child<T>(node, query, pos));
  return count;
} catch (const std::bad_alloc &) {
  MARISA_THROW(MARISA_MEMORY_ERROR);
} catch (const std::length_error &) {
  MARISA_THROW(MARISA_SIZE_ERROR);
}

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

template <typename T>
UInt32 Trie::find_last_(T query, std::size_t *key_length) const {
  UInt32 node = 0;
  UInt32 node_found = notfound();
  std::size_t pos = 0;
  std::size_t pos_found = mismatch();
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

template <typename T>
std::size_t Trie::predict_breadth_first_(T query,
    std::vector<UInt32> *key_ids, std::size_t max_count) const try {
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
    if (++count >= max_count) {
      return count;
    }
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
        if (++count >= max_count) {
          return count;
        }
      }
    } else {
      count += key_id_end - key_id_begin;
      if (count >= max_count) {
        return max_count;
      }
    }
    node_begin = louds_pos_to_node(get_child(node_begin));
    node_end = louds_pos_to_node(get_child(node_end));
  }
  return count;
} catch (const std::bad_alloc &) {
  MARISA_THROW(MARISA_MEMORY_ERROR);
} catch (const std::length_error &) {
  MARISA_THROW(MARISA_SIZE_ERROR);
}

template <typename T>
std::size_t Trie::predict_depth_first_(T query, std::vector<UInt32> *key_ids,
    std::vector<std::string> *keys, std::size_t max_count) const {
  PredictCallback callback(key_ids, keys, max_count);
  return predict_callback_(query, callback);
}

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
          tail_prefix_match<T>(node, query, pos, key);
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
std::size_t Trie::trie_prefix_match(UInt32 node, T query,
    std::size_t pos, std::string *key) const {
  if (has_link(node)) {
    std::size_t next_pos;
    if (has_trie()) {
      next_pos = trie_->trie_prefix_match<T>(get_link(node), query, pos, key);
    } else {
      next_pos = tail_prefix_match<T>(node, query, pos, key);
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
        next_pos = tail_prefix_match<T>(node, query, pos, key);
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
std::size_t Trie::tail_prefix_match(UInt32 node, T query,
    std::size_t pos, std::string *key) const {
  const UInt32 link_id = link_flags_.rank1(node);
  const UInt32 offset = (links_[link_id] * 256) + labels_[node];
  const UInt8 *ptr = tail_[offset];
  if (*ptr != query[pos]) {
    return pos;
  } else if (tail_.mode() == MARISA_BINARY_TAIL) {
    const UInt32 length = (links_[link_id + 1] * 256)
        + labels_[link_flags_.select1(link_id + 1)] - offset;
    for (UInt32 i = 1; i < length; ++i) {
      if (query.ends_at(pos + i)) {
        if (key != NULL) {
          key->append(reinterpret_cast<const char *>(ptr + i), length - i);
        }
        return pos + i;
      } else if (ptr[i] != query[pos + i]) {
        return mismatch();
      }
    }
    return pos + length;
  } else {
    for (++ptr, ++pos; *ptr != '\0'; ++ptr, ++pos) {
      if (query.ends_at(pos)) {
        if (key != NULL) {
          key->append(reinterpret_cast<const char *>(ptr));
        }
        return pos;
      } else if (*ptr != query[pos]) {
        return mismatch();
      }
    }
    return pos;
  }
}

}  // namespace marisa
