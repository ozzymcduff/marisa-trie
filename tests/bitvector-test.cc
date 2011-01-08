#include <cassert>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include <marisa/bitvector.h>

namespace {

void Test(marisa::UInt32 size) {
  marisa::BitVector bv;
  assert(bv.num_bits() == 0);
  assert(bv.empty());
  assert(bv.size() == sizeof(marisa::UInt32) * 5);

  std::vector<bool> bits(size);
  std::vector<marisa::UInt32> zeros, ones;
  for (marisa::UInt32 i = 0; i < size; ++i) {
    bool bit = (std::rand() % 2) == 0;
    bits[i] = bit;
    bv.push_back(bit);
    if (bit) {
      ones.push_back(i);
    } else {
      zeros.push_back(i);
    }
  }
  assert(bv.num_bits() == bits.size());
  assert((size == 0) || !bv.empty());

  bv.build();
  marisa::UInt32 num_zeros = 0, num_ones = 0;
  for (std::size_t i = 0; i < bits.size(); ++i) {
    assert(bits[i] == bv[i]);
    assert(num_zeros == bv.rank0(i));
    assert(num_ones == bv.rank1(i));
    if (bv[i]) {
      ++num_ones;
    } else {
      ++num_zeros;
    }
  }

  for (std::size_t i = 0; i < zeros.size(); ++i) {
    assert(zeros[i] == bv.select0(i));
  }
  for (std::size_t i = 0; i < ones.size(); ++i) {
    assert(ones[i] == bv.select1(i));
  }

  assert(bv.save("bitvector-test.dat"));

  bv.clear();
  assert(bv.num_bits() == 0);
  assert(bv.empty());
  assert(bv.size() == sizeof(marisa::UInt32) * 5);

  marisa::Mapper mapper;
  assert(bv.mmap(&mapper, "bitvector-test.dat"));
  assert(bv.num_bits() == bits.size());

  num_zeros = 0, num_ones = 0;
  for (std::size_t i = 0; i < bits.size(); ++i) {
    assert(bits[i] == bv[i]);
    assert(num_zeros == bv.rank0(i));
    assert(num_ones == bv.rank1(i));
    if (bv[i]) {
      ++num_ones;
    } else {
      ++num_zeros;
    }
  }

  for (std::size_t i = 0; i < zeros.size(); ++i) {
    assert(zeros[i] == bv.select0(i));
  }
  for (std::size_t i = 0; i < ones.size(); ++i) {
    assert(ones[i] == bv.select1(i));
  }

  std::stringstream stream;
  assert(bv.write(&stream));

  bv.clear();
  assert(bv.read(&stream));
  assert(bv.num_bits() == bits.size());

  num_zeros = 0, num_ones = 0;
  for (std::size_t i = 0; i < bits.size(); ++i) {
    assert(bits[i] == bv[i]);
    assert(num_zeros == bv.rank0(i));
    assert(num_ones == bv.rank1(i));
    if (bv[i]) {
      ++num_ones;
    } else {
      ++num_zeros;
    }
  }

  for (std::size_t i = 0; i < zeros.size(); ++i) {
    assert(zeros[i] == bv.select0(i));
  }
  for (std::size_t i = 0; i < ones.size(); ++i) {
    assert(ones[i] == bv.select1(i));
  }
}

}  // namespace

int main() {
  std::srand(static_cast<unsigned int>(time(NULL)));
  for (marisa::UInt32 i = 0; i <= 512; ++i) {
    Test(i);
  }
  return 0;
}
