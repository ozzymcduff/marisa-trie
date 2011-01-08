#ifndef MARISA_STRING_H_
#define MARISA_STRING_H_

#include <cstring>
#include <string>

#include "./base.h"

namespace marisa {

class RString;

class String {
 public:
  String() : ptr_(NULL), length_(0) {}
  explicit String(const char *str) : ptr_(str), length_(std::strlen(str)) {}
  String(const char *ptr, std::size_t length) : ptr_(ptr), length_(length) {}
  explicit String(const std::string &str)
      : ptr_(str.c_str()), length_(str.length()) {}
  explicit String(const RString &str);
  String(const String &str) : ptr_(str.ptr_), length_(str.length_) {}

  String substr(std::size_t pos, std::size_t length) const {
    return String(ptr_ + pos, length);
  }

  String &operator=(const String &str) {
    ptr_ = str.ptr_;
    length_ = str.length_;
    return *this;
  }

  UInt8 operator[](std::size_t i) const {
    return ptr_[i];
  }

  const char *ptr() const {
    return ptr_;
  }
  std::size_t length() const {
    return length_;
  }

 private:
  const char *ptr_;
  std::size_t length_;
};

inline bool operator==(const String &lhs, const String &rhs) {
  if (lhs.length() != rhs.length()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const String &lhs, const String &rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const String &lhs, const String &rhs) {
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (i == rhs.length()) {
      return false;
    }
    if (lhs[i] != rhs[i]) {
      return lhs[i] < rhs[i];
    }
  }
  return lhs.length() < rhs.length();
}

inline bool operator>(const String &lhs, const String &rhs) {
  return rhs < lhs;
}

class RString {
 public:
  RString() : ptr_(static_cast<const char *>(NULL) - 1), length_(0) {}
  explicit RString(const String &str)
      : ptr_(str.ptr() + str.length() - 1), length_(str.length()) {}
  RString(const RString &str) : ptr_(str.ptr_), length_(str.length_) {}

  RString substr(std::size_t pos, std::size_t length) const {
    RString str(*this);
    str.ptr_ -= pos;
    str.length_ = length;
    return str;
  }

  RString &operator=(const RString &str) {
    ptr_ = str.ptr_;
    length_ = str.length_;
    return *this;
  }

  UInt8 operator[](std::size_t i) const {
    return *(ptr_ - i);
  }

  const char *ptr() const {
    return ptr_ - length_ + 1;
  }
  std::size_t length() const {
    return length_;
  }

 private:
  const char *ptr_;
  std::size_t length_;
};

inline String::String(const RString &str)
    : ptr_(str.ptr()), length_(str.length()) {}

inline bool operator==(const RString &lhs, const RString &rhs) {
  if (lhs.length() != rhs.length()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const RString &lhs, const RString &rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const RString &lhs, const RString &rhs) {
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (i == rhs.length()) {
      return false;
    }
    if (lhs[i] != rhs[i]) {
      return lhs[i] < rhs[i];
    }
  }
  return lhs.length() < rhs.length();
}

inline bool operator>(const RString &lhs, const RString &rhs) {
  return rhs < lhs;
}

}  // namespace marisa

#endif  // MARISA_STRING_H_
