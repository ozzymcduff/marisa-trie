#include <cassert>

#include <marisa/base.h>

int main() {
  assert(sizeof(marisa_uint8) == 1);
  assert(sizeof(marisa_uint16) == 2);
  assert(sizeof(marisa_uint32) == 4);
  assert(sizeof(marisa_uint64) == 8);

  assert(MARISA_UINT8_MAX == 0xFFU);
  assert(MARISA_UINT16_MAX == 0xFFFFU);
  assert(MARISA_UINT32_MAX == 0xFFFFFFFFU);
  assert(MARISA_UINT64_MAX == 0xFFFFFFFFFFFFFFFFULL);

  assert(sizeof(marisa::UInt8) == 1);
  assert(sizeof(marisa::UInt16) == 2);
  assert(sizeof(marisa::UInt32) == 4);
  assert(sizeof(marisa::UInt64) == 8);

  return 0;
}
