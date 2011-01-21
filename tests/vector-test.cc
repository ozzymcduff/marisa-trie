#include <cassert>
#include <cstdlib>
#include <ctime>
#include <vector>

#include <marisa/vector.h>

int main() {
  std::srand(static_cast<unsigned int>(time(NULL)));

  std::vector<int> values;
  for (std::size_t i = 0; i < 1000; ++i) {
    values.push_back(std::rand());
  }

  marisa::Vector<int> vec;
  assert(vec.max_size() ==
      (MARISA_UINT32_MAX - sizeof(marisa::UInt32)) / sizeof(int));

  assert(vec.size() == 0);
  assert(vec.capacity() == 0);
  assert(!vec.fixed());
  assert(vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32));

  for (std::size_t i = 0; i < values.size(); ++i) {
    vec.push_back(values[i]);
    assert(vec[i] == values[i]);
  }

  assert(vec.size() == values.size());
  assert(vec.capacity() >= vec.size());
  assert(!vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  assert(static_cast<const marisa::Vector<int> &>(vec).front()
      == values.front());
  assert(static_cast<const marisa::Vector<int> &>(vec).back()
      == values.back());

  assert(vec.front() == values.front());
  assert(vec.back() == values.back());

  vec.shrink();

  assert(vec.size() == values.size());
  assert(vec.capacity() == vec.size());
  assert(!vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(vec[i] == values[i]);
    assert(static_cast<const marisa::Vector<int> &>(vec)[i] == values[i]);
  }

  vec.save("vector-test.dat");

  vec.clear();

  assert(vec.size() == 0);
  assert(vec.capacity() == 0);
  assert(vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32));

  marisa::Mapper mapper;
  vec.mmap(&mapper, "vector-test.dat");
  assert(mapper.is_open());

  assert(vec.size() == values.size());
  assert(vec.capacity() == 0);
  assert(vec.fixed());
  assert(!vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(static_cast<const marisa::Vector<int> &>(vec)[i] == values[i]);
  }

  vec.clear();
  vec.load("vector-test.dat");
  assert(vec.size() == values.size());
  assert(vec.capacity() == vec.size());
  assert(!vec.empty());
  assert(vec.total_size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(vec[i] == values[i]);
    assert(static_cast<const marisa::Vector<int> &>(vec)[i] == values[i]);
  }

  try {
    vec.resize(1U << 30);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_SIZE_ERROR);
  }
  try {
    vec.reserve(1U << 30);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_SIZE_ERROR);
  }

  vec.clear();

  vec.push_back(0);
  assert(vec.capacity() == 1);
  vec.push_back(1);
  assert(vec.capacity() == 2);
  vec.push_back(2);
  assert(vec.capacity() == 4);
  vec.push_back(3);
  assert(vec.capacity() == 4);
  vec.push_back(4);
  assert(vec.capacity() == 8);
  vec.resize(13);
  assert(vec.capacity() == 16);
  vec.resize(100);
  assert(vec.capacity() == 100);

  assert(!vec.fixed());
  vec.fix();
  assert(vec.fixed());
  try {
    vec.fix();
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_STATE_ERROR);
  }
  try {
    vec.push_back(0);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_STATE_ERROR);
  }
  try {
    vec.resize(0);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_STATE_ERROR);
  }
  try {
    vec.reserve(0);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_STATE_ERROR);
  }

  return 0;
}
