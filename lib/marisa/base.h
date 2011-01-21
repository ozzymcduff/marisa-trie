#ifndef MARISA_BASE_H_
#define MARISA_BASE_H_

#ifndef _MSC_VER
#include <stdint.h>
#endif  // _MSC_VER

#ifdef __cplusplus
#include <cstddef>
#include <new>
#else  // __cplusplus
#include <stddef.h>
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#ifdef _MSC_VER
typedef unsigned __int8  marisa_uint8;
typedef unsigned __int16 marisa_uint16;
typedef unsigned __int32 marisa_uint32;
typedef unsigned __int64 marisa_uint64;
#else  // _MSC_VER
typedef uint8_t  marisa_uint8;
typedef uint16_t marisa_uint16;
typedef uint32_t marisa_uint32;
typedef uint64_t marisa_uint64;
#endif  // _MSC_VER

#define MARISA_UINT8_MAX  ((marisa_uint8)-1)
#define MARISA_UINT16_MAX ((marisa_uint16)-1)
#define MARISA_UINT32_MAX ((marisa_uint32)-1)
#define MARISA_UINT64_MAX ((marisa_uint64)-1)

#define MARISA_ZERO_TERMINATED MARISA_UINT32_MAX
#define MARISA_NOT_FOUND       MARISA_UINT32_MAX
#define MARISA_MISMATCH        MARISA_UINT32_MAX

#define MARISA_MAX_LENGTH     (MARISA_UINT32_MAX - 1)
#define MARISA_MAX_NUM_KEYS   (MARISA_UINT32_MAX - 1)

typedef enum marisa_status_ {
  // If a function succeeds.
  MARISA_OK               = 0,

  // If a given handle is wrong.
  MARISA_HANDLE_ERROR     = 1,

  // If an internal state is invalid.
  MARISA_STATE_ERROR      = 2,

  // If a given argument is obviously wrong.
  MARISA_PARAM_ERROR      = 3,

  // If a size is too large.
  MARISA_SIZE_ERROR       = 4,

  // If a memory allocation fails or a given buffer is too small.
  MARISA_MEMORY_ERROR     = 5,

  // If an I/O function fails.
  MARISA_IO_ERROR         = 6,

  // In other cases.
  MARISA_UNEXPECTED_ERROR = 7
} marisa_status;

typedef enum marisa_flags_ {
  MARISA_MIN_NUM_TRIES     = 0x00001,
  MARISA_MAX_NUM_TRIES     = 0x000FF,
  MARISA_DEFAULT_NUM_TRIES = 0x00003,

  MARISA_PATRICIA_TRIE     = 0x00100,
  MARISA_PREFIX_TRIE       = 0x00200,
  MARISA_DEFAULT_TRIE      = MARISA_PATRICIA_TRIE,

  MARISA_WITHOUT_TAIL      = 0x01000,
  MARISA_BINARY_TAIL       = 0x02000,
  MARISA_TEXT_TAIL         = 0x04000,
  MARISA_DEFAULT_TAIL      = MARISA_TEXT_TAIL,

  MARISA_LABEL_ORDER       = 0x10000,
  MARISA_WEIGHT_ORDER      = 0x20000,
  MARISA_DEFAULT_ORDER     = MARISA_WEIGHT_ORDER,

  MARISA_DEFAULT_FLAGS     = MARISA_DEFAULT_NUM_TRIES
      | MARISA_DEFAULT_TRIE | MARISA_DEFAULT_TAIL | MARISA_DEFAULT_ORDER,

  MARISA_NUM_TRIES_MASK    = 0x000FF,
  MARISA_TRIE_MASK         = 0x00F00,
  MARISA_TAIL_MASK         = 0x0F000,
  MARISA_ORDER_MASK        = 0xF0000,
  MARISA_FLAGS_MASK        = 0xFFFFF
} marisa_flags;

const char *marisa_strerror(marisa_status status);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

//#include <cstddef>

#ifdef __cplusplus
namespace marisa {

typedef ::marisa_uint8  UInt8;
typedef ::marisa_uint16 UInt16;
typedef ::marisa_uint32 UInt32;
typedef ::marisa_uint64 UInt64;

typedef ::marisa_status Status;

class Exception {
 public:
  explicit Exception(const char *filename, int line, Status status)
      : filename_(filename), line_(line), status_(status) {}
  Exception(const Exception &ex)
      : filename_(ex.filename_), line_(ex.line_), status_(ex.status_) {}

  Exception &operator=(const Exception &rhs) {
    filename_ = rhs.filename_;
    line_ = rhs.line_;
    status_ = rhs.status_;
    return *this;
  }

  const char *filename() const {
    return filename_;
  }
  int line() const {
    return line_;
  }
  Status status() const {
    return status_;
  }

  const char *what() const {
    return ::marisa_strerror(status_);
  }

 private:
  const char *filename_;
  int line_;
  Status status_;
};

#define MARISA_THROW(status) \
  throw Exception(__FILE__, __LINE__, status)

#define MARISA_THROW_IF(condition, status) \
  do { if (condition) MARISA_THROW(status); } while (false)

#ifdef DEBUG
#define MARISA_DEBUG_IF(condition, status) \
  MARISA_THROW_IF(condition, status)
#else
#define MARISA_DEBUG_IF(condition, status)
#endif

void *Malloc(std::size_t size);
void Free(void *ptr);

template <typename T>
void Swap(T *lhs, T *rhs) {
  T temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

}  // namespace marisa
#endif  // __cplusplus

#endif  // MARISA_BASE_H_
