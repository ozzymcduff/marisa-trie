#include <cassert>
#include <cstdlib>
#include <ctime>

#include <marisa/vector.h>

int main() {
  std::srand(static_cast<unsigned int>(time(NULL)));

  std::vector<int> values;
  for (std::size_t i = 0; i < 1000; ++i) {
    values.push_back(std::rand());
  }

  marisa::Vector<int> vec;
  assert(vec.num_objs() == 0);
  assert(vec.capacity() == 0);
  assert(vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32));

  for (std::size_t i = 0; i < values.size(); ++i) {
    vec.push_back(values[i]);
    assert(vec[i] == values[i]);
  }

  assert(vec.num_objs() == values.size());
  assert(vec.size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));
  assert(!vec.empty());
  assert(vec.capacity() >= vec.num_objs());

  assert(vec.front() == values.front());
  assert(static_cast<const marisa::Vector<int> &>(vec).front()
      == values.front());
  assert(vec.back() == values.back());
  assert(static_cast<const marisa::Vector<int> &>(vec).back()
      == values.back());

  vec.shrink();
  assert(vec.num_objs() == values.size());
  assert(vec.capacity() == vec.num_objs());
  assert(!vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(vec[i] == values[i]);
    assert(static_cast<const marisa::Vector<int> &>(vec)[i] == values[i]);
  }

  assert(vec.save("vector-test.dat"));

  vec.clear();
  assert(vec.num_objs() == 0);
  assert(vec.capacity() == 0);
  assert(vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32));

  marisa::Mapper mapper;
  assert(vec.mmap(&mapper, "vector-test.dat"));
  assert(mapper.is_open());
  assert(vec.num_objs() == values.size());
  assert(vec.capacity() == 0);
  assert(!vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(static_cast<const marisa::Vector<int> &>(vec)[i] == values[i]);
  }

  vec.clear();
  assert(vec.load("vector-test.dat"));
  assert(vec.num_objs() == values.size());
  assert(vec.capacity() == vec.num_objs());
  assert(!vec.empty());
  assert(vec.size() == sizeof(marisa::UInt32)
      + ((sizeof(int) * values.size())));

  for (std::size_t i = 0; i < values.size(); ++i) {
    assert(vec[i] == values[i]);
    assert(static_cast<const marisa::Vector<int> &>(vec)[i] == values[i]);
  }

  return 0;
}
