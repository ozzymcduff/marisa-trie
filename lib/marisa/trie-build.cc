#include <algorithm>
#include <functional>
#include <queue>

#include "./range.h"
#include "./trie.h"

namespace marisa {

void Trie::build(const std::vector<const char *> &keys,
    std::vector<UInt32> *key_ids, UInt32 max_num_tries,
    bool patricia, bool tail, bool freq_order) {
  build_from(keys, key_ids, Config(max_num_tries, patricia, tail, freq_order));
}

void Trie::build(const std::vector<char *> &keys,
    std::vector<UInt32> *key_ids, UInt32 max_num_tries,
    bool patricia, bool tail, bool freq_order) {
  build_from(keys, key_ids, Config(max_num_tries, patricia, tail, freq_order));
}

void Trie::build(const std::vector<std::string> &keys,
    std::vector<UInt32> *key_ids, UInt32 max_num_tries,
    bool patricia, bool tail, bool freq_order) {
  build_from(keys, key_ids, Config(max_num_tries, patricia, tail, freq_order));
}

void Trie::build(const std::vector<std::pair<const char *, double> > &keys,
    std::vector<UInt32> *key_ids, UInt32 max_num_tries,
    bool patricia, bool tail, bool weight_order) {
  build_from(keys, key_ids,
      Config(max_num_tries, patricia, tail, weight_order));
}

void Trie::build(const std::vector<std::pair<char *, double> > &keys,
    std::vector<UInt32> *key_ids, UInt32 max_num_tries,
    bool patricia, bool tail, bool weight_order) {
  build_from(keys, key_ids,
      Config(max_num_tries, patricia, tail, weight_order));
}

void Trie::build(const std::vector<std::pair<std::string, double> > &keys,
    std::vector<UInt32> *key_ids, UInt32 max_num_tries,
    bool patricia, bool tail, bool weight_order) {
  build_from(keys, key_ids,
      Config(max_num_tries, patricia, tail, weight_order));
}

template <typename T>
void Trie::build_from(const std::vector<T> &keys,
    std::vector<UInt32> *key_ids, const Config &config) {
  std::vector<Key<String> > temp_keys(keys.size());
  for (std::size_t i = 0; i < temp_keys.size(); ++i) {
    temp_keys[i].set_str(String(keys[i]));
    temp_keys[i].set_weight(1.0);
  }
  build_trie(temp_keys, key_ids, config);
}

template <typename T>
void Trie::build_from(const std::vector<std::pair<T, double> > &keys,
    std::vector<UInt32> *key_ids, const Config &config) {
  std::vector<Key<String> > temp_keys(keys.size());
  for (std::size_t i = 0; i < temp_keys.size(); ++i) {
    temp_keys[i].set_str(String(keys[i].first));
    temp_keys[i].set_weight(keys[i].second);
  }
  build_trie(temp_keys, key_ids, config);
}

void Trie::build_trie(std::vector<Key<String> > &keys,
    std::vector<UInt32> *key_ids, const Config &config) {
  Trie temp;
  std::vector<UInt32> terminals;
  temp.build_trie(keys, &terminals, 0, config);

  std::vector<std::pair<UInt32, UInt32> > pairs(terminals.size());
  for (std::size_t i = 0; i < pairs.size(); ++i) {
    pairs[i].first = terminals[i];
    pairs[i].second = i;
  }
  std::vector<UInt32>().swap(terminals);
  std::sort(pairs.begin(), pairs.end());

  UInt32 node = 0;
  for (std::size_t i = 0; i < pairs.size(); ++i) {
    while (node < pairs[i].first) {
      temp.terminal_flags_.push_back(false);
      ++node;
    }
    temp.terminal_flags_.push_back(true);
    ++node;
  }
  while (node < temp.labels_.num_objs()) {
    temp.terminal_flags_.push_back(false);
    ++node;
  }
  terminal_flags_.push_back(false);
  temp.terminal_flags_.build();

  if (key_ids != NULL) {
    std::vector<UInt32> temp_key_ids(pairs.size());
    for (std::size_t i = 0; i < temp_key_ids.size(); ++i) {
      temp_key_ids[pairs[i].second] = temp.node_to_key_id(pairs[i].first);
    }
    key_ids->swap(temp_key_ids);
  }
  temp.swap(this);
}

template <typename T>
void Trie::build_trie(std::vector<Key<T> > &keys,
    std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config) {
  build_cur(keys, terminals, trie_id, config);
  if (link_flags_.empty()) {
    return;
  }

  std::vector<UInt32> next_terminals;
  build_next(keys, &next_terminals, trie_id, config);

  for (std::size_t i = 0; i < next_terminals.size(); ++i) {
    labels_[link_flags_.select1(i)] = next_terminals[i] % 256;
    next_terminals[i] /= 256;
  }
  links_.build(next_terminals);
}

template <typename T>
void Trie::build_cur(std::vector<Key<T> > &keys,
    std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config) {
  num_keys_ = sort_keys(keys);

  louds_.push_back(true);
  louds_.push_back(false);
  labels_.push_back('\0');
  link_flags_.push_back(false);

  std::vector<Key<T> > rest_keys;
  std::queue<Range> queue;
  std::vector<WRange> wranges;
  queue.push(Range(0, keys.size(), 0));
  while (!queue.empty()) {
    UInt32 node = link_flags_.num_bits() - queue.size();
    Range range = queue.front();
    queue.pop();

    while ((range.begin() < range.end()) &&
        (keys[range.begin()].str().length() == range.pos())) {
      keys[range.begin()].set_terminal(node);
      range.set_begin(range.begin() + 1);
    }
    if (range.begin() == range.end()) {
      louds_.push_back(false);
      continue;
    }

    wranges.clear();
    double weight = keys[range.begin()].weight();
    for (UInt32 i = range.begin() + 1; i < range.end(); ++i) {
      if (keys[i - 1].str()[range.pos()] != keys[i].str()[range.pos()]) {
        wranges.push_back(WRange(range.begin(), i, range.pos(), weight));
        range.set_begin(i);
        weight = 0.0;
      }
      weight += keys[i].weight();
    }
    wranges.push_back(WRange(range, weight));
    if (config.weight_order()) {
      std::stable_sort(wranges.begin(), wranges.end(), std::greater<WRange>());
    }
    for (std::size_t i = 0; i < wranges.size(); ++i) {
      const WRange &wrange = wranges[i];
      UInt32 pos = wrange.pos() + 1;
      if (config.tail() || !config.is_last_trie(trie_id)) {
        while (pos < keys[wrange.begin()].str().length()) {
          UInt32 j;
          for (j = wrange.begin() + 1; j < wrange.end(); ++j) {
            if (keys[j - 1].str()[pos] != keys[j].str()[pos]) {
              break;
            }
          }
          if (j < wrange.end()) {
            break;
          }
          ++pos;
        }
      }
      if (!config.patricia() &&
          (pos != keys[wrange.end() - 1].str().length())) {
        pos = wrange.pos() + 1;
      }
      louds_.push_back(true);
      if (pos == wrange.pos() + 1) {
        labels_.push_back(keys[wrange.begin()].str()[wrange.pos()]);
        link_flags_.push_back(false);
      } else {
        labels_.push_back('\0');
        link_flags_.push_back(true);
        Key<T> rest_key;
        rest_key.set_str(keys[wrange.begin()].str().substr(
            wrange.pos(), pos - wrange.pos()));
        rest_key.set_weight(wrange.weight());
        rest_keys.push_back(rest_key);
      }
      wranges[i].set_pos(pos);
      queue.push(wranges[i].range());
    }
    louds_.push_back(false);
  }
  louds_.push_back(false);
  louds_.build();
  labels_.shrink();
  if (rest_keys.empty()) {
    BitVector().swap(&link_flags_);
  } else {
    link_flags_.build();
  }

  build_terminals(keys, terminals);
  keys.swap(rest_keys);
}

void Trie::build_next(std::vector<Key<String> > &keys,
    std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config) {
  if (config.is_last_trie(trie_id)) {
    std::vector<String> strs(keys.size());
    for (std::size_t i = 0; i < strs.size(); ++i) {
      strs[i] = keys[i].str();
    }
    if (tail_.build(strs, terminals)) {
      std::vector<Key<String> >().swap(keys);
      return;
    }
  }
  std::vector<Key<RString> > rkeys(keys.size());
  for (std::size_t i = 0; i < rkeys.size(); ++i) {
    rkeys[i].set_str(RString(keys[i].str()));
    rkeys[i].set_weight(keys[i].weight());
  }
  std::vector<Key<String> >().swap(keys);
  trie_.reset(new Trie);
  if (config.is_last_trie(trie_id)) {
    trie_->build_trie(rkeys, terminals, trie_id, Config(config.max_num_tries(),
        config.patricia(), false, config.weight_order()));
  } else {
    trie_->build_trie(rkeys, terminals, trie_id + 1, config);
  }
}

void Trie::build_next(std::vector<Key<RString> > &rkeys,
    std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config) {
  if (config.is_last_trie(trie_id)) {
    std::vector<String> strs(rkeys.size());
    for (std::size_t i = 0; i < strs.size(); ++i) {
      strs[i] = String(rkeys[i].str());
    }
    if (tail_.build(strs, terminals)) {
      std::vector<Key<RString> >().swap(rkeys);
      return;
    }
  }
  trie_.reset(new Trie);
  if (config.is_last_trie(trie_id)) {
    trie_->build_trie(rkeys, terminals, trie_id, Config(config.max_num_tries(),
        config.patricia(), false, config.weight_order()));
  } else {
    trie_->build_trie(rkeys, terminals, trie_id + 1, config);
  }
}

template <typename T>
UInt32 Trie::sort_keys(std::vector<Key<T> > &keys) const {
  if (keys.empty()) {
    return 0;
  }
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keys[i].set_id(i);
  }
  std::sort(keys.begin(), keys.end());
  UInt32 count = 1;
  for (std::size_t i = 1; i < keys.size(); ++i) {
    if (keys[i - 1].str() != keys[i].str()) {
      ++count;
    }
  }
  return count;
}

template <typename T>
void Trie::build_terminals(const std::vector<Key<T> > &keys,
    std::vector<UInt32> *terminals) const {
  std::vector<UInt32> temp_terminals(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    temp_terminals[keys[i].id()] = keys[i].terminal();
  }
  terminals->swap(temp_terminals);
}

}  // namespace marisa
