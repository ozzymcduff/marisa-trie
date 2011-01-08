#ifndef MARISA_BITVECTOR_H_
#define MARISA_BITVECTOR_H_

#include "./rank.h"
#include "./vector.h"

namespace marisa {

class BitVector {
 public:
  BitVector();

  void build();

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

  void push_back(bool bit) {
    if ((num_bits_ % 32) == 0) {
      blocks_.push_back(0);
    }
    if (bit) {
      blocks_.back() |= static_cast<UInt32>(1) << (num_bits_ % 32);
    }
    ++num_bits_;
  }

  bool operator[](UInt32 i) const {
    return (blocks_[i / 32] & (static_cast<UInt32>(1) << (i % 32))) != 0;
  }

  UInt32 rank0(UInt32 i) const {
    return i - rank1(i);
  }
  UInt32 rank1(UInt32 i) const;

  UInt32 select0(UInt32 i) const;
  UInt32 select1(UInt32 i) const;

  UInt32 num_bits() const {
    return num_bits_;
  }
  bool empty() const {
    return blocks_.empty();
  }
  std::size_t size() const {
    return blocks_.size() + sizeof(num_bits_) + ranks_.size()
        + select0s_.size() + select1s_.size();
  }

  void clear();
  void swap(BitVector *rhs);

 private:
  Vector<UInt32> blocks_;
  UInt32 num_bits_;
  Vector<Rank> ranks_;
  Vector<UInt32> select0s_;
  Vector<UInt32> select1s_;

  // Disallows copy and assignment.
  BitVector(const BitVector &);
  BitVector &operator=(const BitVector &);
};

}  // namespace marisa

#endif  // MARISA_BITVECTOR_H_
