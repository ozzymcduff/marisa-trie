#include <cassert>

#include <marisa/base.h>

int main() {
  assert(sizeof(marisa::UInt8) == 1);
  assert(sizeof(marisa::UInt16) == 2);
  assert(sizeof(marisa::UInt32) == 4);
  assert(sizeof(marisa::UInt64) == 8);

  return 0;
}
