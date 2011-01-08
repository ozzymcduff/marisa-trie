#if defined _WIN32 || defined _WIN64
#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>
#else  // defined _WIN32 || defined _WIN64
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif  // defined _WIN32 || defined _WIN64

#include <algorithm>
#include <limits>

#include "./mapper.h"

#ifdef max
#undef max
#endif  // max

namespace marisa {

#if defined _WIN32 || defined _WIN64
Mapper::Mapper()
    : ptr_(NULL), avail_(0),
      origin_(NULL), size_(0), file_(NULL), map_(NULL) {}

Mapper::Mapper(const void *ptr)
    : ptr_(ptr), avail_(std::numeric_limits<std::size_t>::max()),
      origin_(NULL), size_(0), file_(NULL), map_(NULL) {}

Mapper::Mapper(const void *ptr, std::size_t size)
    : ptr_(ptr), avail_(size),
      origin_(NULL), size_(0), file_(NULL), map_(NULL) {}
#else  // defined _WIN32 || defined _WIN64
Mapper::Mapper()
    : ptr_(NULL), avail_(0),
      origin_(MAP_FAILED), size_(0), fd_(-1) {}

Mapper::Mapper(const void *ptr)
    : ptr_(ptr), avail_(std::numeric_limits<std::size_t>::max()),
      origin_(MAP_FAILED), size_(0), fd_(-1) {}

Mapper::Mapper(const void *ptr, std::size_t size)
    : ptr_(ptr), avail_(size),
      origin_(MAP_FAILED), size_(0), fd_(-1) {}
#endif  // defined _WIN32 || defined _WIN64

#if defined _WIN32 || defined _WIN64
Mapper::~Mapper() {
  if (origin_ != NULL) {
    ::UnmapViewOfFile(origin_);
  }

  if (map_ != NULL) {
    ::CloseHandle(map_);
  }

  if (file_ != NULL) {
    ::CloseHandle(file_);
  }
}
#else  // defined _WIN32 || defined _WIN64
Mapper::~Mapper() {
  if (origin_ != MAP_FAILED) {
    ::munmap(origin_, size_);
  }

  if (fd_ != -1) {
    ::close(fd_);
  }
}
#endif  // defined _WIN32 || defined _WIN64

#if defined _WIN32 || defined _WIN64
bool Mapper::open(const char *filename, long offset, int whence) {
  if (is_open()) {
    return false;
  }

  Mapper temp;

  struct __stat64 st;
  if (::_stat64(filename, &st) != 0) {
    return false;
  }
  UInt64 file_size = st.st_size;
  if (file_size > std::numeric_limits<std::size_t>::max()) {
    return false;
  }
  temp.size_ = static_cast<std::size_t>(file_size);

  temp.file_ = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (temp.file_ == NULL) {
    return false;
  }

  temp.map_ = ::CreateFileMapping(temp.file_, NULL, PAGE_READONLY, 0, 0, NULL);
  if (temp.map_ == NULL) {
    return false;
  }

  temp.origin_ = ::MapViewOfFile(temp.map_, FILE_MAP_READ, 0, 0, 0);
  if (temp.origin_ == NULL) {
    return false;
  }

  if (!temp.seek(offset, whence)) {
    return false;
  }

  temp.swap(this);
  return true;
}
#else  // defined _WIN32 || defined _WIN64
bool Mapper::open(const char *filename, long offset, int whence) {
  if (is_open()) {
    return false;
  }

  Mapper temp;

  struct stat st;
  if (::stat(filename, &st) != 0) {
    return false;
  }
  UInt64 file_size = st.st_size;
  if (file_size > std::numeric_limits<std::size_t>::max()) {
    return false;
  }
  temp.size_ = static_cast<std::size_t>(file_size);

  temp.fd_ = ::open(filename, O_RDONLY);
  if (temp.fd_ == -1) {
    return false;
  }

  temp.origin_ = ::mmap(NULL, temp.size_, PROT_READ, MAP_SHARED, temp.fd_, 0);
  if (temp.origin_ == MAP_FAILED) {
    return false;
  }

  if (!temp.seek(offset, whence)) {
    return false;
  }

  temp.swap(this);
  return true;
}
#endif  // defined _WIN32 || defined _WIN64

void Mapper::clear() {
  Mapper().swap(this);
}

void Mapper::swap(Mapper *rhs) {
  std::swap(ptr_, rhs->ptr_);
  std::swap(avail_, rhs->avail_);
  std::swap(origin_, rhs->origin_);
  std::swap(size_, rhs->size_);
#if defined _WIN32 || defined _WIN64
  std::swap(file_, rhs->file_);
  std::swap(map_, rhs->map_);
#else  // defined _WIN32 || defined _WIN64
  std::swap(fd_, rhs->fd_);
#endif  // defined _WIN32 || defined _WIN64
}

bool Mapper::seek(long offset, int whence) {
  switch (whence) {
    case SEEK_SET:
    case SEEK_CUR: {
      if ((offset < 0) ||
          (static_cast<unsigned long>(offset) > size_)) {
        return false;
      }
      ptr_ = static_cast<const UInt8 *>(origin_) + offset;
      avail_ = size_ - offset;
      return true;
    }
    case SEEK_END: {
      if ((offset > 0) ||
          (static_cast<unsigned long>(-offset) > size_)) {
        return false;
      }
      ptr_ = static_cast<const UInt8 *>(origin_) + size_ + offset;
      avail_ = -offset;
      return true;
    }
    default: {
      return false;
    }
  }
}

const void *Mapper::map_data(std::size_t size) {
  if (!is_open()) {
    return NULL;
  }
  if (size > avail_) {
    return NULL;
  }
  ptr_ = static_cast<const UInt8 *>(ptr_) + size;
  avail_ -= size;
  return static_cast<const UInt8 *>(ptr_) - size;
}

}  // namespace marisa
