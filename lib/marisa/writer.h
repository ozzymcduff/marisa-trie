#ifndef MARISA_WRITER_H_
#define MARISA_WRITER_H_

#include <stdio.h>

#include <iostream>
#include <vector>

#include "./base.h"

namespace marisa {

class Writer {
 public:
  Writer();
  explicit Writer(int fd);
  explicit Writer(::FILE *file);
  explicit Writer(std::ostream *stream);
  ~Writer();

  bool is_open() const {
    return (fd_ != -1) || (file_ != NULL) || (stream_ != NULL);
  }

  bool open(const char *filename, bool trunc_flag = true,
      long offset = 0, int whence = SEEK_SET);

  template <typename T>
  bool write(const T &obj) {
    return write_data(&obj, sizeof(T));
  }

  template <typename T>
  bool write(const T *objs, std::size_t num_objs) {
    if (!write(static_cast<UInt32>(num_objs))) {
      return false;
    }
    return (num_objs == 0) ? true : write_data(objs, sizeof(T) * num_objs);
  }

  template <typename T>
  bool write(const std::vector<T> &vec) {
    if (!write(static_cast<UInt32>(vec.size()))) {
      return false;
    }
    return vec.empty() ? true : write_data(&vec[0], sizeof(T) * vec.size());
  }

  void clear();
  void swap(Writer *rhs);

 private:
  int fd_;
  ::FILE *file_;
  std::ostream *stream_;
  bool needs_fclose_;

  bool write_data(const void *data, std::size_t size);

  // Disallows copy and assignment.
  Writer(const Writer &);
  Writer &operator=(const Writer &);
};

}  // namespace marisa

#endif  // MARISA_WRITER_H_
