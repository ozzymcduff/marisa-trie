#ifndef MARISA_READER_H_
#define MARISA_READER_H_

#include <stdio.h>

#include <iostream>
#include <vector>

#include "./base.h"

namespace marisa {

class Reader {
 public:
  Reader();
  explicit Reader(int fd);
  explicit Reader(::FILE *file);
  explicit Reader(std::istream *stream);
  ~Reader();

  bool is_open() const {
    return (fd_ != -1) || (file_ != NULL) || (stream_ != NULL);
  }

  bool open(const char *filename, long offset = 0, int whence = SEEK_SET);

  template <typename T>
  bool read(T *obj) {
    return read_data(obj, sizeof(T));
  }

  template <typename T>
  bool read(std::vector<T> *vec) {
    UInt32 num_objs;
    if (!read(&num_objs)) {
      return false;
    }
    if (num_objs == 0) {
      std::vector<T>().swap(*vec);
    } else {
      std::vector<T> buf(num_objs);
      if (!read_data(&buf[0], sizeof(T) * num_objs)) {
        return false;
      }
      vec->swap(buf);
    }
    return true;
  }

  void clear();
  void swap(Reader *rhs);

 private:
  int fd_;
  ::FILE *file_;
  std::istream *stream_;
  bool needs_fclose_;

  bool read_data(void *buf, std::size_t size);

  // Disallows copy and assignment.
  Reader(const Reader &);
  Reader &operator=(const Reader &);
};

}  // namespace marisa

#endif  // MARISA_READER_H_
