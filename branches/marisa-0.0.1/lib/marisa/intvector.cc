#include <algorithm>

#include "./intvector.h"

namespace marisa {

IntVector::IntVector()
    : units_(), num_bits_per_int_(0), mask_(0), num_ints_(0) {}

void IntVector::build(const std::vector<UInt32> &ints) {
  UInt32 max_int = ints.empty() ?
      0 : *std::max_element(ints.begin(), ints.end());;
  build(max_int, ints.size());
  for (std::size_t i = 0; i < ints.size(); ++i) {
    set(i, ints[i]);
  }
}

void IntVector::build(UInt32 max_int, UInt32 num_ints) {
  UInt32 num_bits_per_int = 0;
  do {
    ++num_bits_per_int;
    max_int >>= 1;
  } while (max_int != 0);

  Vector<UInt32> units(((num_bits_per_int * num_ints) + 31) / 32, 0);
  units_.swap(&units);
  num_bits_per_int_ = num_bits_per_int;
  mask_ = (num_bits_per_int == 32) ? ~0UL : ((1UL << num_bits_per_int) - 1);
  num_ints_ = num_ints;
}

bool IntVector::mmap(Mapper *mapper, const char *filename,
    long offset, int whence) {
  Mapper temp_mapper;
  if (!temp_mapper.open(filename, offset, whence) || !map(&temp_mapper)) {
    return false;
  }
  temp_mapper.swap(mapper);
  return true;
}

bool IntVector::map(const void *ptr) {
  Mapper mapper(ptr);
  return map(&mapper);
}

bool IntVector::map(const void *ptr, std::size_t size) {
  Mapper mapper(ptr, size);
  return map(&mapper);
}

bool IntVector::map(Mapper *mapper) {
  IntVector temp;
  if (!temp.units_.map(mapper) ||
      !mapper->map(&temp.num_bits_per_int_) ||
      !mapper->map(&temp.mask_) ||
      !mapper->map(&temp.num_ints_)) {
    return false;
  }
  temp.swap(this);
  return true;
}

bool IntVector::load(const char *filename, long offset, int whence) {
  Reader reader;
  if (!reader.open(filename, offset, whence)) {
    return false;
  }
  return read(&reader);
}

bool IntVector::read(int fd) {
  Reader reader(fd);
  return read(&reader);
}

bool IntVector::read(::FILE *file) {
  Reader reader(file);
  return read(&reader);
}

bool IntVector::read(std::istream *stream) {
  Reader reader(stream);
  return read(&reader);
}

bool IntVector::read(Reader *reader) {
  IntVector temp;
  if (!temp.units_.read(reader) ||
      !reader->read(&temp.num_bits_per_int_) ||
      !reader->read(&temp.mask_) ||
      !reader->read(&temp.num_ints_)) {
    return false;
  }
  temp.swap(this);
  return true;
}

bool IntVector::save(const char *filename, bool trunc_flag,
    long offset, int whence) const {
  Writer writer;
  if (!writer.open(filename, trunc_flag, offset, whence)) {
    return false;
  }
  return write(&writer);
}

bool IntVector::write(int fd) const {
  Writer writer(fd);
  return write(&writer);
}

bool IntVector::write(::FILE *file) const {
  Writer writer(file);
  return write(&writer);
}

bool IntVector::write(std::ostream *stream) const {
  Writer writer(stream);
  return write(&writer);
}

bool IntVector::write(Writer *writer) const {
  return units_.write(writer) &&
      writer->write(num_bits_per_int_) &&
      writer->write(mask_) &&
      writer->write(num_ints_);
}

void IntVector::clear() {
  IntVector().swap(this);
}

void IntVector::swap(IntVector *rhs) {
  units_.swap(&rhs->units_);
  std::swap(num_bits_per_int_, rhs->num_bits_per_int_);
  std::swap(mask_, rhs->mask_);
  std::swap(num_ints_, rhs->num_ints_);
}

}  // namespace marisa
