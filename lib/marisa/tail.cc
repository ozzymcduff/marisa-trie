#include <algorithm>
#include <functional>
#include <utility>

#include "./tail.h"

namespace marisa {

Tail::Tail() : buf_() {}

bool Tail::build(const std::vector<const char *> &keys,
    std::vector<UInt32> *offsets) {
  return build_tail(keys, offsets);
}

bool Tail::build(const std::vector<char *> &keys,
    std::vector<UInt32> *offsets) {
  return build_tail(keys, offsets);
}

bool Tail::build(const std::vector<std::string> &keys,
    std::vector<UInt32> *offsets) {
  return build_tail(keys, offsets);
}

bool Tail::build(const std::vector<String> &keys,
    std::vector<UInt32> *offsets) {
  return build_tail(keys, offsets);
}

bool Tail::mmap(Mapper *mapper, const char *filename,
    long offset, int whence) {
  Mapper temp_mapper;
  if (!temp_mapper.open(filename, offset, whence) || !map(&temp_mapper)) {
    return false;
  }
  temp_mapper.swap(mapper);
  return true;
}

bool Tail::map(const void *ptr) {
  Mapper mapper(ptr);
  return map(&mapper);
}

bool Tail::map(const void *ptr, std::size_t size) {
  Mapper mapper(ptr, size);
  return map(&mapper);
}

bool Tail::map(Mapper *mapper) {
  Tail temp;
  if (!temp.buf_.map(mapper)) {
    return false;
  }
  temp.swap(this);
  return true;
}

bool Tail::load(const char *filename, long offset, int whence) {
  Reader reader;
  if (!reader.open(filename, offset, whence)) {
    return false;
  }
  return read(&reader);
}

bool Tail::read(int fd) {
  Reader reader(fd);
  return read(&reader);
}

bool Tail::read(::FILE *file) {
  Reader reader(file);
  return read(&reader);
}

bool Tail::read(std::istream *stream) {
  Reader reader(stream);
  return read(&reader);
}

bool Tail::read(Reader *reader) {
  Tail temp;
  if (!temp.buf_.read(reader)) {
    return false;
  }
  temp.swap(this);
  return true;
}

bool Tail::save(const char *filename, bool trunc_flag,
    long offset, int whence) const {
  Writer writer;
  if (!writer.open(filename, trunc_flag, offset, whence)) {
    return false;
  }
  return write(&writer);
}

bool Tail::write(int fd) const {
  Writer writer(fd);
  return write(&writer);
}

bool Tail::write(::FILE *file) const {
  Writer writer(file);
  return write(&writer);
}

bool Tail::write(std::ostream *stream) const {
  Writer writer(stream);
  return write(&writer);
}

bool Tail::write(Writer *writer) const {
  return buf_.write(writer);
}

void Tail::clear() {
  Tail().swap(this);
}

void Tail::swap(Tail *rhs) {
  buf_.swap(&rhs->buf_);
}

template <typename T>
bool Tail::build_tail(const std::vector<T> &keys,
    std::vector<UInt32> *offsets) {
  typedef std::pair<RString, UInt32> KeyAndID;

  if (keys.empty()) {
    buf_.clear();
    if (offsets != NULL) {
      std::vector<UInt32>().swap(*offsets);
    }
    return true;
  }

  std::vector<KeyAndID> pairs(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    String str(keys[i]);
    for (std::size_t j = 0; j < str.length(); ++j) {
      if (str[j] == '\0') {
        return false;
      }
    }
    pairs[i].first = RString(str);
    pairs[i].second = i;
  }
  std::sort(pairs.begin(), pairs.end(), std::greater<KeyAndID>());

  Vector<UInt8> buf;
  std::vector<UInt32> temp_offsets(pairs.size(), 0);
  KeyAndID dummy_key;
  const KeyAndID *last = &dummy_key;
  for (std::size_t i = 0; i < pairs.size(); ++i) {
    const KeyAndID &cur = pairs[i];
    std::size_t match = 0;
    while ((match < cur.first.length()) && (match < last->first.length()) &&
        last->first[match] == cur.first[match]) {
      ++match;
    }
    if ((match == cur.first.length()) && (last->first.length() != 0)) {
      temp_offsets[cur.second] = temp_offsets[last->second]
          + (last->first.length() - match);
    } else {
      temp_offsets[cur.second] = buf.num_objs();
      for (std::size_t j = 1; j <= cur.first.length(); ++j) {
        buf.push_back(cur.first[cur.first.length() - j]);
      }
      buf.push_back('\0');
    }
    last = &cur;
  }
  buf.shrink();

  if (offsets != NULL) {
    offsets->swap(temp_offsets);
  }
  buf_.swap(&buf);
  return true;
}

}  // namespace marisa
