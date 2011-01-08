#include <cassert>

#include <marisa/intvector.h>

int main() {
  marisa::IntVector vec;
  assert(vec.num_bits_per_int() == 0);
  assert(vec.mask() == 0);
  assert(vec.num_ints() == 0);
  assert(vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32) * 4);

  std::vector<marisa::UInt32> values;
  values.push_back(0);

  vec.build(values);
  assert(vec.num_bits_per_int() == 1);
  assert(vec.mask() == 0x1);
  assert(vec.num_ints() == 1);
  assert(!vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32) * 5);
  assert(vec[0] == 0);

  values.push_back(255);

  vec.build(values);
  assert(vec.num_bits_per_int() == 8);
  assert(vec.mask() == 0xFF);
  assert(vec.num_ints() == 2);
  assert(vec[0] == 0);
  assert(vec[1] == 255);

  values.push_back(65536);

  vec.build(values);
  assert(vec.num_bits_per_int() == 17);
  assert(vec.mask() == 0x1FFFF);
  assert(vec.num_ints() == 3);
  assert(vec[0] == 0);
  assert(vec[1] == 255);
  assert(vec[2] == 65536);

  assert(vec.save("intvector-test.dat"));

  vec.clear();
  assert(vec.num_bits_per_int() == 0);
  assert(vec.mask() == 0);
  assert(vec.num_ints() == 0);

  marisa::Mapper mapper;
  assert(vec.mmap(&mapper, "intvector-test.dat"));
  assert(mapper.is_open());
  assert(vec.num_bits_per_int() == 17);
  assert(vec.mask() == 0x1FFFF);
  assert(vec.num_ints() == 3);
  assert(vec[0] == 0);
  assert(vec[1] == 255);
  assert(vec[2] == 65536);

  vec.clear();
  assert(vec.load("intvector-test.dat"));
  assert(vec.num_bits_per_int() == 17);
  assert(vec.mask() == 0x1FFFF);
  assert(vec.num_ints() == 3);
  assert(vec[0] == 0);
  assert(vec[1] == 255);
  assert(vec[2] == 65536);

  values.clear();
  for (std::size_t i = 0; i < 500; ++i) {
    values.push_back(i);
  }
  vec.build(values);
  assert(vec.num_ints() == values.size());
  for (marisa::UInt32 i = 0; i < vec.num_ints(); ++i) {
    assert(vec[i] == values[i]);
  }

  return 0;
}
