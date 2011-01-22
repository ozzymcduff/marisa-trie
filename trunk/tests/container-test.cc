#include <cassert>
#include <cstdlib>
#include <ctime>

#include <marisa/container.h>

int main() {
  std::srand((unsigned int)std::time(NULL));

  int values[10];
  marisa::Container<int *> values_container(values);
  for (int i = 0; i < 10; ++i) {
    int value = std::rand();
    values_container.insert(i, value);
    assert(values[i] == value);
  }

  std::vector<int> vec;
  marisa::Container<std::vector<int> *> vec_container(&vec);
  for (int i = 0; i < 10; ++i) {
    int value = std::rand();
    vec_container.insert(i, value);
    assert(vec.back() == value);
    assert(vec[i] == value);
  }
  assert(vec.size() == 10);

  marisa::Container<std::vector<int> *> vec_container2(&vec);
  for (int i = 0; i < 10; ++i) {
    int value = std::rand();
    vec_container.insert(i, value);
    assert(vec.back() == value);
    assert(vec[i + 10] == value);
  }
  assert(vec.size() == 20);

  return 0;
}
