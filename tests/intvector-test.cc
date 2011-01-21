#include <cassert>

#include <marisa/intvector.h>

int main() {
  marisa::IntVector vec;
  assert(vec.num_bits_per_int() == 0);
  assert(vec.mask() == 0);
  assert(vec.size() == 0);
  assert(vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32) * 4);

  marisa::Vector<marisa::UInt32> values;
  values.push_back(0);

  vec.build(values);
  assert(vec.num_bits_per_int() == 1);
  assert(vec.mask() == 0x1);
  assert(vec.size() == 1);
  assert(!vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32) * 5);
  assert(vec[0] == 0);

  values.push_back(255);

  vec.build(values);
  assert(vec.num_bits_per_int() == 8);
  assert(vec.mask() == 0xFF);
  assert(vec.size() == 2);
  assert(vec[0] == 0);
  assert(vec[1] == 255);

  values.push_back(65536);

  vec.build(values);
  assert(vec.num_bits_per_int() == 17);
  assert(vec.mask() == 0x1FFFF);
  assert(vec.size() == 3);
  assert(vec[0] == 0);
  assert(vec[1] == 255);
  assert(vec[2] == 65536);

  vec.save("intvector-test.dat");

  vec.clear();
  assert(vec.num_bits_per_int() == 0);
  assert(vec.mask() == 0);
  assert(vec.size() == 0);

  marisa::Mapper mapper;
  vec.mmap(&mapper, "intvector-test.dat");
  assert(mapper.is_open());

  assert(vec.num_bits_per_int() == 17);
  assert(vec.mask() == 0x1FFFF);
  assert(vec.size() == 3);
  assert(vec[0] == 0);
  assert(vec[1] == 255);
  assert(vec[2] == 65536);

  vec.clear();
  vec.load("intvector-test.dat");
  assert(vec.num_bits_per_int() == 17);
  assert(vec.mask() == 0x1FFFF);
  assert(vec.size() == 3);
  assert(vec[0] == 0);
  assert(vec[1] == 255);
  assert(vec[2] == 65536);

  values.clear();
  for (std::size_t i = 0; i < 500; ++i) {
    values.push_back(i);
  }
  vec.build(values);
  assert(vec.size() == values.size());
  for (marisa::UInt32 i = 0; i < vec.size(); ++i) {
    assert(vec[i] == values[i]);
  }

  try {
    vec.build(MARISA_UINT32_MAX, MARISA_UINT32_MAX - 3);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_SIZE_ERROR);
  }

  return 0;
}
