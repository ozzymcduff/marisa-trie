#ifndef MARISA_MAPPER_H_
#define MARISA_MAPPER_H_

#include <stdio.h>

#include "./base.h"

namespace marisa {

class Mapper {
 public:
  Mapper();
  explicit Mapper(const void *ptr);
  Mapper(const void *ptr, std::size_t size);
  ~Mapper();

  bool is_open() const {
    return ptr_ != NULL;
  }

  bool open(const char *filename, long offset = 0, int whence = SEEK_SET);

  template <typename T>
  bool map(T *obj) {
    const void *ptr = map_data(sizeof(T));
    if (ptr == NULL) {
      return false;
    }
    *obj = *static_cast<const T *>(ptr);
    return true;
  }

  template <typename T>
  bool map(const T **objs, UInt32 *num_objs) {
    UInt32 temp_num_objs = 0;
    if (!map(&temp_num_objs)) {
      return false;
    }
    const void *ptr = map_data(sizeof(T) * temp_num_objs);
    if (ptr == NULL) {
      return false;
    }
    *objs = static_cast<const T *>(ptr);
    *num_objs = temp_num_objs;
    return true;
  }

  void clear();
  void swap(Mapper *rhs);

 private:
  const void *ptr_;
  std::size_t avail_;
  void *origin_;
  std::size_t size_;
#if defined _WIN32 || defined _WIN64
  void *file_;
  void *map_;
#else  // defined _WIN32 || defined _WIN64
  int fd_;
#endif  // defined _WIN32 || defined _WIN64

  bool seek(long offset, int whence);

  const void *map_data(std::size_t size);

  // Disallows copy and assignment.
  Mapper(const Mapper &);
  Mapper &operator=(const Mapper &);
};

}  // namespace marisa

#endif  // MARISA_MAPPER_H_
