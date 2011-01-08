#ifndef MARISA_VECTOR_H_
#define MARISA_VECTOR_H_

#include "./io.h"

namespace marisa {

template <typename T>
class Vector {
 public:
  Vector() : buf_(), ptr_(NULL), num_objs_(0) {}
  explicit Vector(UInt32 num_objs)
      : buf_(num_objs), ptr_(get_ptr(buf_)), num_objs_(num_objs) {}
  Vector(UInt32 num_objs, const T &value)
      : buf_(num_objs, value), ptr_(get_ptr(buf_)), num_objs_(num_objs) {}
  Vector(const Vector &vec)
      : buf_(vec.buf_), ptr_(get_ptr(buf_)), num_objs_(vec.num_objs_) {}

  bool mmap(Mapper *mapper, const char *filename,
      long offset = 0, int whence = SEEK_SET) {
    Mapper temp_mapper;
    if (!temp_mapper.open(filename, offset, whence) || !map(&temp_mapper)) {
      return false;
    }
    temp_mapper.swap(mapper);
    return true;
  }
  bool map(const void *ptr) {
    Mapper mapper(ptr);
    return map(&mapper);
  }
  bool map(const void *ptr, std::size_t size) {
    Mapper mapper(ptr, size);
    return map(&mapper);
  }
  bool map(Mapper *mapper) {
    Vector temp;
    if (!mapper->map(&temp.ptr_, &temp.num_objs_)) {
      return false;
    }
    temp.swap(this);
    return true;
  }

  bool load(const char *filename, long offset = 0, int whence = SEEK_SET) {
    Reader reader;
    if (!reader.open(filename, offset, whence)) {
      return false;
    }
    return read(&reader);
  }
  bool read(int fd) {
    Reader reader(fd);
    return read(&reader);
  }
  bool read(::FILE *file) {
    Reader reader(file);
    return read(&reader);
  }
  bool read(std::istream *stream) {
    Reader reader(stream);
    return read(&reader);
  }
  bool read(Reader *reader) {
    Vector temp;
    if (!reader->read(&temp.buf_)) {
      return false;
    }
    temp.ptr_ = get_ptr(temp.buf_);
    temp.num_objs_ = temp.buf_.size();
    temp.swap(this);
    return true;
  }

  bool save(const char *filename, bool trunc_flag = false,
    long offset = 0, int whence = SEEK_SET) const {
    Writer writer;
    if (!writer.open(filename, trunc_flag, offset, whence)) {
      return false;
    }
    return write(&writer);
  }
  bool write(int fd) const {
    Writer writer(fd);
    return write(&writer);
  }
  bool write(::FILE *file) const {
    Writer writer(file);
    return write(&writer);
  }
  bool write(std::ostream *stream) const {
    Writer writer(stream);
    return write(&writer);
  }
  bool write(Writer *writer) const {
    return writer->write(ptr_, num_objs_);
  }

  void push_back(const T &value) {
    buf_.push_back(value);
    ptr_ = &buf_[0];
    num_objs_ = buf_.size();
  }

  void shrink() {
    if (buf_.size() != buf_.capacity()) {
      Vector(*this).swap(this);
    }
  }

  const T &operator[](UInt32 i) const {
    return ptr_[i];
  }
  T &operator[](UInt32 i) {
    return buf_[i];
  }

  const T &front() const {
    return ptr_[0];
  }
  T &front() {
    return buf_.front();
  }

  const T &back() const {
    return ptr_[num_objs_ - 1];
  }
  T &back() {
    return buf_.back();
  }

  UInt32 num_objs() const {
    return num_objs_;
  }
  std::size_t capacity() const {
    return buf_.capacity();
  }
  bool empty() const {
    return num_objs_ == 0;
  }
  std::size_t size() const {
    return (sizeof(T) * num_objs_) + sizeof(num_objs_);
  }

  void clear() {
    Vector().swap(this);
  }
  void swap(Vector *rhs) {
    buf_.swap(rhs->buf_);

    const T *temp_ptr = ptr_;
    ptr_ = rhs->ptr_;
    rhs->ptr_ = temp_ptr;

    UInt32 temp_num_objs = num_objs_;
    num_objs_ = rhs->num_objs_;
    rhs->num_objs_ = temp_num_objs;
  }

 private:
  std::vector<T> buf_;
  const T *ptr_;
  UInt32 num_objs_;

  static const T *get_ptr(const std::vector<T> &buf) {
    return buf.empty() ? NULL : &buf[0];
  }

  // Disallows assignment.
  Vector &operator=(const Vector &);
};

}  // namespace marisa

#endif  // MARISA_VECTOR_H_
