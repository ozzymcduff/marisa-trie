#include <limits.h>

#ifdef _MSC_VER
#include <io.h>
#else  // _MSC_VER
#include <unistd.h>
#endif  // _MSC_VER

#include <algorithm>

#include "./reader.h"

namespace marisa {

Reader::Reader()
    : fd_(-1), file_(NULL), stream_(NULL), needs_fclose_(false) {}

Reader::Reader(int fd)
    : fd_(fd), file_(NULL), stream_(NULL), needs_fclose_(false) {}

Reader::Reader(::FILE *file)
    : fd_(-1), file_(file), stream_(NULL), needs_fclose_(false) {}

Reader::Reader(std::istream *stream)
    : fd_(-1), file_(NULL), stream_(stream), needs_fclose_(false) {}

Reader::~Reader() {
  if (needs_fclose_) {
    ::fclose(file_);
  }
}

bool Reader::open(const char *filename, long offset, int whence) {
  if (is_open()) {
    return false;
  }
#ifdef _MSC_VER
  ::FILE *file = NULL;
  if (::fopen_s(&file, filename, "rb") != 0) {
    return false;
  }
#else  // _MSC_VER
  ::FILE *file = ::fopen(filename, "rb");
  if (file == NULL) {
    return false;
  }
#endif  // _MSC_VER
  if (::fseek(file, offset, whence) != 0) {
    ::fclose(file);
    return false;
  }
  file_ = file;
  needs_fclose_ = true;
  return true;
}

void Reader::clear() {
  Reader().swap(this);
}

void Reader::swap(Reader *rhs) {
  std::swap(fd_, rhs->fd_);
  std::swap(file_, rhs->file_);
  std::swap(stream_, rhs->stream_);
  std::swap(needs_fclose_, rhs->needs_fclose_);
}

bool Reader::read_data(void *buf, std::size_t size) {
  if (fd_ != -1) {
    while (size != 0) {
#ifdef _MSC_VER
      unsigned int count = (size < INT_MAX) ? size : INT_MAX;
      int size_read = _read(fd_, buf, count);
#else  // _MSC_VER
      ::size_t count = (size < SSIZE_MAX) ? size : SSIZE_MAX;
      ::ssize_t size_read = ::read(fd_, buf, count);
#endif  // _MSC_VER
      if (size_read <= 0) {
        return false;
      }
      buf = static_cast<char *>(buf) + size_read;
      size -= size_read;
    }
    return true;
  } else if (file_ != NULL) {
    return ::fread(buf, 1, size, file_) == size;
  } else if (stream_ != NULL) {
    return !stream_->read(static_cast<char *>(buf), size) ? false : true;
  }
  return false;
}

}  // namespace marisa
