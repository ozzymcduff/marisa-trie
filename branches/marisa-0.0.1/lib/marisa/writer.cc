#include <limits.h>

#ifdef _MSC_VER
#include <io.h>
#else  // _MSC_VER
#include <unistd.h>
#endif  // _MSC_VER

#include "./writer.h"

namespace marisa {

Writer::Writer()
    : fd_(-1), file_(NULL), stream_(NULL), needs_fclose_(false) {}

Writer::Writer(int fd)
    : fd_(fd), file_(NULL), stream_(NULL), needs_fclose_(false) {}

Writer::Writer(::FILE *file)
    : fd_(-1), file_(file), stream_(NULL), needs_fclose_(false) {}

Writer::Writer(std::ostream *stream)
    : fd_(-1), file_(NULL), stream_(stream), needs_fclose_(false) {}

Writer::~Writer() {
  if (needs_fclose_) {
    ::fclose(file_);
  }
}

bool Writer::open(const char *filename, bool trunc_flag,
    long offset, int whence) {
  if (is_open()) {
    return false;
  }
#ifdef _MSC_VER
  ::FILE *file = NULL;
  if (!trunc_flag) {
    ::fopen_s(&file, filename, "rb+");
  }
  if (file == NULL) {
    if (::fopen_s(&file, filename, "wb") != 0) {
      return false;
    }
  }
#else  // _MSC_VER
  ::FILE *file = NULL;
  if (!trunc_flag) {
    file = ::fopen(filename, "rb+");
  }
  if (file == NULL) {
    file = ::fopen(filename, "wb");
    if (file == NULL) {
      return false;
    }
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

bool Writer::write_data(const void *data, std::size_t size) {
  if (fd_ != -1) {
    while (size != 0) {
#ifdef _MSC_VER
      unsigned int count = (size < INT_MAX) ? size : INT_MAX;
      int size_written = _write(fd_, data, count);
#else  // _MSC_VER
      ::size_t count = (size < SSIZE_MAX) ? size : SSIZE_MAX;
      ::ssize_t size_written = ::write(fd_, data, count);
#endif  // _MSC_VER
      if (size_written <= 0) {
        return false;
      }
      data = static_cast<const char *>(data) + size_written;
      size -= size_written;
    }
    return true;
  } else if (file_ != NULL) {
    if (::fwrite(data, 1, size, file_) != size) {
      return false;
    } else if (::fflush(file_) != 0) {
      return false;
    }
    return true;
  } else if (stream_ != NULL) {
    if (!stream_->write(static_cast<const char *>(data), size)) {
      return false;
    } else if (!stream_->flush()) {
      return false;
    }
    return true;
  }
  return false;
}

void Writer::clear() {
  Writer().swap(this);
}

void Writer::swap(Writer *rhs) {
  std::swap(fd_, rhs->fd_);
  std::swap(file_, rhs->file_);
  std::swap(stream_, rhs->stream_);
  std::swap(needs_fclose_, rhs->needs_fclose_);
}

}  // namespace marisa
