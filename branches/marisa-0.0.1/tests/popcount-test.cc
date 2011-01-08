#include <cassert>
#include <cstdlib>

#include <marisa/popcount.h>

namespace {

marisa::UInt32 Count(marisa::UInt32 x) {
  marisa::UInt32 count = 0;
  while (x) {
    count += x & 1;
    x >>= 1;
  }
  return count;
}

}  // namespace

int main() {
  assert(marisa::PopCount(0).lo8() == 0);
  assert(marisa::PopCount(0).lo16() == 0);
  assert(marisa::PopCount(0).lo24() == 0);
  assert(marisa::PopCount(0).lo32() == 0);
  assert(marisa::PopCount(0) == 0);

  assert(marisa::PopCount(0xFFFFFFFFU).lo8() == 8);
  assert(marisa::PopCount(0xFFFFFFFFU).lo16() == 16);
  assert(marisa::PopCount(0xFFFFFFFFU).lo24() == 24);
  assert(marisa::PopCount(0xFFFFFFFFU).lo32() == 32);
  assert(marisa::PopCount(0xFFFFFFFFU) == 32);

  for (std::size_t i = 0; i < 1000; ++i) {
    marisa::UInt32 value = std::rand();
    marisa::PopCount popcount(value);
    assert(popcount.lo8() == Count(value & 0xFFU));
    assert(popcount.lo16() == Count(value & 0xFFFFU));
    assert(popcount.lo24() == Count(value & 0xFFFFFFU));
    assert(popcount.lo32() == Count(value));
    assert(popcount == Count(value));
  }
  return 0;
}
