#include <cassert>
#include <sstream>

#include <marisa/tail.h>

int main() {
  marisa::Tail tail;
  assert(tail.num_objs() == 0);
  assert(tail.empty());
  assert(tail.size() == sizeof(marisa::UInt32));

  std::vector<std::string> keys;

  assert(tail.build(keys, NULL));
  assert(tail.num_objs() == 0);
  assert(tail.empty());
  assert(tail.size() == sizeof(marisa::UInt32));

  keys.push_back("");

  std::vector<marisa::UInt32> offsets;
  assert(tail.build(keys, &offsets));

  assert(tail.num_objs() == 1);
  assert(!tail.empty());
  assert(tail.size() == (sizeof(marisa::UInt32) + 1));
  assert(offsets.size() == 1);
  assert(offsets[0] == 0);
  assert(keys[0] == reinterpret_cast<const char *>(tail[offsets[0]]));

  keys.clear();
  char binary_key[] = { 'N', 'P', '\0', 'T', 'r', 'i', 'e' };
  keys.push_back(std::string(binary_key, sizeof(binary_key)));
  assert(!tail.build(keys, &offsets));

  keys.clear();
  keys.push_back("abc");
  keys.push_back("bc");
  keys.push_back("abc");
  keys.push_back("c");
  keys.push_back("ABC");
  keys.push_back("AB");

  assert(tail.build(keys, NULL));
  assert(tail.num_objs() == 11);
  assert(!tail.empty());
  assert(tail.size() == (sizeof(marisa::UInt32) + 11));

  assert(tail.build(keys, &offsets));
  assert(tail.num_objs() == 11);
  assert(!tail.empty());
  assert(tail.size() == (sizeof(marisa::UInt32) + 11));
  assert(offsets.size() == keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(keys[i] == reinterpret_cast<const char *>(tail[offsets[i]]));
  }

  assert(tail.save("tail-test.dat"));

  tail.clear();
  assert(tail.num_objs() == 0);
  assert(tail.empty());
  assert(tail.size() == sizeof(marisa::UInt32));

  marisa::Mapper mapper;
  assert(tail.mmap(&mapper, "tail-test.dat"));

  assert(tail.num_objs() == 11);
  assert(!tail.empty());
  assert(tail.size() == sizeof(marisa::UInt32) + 11);
  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(keys[i] == reinterpret_cast<const char *>(tail[offsets[i]]));
  }

  tail.clear();
  assert(tail.load("tail-test.dat"));

  assert(tail.num_objs() == 11);
  assert(!tail.empty());
  assert(tail.size() == sizeof(marisa::UInt32) + 11);
  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(keys[i] == reinterpret_cast<const char *>(tail[offsets[i]]));
  }

  std::stringstream stream;
  assert(tail.write(&stream));

  tail.clear();

  assert(tail.read(&stream));

  assert(tail.num_objs() == 11);
  assert(!tail.empty());
  assert(tail.size() == (sizeof(marisa::UInt32) + 11));
  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(keys[i] == reinterpret_cast<const char *>(tail[offsets[i]]));
  }

  std::vector<const char *> keys2;

  assert(tail.build(keys2, NULL));
  assert(tail.num_objs() == 0);
  assert(tail.empty());
  assert(tail.size() == sizeof(marisa::UInt32));

  for (std::size_t i = 0; i < keys.size(); ++i) {
    keys2.push_back(keys[i].c_str());
  }
  std::vector<marisa::UInt32> offsets2;

  assert(tail.build(keys2, &offsets2));
  assert(tail.num_objs() == 11);
  assert(!tail.empty());
  assert(tail.size() == (sizeof(marisa::UInt32) + 11));
  assert(offsets2 == offsets);
  for (std::size_t i = 0; i < keys2.size(); ++i) {
    assert(keys[i] == reinterpret_cast<const char *>(tail[offsets2[i]]));
  }

  return 0;
}
