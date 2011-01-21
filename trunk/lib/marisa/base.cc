#include <cstdlib>

#include "base.h"

extern "C" {

const char *marisa_strerror(marisa_status status) {
#define MARISA_STRERROR_CASE(x) case x: { return #x; }
  switch (status) {
    MARISA_STRERROR_CASE(MARISA_OK)
    MARISA_STRERROR_CASE(MARISA_HANDLE_ERROR)
    MARISA_STRERROR_CASE(MARISA_STATE_ERROR)
    MARISA_STRERROR_CASE(MARISA_PARAM_ERROR)
    MARISA_STRERROR_CASE(MARISA_SIZE_ERROR)
    MARISA_STRERROR_CASE(MARISA_MEMORY_ERROR)
    MARISA_STRERROR_CASE(MARISA_IO_ERROR)
    MARISA_STRERROR_CASE(MARISA_UNEXPECTED_ERROR)
  }
  return "MARISA_UNKNOWN_ERROR";
#undef MARISA_STRERROR_CASE
}

}  // extern "C"

namespace marisa {

void *Malloc(std::size_t size) {
  if (size == 0) {
    return NULL;
  }
  void *ptr = std::malloc(size);
  MARISA_THROW_IF(ptr == NULL, MARISA_MEMORY_ERROR);
  return ptr;
}

void Free(void *ptr) {
  std::free(ptr);
}

}  // namespace marisa
