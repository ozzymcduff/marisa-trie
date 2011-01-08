#include <cassert>
#include <sstream>

#include <marisa/trie.h>

namespace {

class FindCallback {
 public:
  FindCallback(std::vector<marisa::UInt32> *key_ids,
      std::vector<std::size_t> *key_lengths)
      : key_ids_(key_ids), key_lengths_(key_lengths) {}
  FindCallback(const FindCallback &callback)
      : key_ids_(callback.key_ids_), key_lengths_(callback.key_lengths_) {}

  bool operator()(marisa::UInt32 key_id, std::size_t key_length) const {
    key_ids_->push_back(key_id);
    key_lengths_->push_back(key_length);
    return true;
  }

 private:
  std::vector<marisa::UInt32> *key_ids_;
  std::vector<std::size_t> *key_lengths_;

  // Disallows assignment.
  FindCallback &operator=(const FindCallback &);
};

class PredictCallback {
 public:
  PredictCallback(std::vector<marisa::UInt32> *key_ids,
      std::vector<std::string> *keys)
      : key_ids_(key_ids), keys_(keys) {}
  PredictCallback(const PredictCallback &callback)
      : key_ids_(callback.key_ids_), keys_(callback.keys_) {}

  bool operator()(marisa::UInt32 key_id, const std::string &key) const {
    key_ids_->push_back(key_id);
    keys_->push_back(key);
    return true;
  }

 private:
  std::vector<marisa::UInt32> *key_ids_;
  std::vector<std::string> *keys_;

  // Disallows assignment.
  PredictCallback &operator=(const PredictCallback &);
};

void TestTrie() {
  marisa::Trie trie;
  assert(trie.num_keys() == 0);
  assert(trie.num_tries() == 0);
  assert(trie.num_nodes() == 0);
  assert(trie.size() == (sizeof(marisa::UInt32) * 22));

  std::vector<std::string> keys;
  trie.build(keys);
  assert(trie.num_keys() == 0);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 1);

  keys.push_back("apple");
  keys.push_back("and");
  keys.push_back("Bad");
  keys.push_back("apple");
  keys.push_back("app");

  std::vector<marisa::UInt32> key_ids;
  trie.build(keys, &key_ids, 0, false, false, false);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 11);

  assert(key_ids.size() == 5);
  assert(key_ids[0] == 3);
  assert(key_ids[1] == 1);
  assert(key_ids[2] == 0);
  assert(key_ids[3] == 3);
  assert(key_ids[4] == 2);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.clear();
  assert(trie.num_keys() == 0);
  assert(trie.num_tries() == 0);
  assert(trie.num_nodes() == 0);
  assert(trie.size() == (sizeof(marisa::UInt32) * 22));

  trie.build(keys, &key_ids, 1, false, false, true);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 11);

  assert(key_ids.size() == 5);
  assert(key_ids[0] == 3);
  assert(key_ids[1] == 1);
  assert(key_ids[2] == 2);
  assert(key_ids[3] == 3);
  assert(key_ids[4] == 0);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  assert(trie["appl"] == trie.notfound());
  assert(trie["Apple"] == trie.notfound());
  assert(trie["applex"] == trie.notfound());

  assert(trie.find_first("ap") == trie.notfound());
  assert(trie.find_first("applex") == trie["app"]);

  assert(trie.find_last("ap") == trie.notfound());
  assert(trie.find_last("applex") == trie["apple"]);

  std::vector<marisa::UInt32> ids;
  assert(trie.find("ap", &ids) == 0);
  assert(trie.find("applex", &ids) == 2);
  assert(ids.size() == 2);
  assert(ids[0] == trie["app"]);
  assert(ids[1] == trie["apple"]);

  std::vector<std::size_t> lengths;
  assert(trie.find("Baddie", &ids, &lengths) == 1);
  assert(ids.size() == 3);
  assert(ids[0] == trie["app"]);
  assert(ids[1] == trie["apple"]);
  assert(ids[2] == trie["Bad"]);
  assert(lengths.size() == 1);
  assert(lengths[0] == 3);

  assert(trie.find_callback("anderson", FindCallback(&ids, &lengths)) == 1);
  assert(ids.size() == 4);
  assert(ids[3] == trie["and"]);
  assert(lengths.size() == 2);
  assert(lengths[1] == 3);

  ids.clear();
  assert(trie.predict("a", &ids) == 3);
  assert(ids.size() == 3);
  assert(ids[0] == trie["app"]);
  assert(ids[1] == trie["and"]);
  assert(ids[2] == trie["apple"]);

  std::vector<std::string> strs;
  assert(trie.predict("a", &ids, &strs) == 3);
  assert(ids.size() == 6);
  assert(ids[3] == trie["app"]);
  assert(ids[4] == trie["apple"]);
  assert(ids[5] == trie["and"]);
  assert(strs[0] == "app");
  assert(strs[1] == "apple");
  assert(strs[2] == "and");
}

void TestPrefixTrie() {
  std::vector<std::string> keys;
  keys.push_back("after");
  keys.push_back("bar");
  keys.push_back("car");
  keys.push_back("caster");

  marisa::Trie trie;
  std::vector<marisa::UInt32> key_ids;
  trie.build(keys, &key_ids, 1, false, true, false);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 7);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 2, false, false, true);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 16);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 2, false, true, false);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 14);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  assert(trie.save("trie-test.dat"));

  trie.clear();
  marisa::Mapper mapper;
  assert(trie.mmap(&mapper, "trie-test.dat"));
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 14);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  std::stringstream stream;
  assert(trie.write(&stream));

  trie.clear();
  assert(trie.read(&stream));
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 14);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 3, false, false, true);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 3);
  assert(trie.num_nodes() == 19);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  assert(trie["ca"] == trie.notfound());
  assert(trie["card"] == trie.notfound());

  std::size_t length = 0;
  assert(trie.find_first("ca") == trie.notfound());
  assert(trie.find_first("car") == trie["car"]);
  assert(trie.find_first("card", &length) == trie["car"]);
  assert(length == 3);

  assert(trie.find_last("afte") == trie.notfound());
  assert(trie.find_last("after") == trie["after"]);
  assert(trie.find_last("afternoon", &length) == trie["after"]);
  assert(length == 5);

  std::vector<marisa::UInt32> ids;
  assert(trie.predict("ca", &ids) == 2);
  assert(ids.size() == 2);
  assert(ids[0] == trie["car"]);
  assert(ids[1] == trie["caster"]);

  std::vector<std::string> strs;
  assert(trie.predict_callback("", PredictCallback(&ids, &strs)) == 4);
  assert(ids.size() == 6);
  assert(ids[2] == trie["car"]);
  assert(ids[3] == trie["caster"]);
  assert(ids[4] == trie["after"]);
  assert(ids[5] == trie["bar"]);
  assert(strs[0] == "car");
  assert(strs[1] == "caster");
  assert(strs[2] == "after");
  assert(strs[3] == "bar");
}

void TestPatriciaTrie() {
  std::vector<std::string> keys;
  keys.push_back("bach");
  keys.push_back("bet");
  keys.push_back("chat");
  keys.push_back("check");
  keys.push_back("check");

  marisa::Trie trie;
  std::vector<marisa::UInt32> key_ids;
  trie.build(keys, &key_ids, 1, true, true);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 7);

  assert(key_ids.size() == 5);
  assert(key_ids[0] == 2);
  assert(key_ids[1] == 3);
  assert(key_ids[2] == 1);
  assert(key_ids[3] == 0);
  assert(key_ids[4] == 0);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 2, true, false);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 17);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 2, true, true);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 14);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 3, true, false);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 3);
  assert(trie.num_nodes() == 20);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  std::stringstream stream;
  trie.write(&stream);
  trie.clear();

  trie.read(&stream);
  assert(trie.num_keys() == 4);
  assert(trie.num_tries() == 3);
  assert(trie.num_nodes() == 20);

  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }
}

void TestEmptyString() {
  std::vector<std::string> keys;
  keys.push_back("");

  marisa::Trie trie;
  std::vector<marisa::UInt32> key_ids;
  trie.build(keys, &key_ids);
  assert(trie.num_keys() == 1);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 1);

  assert(key_ids.size() == 1);
  assert(key_ids[0] == 0);

  assert(trie[""] == 0);
  assert(trie[0U] == "");

  assert(trie["x"] == trie.notfound());
  assert(trie.find_first("") == 0);
  assert(trie.find_first("x") == 0);
  assert(trie.find_last("") == 0);
  assert(trie.find_last("x") == 0);

  std::vector<marisa::UInt32> ids;
  assert(trie.find("xyz", &ids) == 1);
  assert(ids.size() == 1);
  assert(ids[0] == trie[""]);

  std::vector<std::size_t> lengths;
  assert(trie.find("xyz", &ids, &lengths) == 1);
  assert(ids.size() == 2);
  assert(ids[0] == trie[""]);
  assert(ids[1] == trie[""]);
  assert(lengths.size() == 1);
  assert(lengths[0] == 0);

  assert(trie.find_callback("xyz", FindCallback(&ids, &lengths)) == 1);
  assert(ids.size() == 3);
  assert(ids[2] == trie[""]);
  assert(lengths.size() == 2);
  assert(lengths[1] == 0);

  ids.clear();
  assert(trie.predict("xyz", &ids) == 0);

  assert(trie.predict("", &ids) == 1);
  assert(ids.size() == 1);
  assert(ids[0] == trie[""]);

  std::vector<std::string> strs;
  assert(trie.predict("", &ids, &strs) == 1);
  assert(ids.size() == 2);
  assert(ids[1] == trie[""]);
  assert(strs[0] == "");
}

void TestBinaryKey() {
  std::string binary_key = "NP";
  binary_key += '\0';
  binary_key += "Trie";

  std::vector<std::string> keys;
  keys.push_back(binary_key);

  marisa::Trie trie;
  std::vector<marisa::UInt32> key_ids;
  trie.build(keys, &key_ids, 1, false, false);
  assert(trie.num_keys() == 1);
  assert(trie.num_tries() == 1);
  assert(trie.num_nodes() == 8);

  assert(key_ids.size() == 1);
  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }

  trie.build(keys, &key_ids, 1, false, true);
  assert(trie.num_keys() == 1);
  assert(trie.num_tries() == 2);
  assert(trie.num_nodes() == 10);

  assert(key_ids.size() == 1);
  for (std::size_t i = 0; i < keys.size(); ++i) {
    assert(key_ids[i] == i);
    assert(trie[keys[i]] == key_ids[i]);
    assert(trie[key_ids[i]] == keys[i]);
  }
}

}  // namespace

int main() {
  TestTrie();
  TestPrefixTrie();
  TestPatriciaTrie();
  TestEmptyString();
  TestBinaryKey();

  return 0;
}
