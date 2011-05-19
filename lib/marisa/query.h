#ifndef MARISA_QUERY_H_
#define MARISA_QUERY_H_

#include "base.h"

namespace marisa {

class Query {
 public:
  Query() : ptr_(NULL), length_(0), key_id_(0) {}
  Query(const Query &query)
      : ptr_(query.ptr_), length_(query.length_), key_id_(query.key_id_) {}

  Query &operator=(const Query &query) {
    ptr_ = query.ptr_;
    length_ = query.length_;
    key_id_ = query.key_id_;
    return *this;
  }

  char operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= length_, MARISA_BOUND_ERROR);
    return ptr_[i];
  }

  void set_str(const char *str) {
    MARISA_DEBUG_IF(str == NULL, MARISA_NULL_ERROR);
    std::size_t length = 0;
    while (str[length] != '\0') {
      ++length;
    }
    ptr_ = str;
    length_ = length;
  }
  void set_str(const char *ptr, std::size_t length) {
    MARISA_DEBUG_IF((ptr == NULL) && (length != 0), MARISA_NULL_ERROR);
    ptr_ = ptr;
    length_ = length;
  }
  void set_key_id(std::size_t key_id) {
    key_id_ = key_id;
  }

  const char *ptr() const {
    return ptr_;
  }
  std::size_t length() const {
    return length_;
  }
  std::size_t key_id() const {
    return key_id_;
  }

  void clear() {
    Query().swap(*this);
  }
  void swap(Query &rhs) {
    marisa::swap(ptr_, rhs.ptr_);
    marisa::swap(length_, rhs.length_);
    marisa::swap(key_id_, rhs.key_id_);
  }

 private:
  const char *ptr_;
  std::size_t length_;
  std::size_t key_id_;
};

}  // namespace marisa

#endif  // MARISA_QUERY_H_
