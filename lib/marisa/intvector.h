#ifndef MARISA_INTVECTOR_H_
#define MARISA_INTVECTOR_H_

#include "./vector.h"

namespace marisa {

class IntVector {
 public:
  IntVector();

  void build(const std::vector<UInt32> &ints);
  void build(UInt32 max_int, UInt32 num_ints);

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

  void set(UInt32 i, UInt32 value) {
    UInt32 pos = i * num_bits_per_int_;
    UInt32 unit_id = pos / 32;
    UInt32 unit_offset = pos % 32;
    units_[unit_id] &= ~(mask_ << unit_offset);
    units_[unit_id] |= (value & mask_) << unit_offset;
    if (unit_offset + num_bits_per_int_ > 32) {
      units_[unit_id + 1] &= ~(mask_ >> (32 - unit_offset));
      units_[unit_id + 1] |= (value & mask_) >> (32 - unit_offset);
    }
  }

  UInt32 get(UInt32 i) const {
    UInt32 pos = i * num_bits_per_int_;
    UInt32 unit_id = pos / 32;
    UInt32 unit_offset = pos % 32;
    if (unit_offset + num_bits_per_int_ <= 32) {
      return (units_[unit_id] >> unit_offset) & mask_;
    } else {
      return ((units_[unit_id] >> unit_offset)
          | (units_[unit_id + 1] << (32 - unit_offset))) & mask_;
    }
  }

  UInt32 operator[](UInt32 i) const {
    return get(i);
  }

  UInt32 num_bits_per_int() const {
    return num_bits_per_int_;
  }
  UInt32 mask() const {
    return mask_;
  }
  UInt32 num_ints() const {
    return num_ints_;
  }
  bool empty() const {
    return num_ints_ == 0;
  }
  std::size_t size() const {
    return units_.size() + sizeof(num_bits_per_int_)
        + sizeof(mask_) + sizeof(num_ints_);
  }

  void clear();
  void swap(IntVector *rhs);

 private:
  Vector<UInt32> units_;
  UInt32 num_bits_per_int_;
  UInt32 mask_;
  UInt32 num_ints_;

  // Disallows copy and assignment.
  IntVector(const IntVector &);
  IntVector &operator=(const IntVector &);
};

}  // namespace marisa

#endif  // MARISA_INTVECTOR_H_
