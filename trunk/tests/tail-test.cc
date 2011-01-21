#include <cassert>
#include <sstream>

#include <marisa/tail.h>

int main() {
  marisa::Tail tail;
  assert(tail.size() == 0);
  assert(tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32));

  marisa::Vector<marisa::String> keys;

  tail.build(keys, NULL, MARISA_BINARY_TAIL);
  assert(tail.size() == 0);
  assert(tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32));

  tail.build(keys, NULL, MARISA_TEXT_TAIL);
  assert(tail.size() == 0);
  assert(tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32));

  keys.push_back(marisa::String(""));

  marisa::Vector<marisa::UInt32> offsets;
  tail.build(keys, &offsets, MARISA_BINARY_TAIL);

  assert(tail.size() == 1);
  assert(tail.mode() == MARISA_BINARY_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));
  assert(offsets.size() == keys.size() + 1);
  assert(offsets[0] == 1);
  assert(offsets[1] == tail.size());

  tail.build(keys, &offsets, MARISA_TEXT_TAIL);

  assert(tail.size() == 2);
  assert(tail.mode() == MARISA_TEXT_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));
  assert(offsets.size() == keys.size());
  assert(offsets[0] == 1);
  assert(*tail[offsets[0]] == '\0');

  const char binary_key[] = { 'N', 'P', '\0', 'T', 'r', 'i', 'e' };
  keys[0] = marisa::String(binary_key, sizeof(binary_key));
  try {
    tail.build(keys, &offsets, MARISA_TEXT_TAIL);
    assert(false);
  } catch (const marisa::Exception &ex) {
    assert(ex.status() == MARISA_PARAM_ERROR);
  }

  tail.build(keys, &offsets, MARISA_BINARY_TAIL);

  assert(tail.size() == sizeof(binary_key) + 1);
  assert(tail.mode() == MARISA_BINARY_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));
  assert(offsets.size() == keys.size() + 1);
  assert(offsets[0] == 1);
  assert(offsets[1] == tail.size());

  keys.clear();
  keys.push_back(marisa::String("abc"));
  keys.push_back(marisa::String("bc"));
  keys.push_back(marisa::String("abc"));
  keys.push_back(marisa::String("c"));
  keys.push_back(marisa::String("ABC"));
  keys.push_back(marisa::String("AB"));

  tail.build(keys, NULL, MARISA_BINARY_TAIL);
  assert(tail.size() == 15);
  assert(tail.mode() == MARISA_BINARY_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));

  tail.build(keys, &offsets, MARISA_BINARY_TAIL);
  assert(tail.size() == 15);
  assert(tail.mode() == MARISA_BINARY_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));
  assert(offsets.size() == 7);
  for (marisa::UInt32 i = 0; i < keys.size(); ++i) {
    assert(marisa::String(reinterpret_cast<const char *>(tail[offsets[i]]),
        offsets[i + 1] - offsets[i]) == keys[i]);
  }

  tail.build(keys, NULL, MARISA_TEXT_TAIL);
  assert(tail.size() == 12);
  assert(tail.mode() == MARISA_TEXT_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));

  tail.build(keys, &offsets, MARISA_TEXT_TAIL);
  assert(tail.size() == 12);
  assert(tail.mode() == MARISA_TEXT_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == (sizeof(marisa::UInt32) + tail.size()));
  assert(offsets.size() == keys.size());
  for (marisa::UInt32 i = 0; i < keys.size(); ++i) {
    assert(marisa::String(reinterpret_cast<const char *>(
        tail[offsets[i]])) == keys[i]);
  }

  tail.save("tail-test.dat");

  tail.clear();
  assert(tail.size() == 0);
  assert(tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32));

  marisa::Mapper mapper;
  tail.mmap(&mapper, "tail-test.dat");

  assert(tail.size() == 12);
  assert(tail.mode() == MARISA_TEXT_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32) + tail.size());
  for (marisa::UInt32 i = 0; i < keys.size(); ++i) {
    assert(marisa::String(reinterpret_cast<const char *>(
        tail[offsets[i]])) == keys[i]);
  }

  tail.clear();
  tail.load("tail-test.dat");

  assert(tail.size() == 12);
  assert(tail.mode() == MARISA_TEXT_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32) + tail.size());
  for (marisa::UInt32 i = 0; i < keys.size(); ++i) {
    assert(marisa::String(reinterpret_cast<const char *>(
        tail[offsets[i]])) == keys[i]);
  }

  std::stringstream stream;
  tail.write(stream);

  tail.clear();
  tail.read(stream);

  assert(tail.size() == 12);
  assert(tail.mode() == MARISA_TEXT_TAIL);
  assert(!tail.empty());
  assert(tail.total_size() == sizeof(marisa::UInt32) + tail.size());
  for (marisa::UInt32 i = 0; i < keys.size(); ++i) {
    assert(marisa::String(reinterpret_cast<const char *>(
        tail[offsets[i]])) == keys[i]);
  }

  return 0;
}
