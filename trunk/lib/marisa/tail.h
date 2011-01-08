#ifndef MARISA_TAIL_H_
#define MARISA_TAIL_H_

#include "./string.h"
#include "./vector.h"

namespace marisa {

class Tail {
 public:
  Tail();

  bool build(const std::vector<const char *> &keys,
      std::vector<UInt32> *offsets);
  bool build(const std::vector<char *> &keys,
      std::vector<UInt32> *offsets);
  bool build(const std::vector<std::string> &keys,
      std::vector<UInt32> *offsets);
  bool build(const std::vector<String> &keys,
      std::vector<UInt32> *offsets);

  bool mmap(Mapper *mapper, const char *filename,
      long offset = 0, int whence = SEEK_SET);
  bool map(const void *ptr);
  bool map(const void *ptr, std::size_t size);
  bool map(Mapper *mapper);

  bool load(const char *filename, long offset = 0, int whence = SEEK_SET);
  bool read(int fd);
  bool read(::FILE *file);
  bool read(std::istream *stream);
  bool read(Reader *reader);

  bool save(const char *filename, bool trunc_flag = true,
      long offset = 0, int whence = SEEK_SET) const;
  bool write(int fd) const;
  bool write(::FILE *file) const;
  bool write(std::ostream *stream) const;
  bool write(Writer *writer) const;

  const UInt8 *operator[](UInt32 offset) const {
    return &buf_[offset];
  }

  UInt32 num_objs() const {
    return buf_.num_objs();
  }
  bool empty() const {
    return buf_.empty();
  }
  std::size_t size() const {
    return buf_.size();
  }

  void clear();
  void swap(Tail *rhs);

 private:
  Vector<UInt8> buf_;

  template <typename T>
  bool build_tail(const std::vector<T> &keys,
      std::vector<UInt32> *offsets);

  // Disallows copy and assignment.
  Tail(const Tail &);
  Tail &operator=(const Tail &);
};

}  // namespace marisa

#endif  // MARISA_TAIL_H_
