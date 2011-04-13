#include <algorithm>
#include <queue>

#include "../algorithm.h"
#include "header.h"
#include "range.h"
#include "state.h"
#include "louds-trie.h"

namespace marisa {
namespace grimoire {
namespace trie {

LoudsTrie::LoudsTrie()
    : louds_(), terminal_flags_(), labels_(), link_flags_(), links_(),
      tail_(), next_trie_(), num_l1_nodes_(0), config_(), mapper_() {}

LoudsTrie::~LoudsTrie() {}

void LoudsTrie::build(Keyset &keyset, int flags) {
  Config config;
  config.parse(flags);

  LoudsTrie temp;
  temp.build_(keyset, config);
  swap(temp);
}

void LoudsTrie::map(Mapper &mapper) {
  Header().map(mapper);

  LoudsTrie temp;
  temp.map_(mapper);
  swap(temp);
}

void LoudsTrie::read(Reader &reader) {
  Header().read(reader);

  LoudsTrie temp;
  temp.read_(reader);
  swap(temp);
}

void LoudsTrie::write(Writer &writer) const {
  Header().write(writer);

  write_(writer);
}

bool LoudsTrie::lookup(Agent &agent) const {
  MARISA_DEBUG_IF(!agent.has_state(), MARISA_STATE_ERROR);

  State &state = agent.state();
  state.lookup_init();
  while (state.query_pos() < agent.query().length()) {
    if (!find_child(agent)) {
      return false;
    }
  }
  if (!terminal_flags_[state.node_id()]) {
    return false;
  }
  agent.set_key(agent.query().ptr(), agent.query().length());
  agent.set_key(terminal_flags_.rank1(state.node_id()));
  return true;
}

void LoudsTrie::reverse_lookup(Agent &agent) const {
  MARISA_DEBUG_IF(!agent.has_state(), MARISA_STATE_ERROR);
  MARISA_THROW_IF(agent.query().key_id() >= size(), MARISA_BOUND_ERROR);

  State &state = agent.state();
  state.reverse_lookup_init();

  state.set_node_id(terminal_flags_.select1(agent.query().key_id()));
  if (state.node_id() == 0) {
    agent.set_key(state.key_buf().begin(), state.key_buf().size());
    agent.set_key(agent.query().key_id());
    return;
  }
  for ( ; ; ) {
    if (link_flags_[state.node_id()]) {
      const std::size_t prev_key_pos = state.key_buf().size();
      const std::size_t link = labels_[state.node_id()]
          + (links_[link_flags_.rank1(state.node_id())] * 256);
      if (next_trie_.get() != NULL) {
        next_trie_->restore(agent, link);
      } else {
        tail_.restore(agent, link);
      }
      std::reverse(state.key_buf().begin() + prev_key_pos,
          state.key_buf().end());
    } else {
      state.key_buf().push_back((char)labels_[state.node_id()]);
    }

    if (state.node_id() <= num_l1_nodes_) {
      std::reverse(state.key_buf().begin(), state.key_buf().end());
      agent.set_key(state.key_buf().begin(), state.key_buf().size());
      agent.set_key(agent.query().key_id());
      return;
    }
    state.set_node_id(louds_.select1(state.node_id()) - state.node_id() - 1);
  }
}

bool LoudsTrie::common_prefix_search(Agent &agent) const {
  MARISA_DEBUG_IF(!agent.has_state(), MARISA_STATE_ERROR);

  State &state = agent.state();
  if (state.status_code() == MARISA_END_OF_COMMON_PREFIX_SEARCH) {
    return false;
  }

  if (state.status_code() != MARISA_READY_TO_COMMON_PREFIX_SEARCH) {
    state.common_prefix_search_init();
    if (terminal_flags_[state.node_id()]) {
      agent.set_key(agent.query().ptr(), state.query_pos());
      agent.set_key(terminal_flags_.rank1(state.node_id()));
      return true;
    }
  }

  while (state.query_pos() < agent.query().length()) {
    if (!find_child(agent)) {
      state.set_status_code(MARISA_END_OF_COMMON_PREFIX_SEARCH);
      return false;
    } else if (terminal_flags_[state.node_id()]) {
      agent.set_key(agent.query().ptr(), state.query_pos());
      agent.set_key(terminal_flags_.rank1(state.node_id()));
      return true;
    }
  }
  state.set_status_code(MARISA_END_OF_COMMON_PREFIX_SEARCH);
  return false;
}

bool LoudsTrie::predictive_search(Agent &agent) const {
  MARISA_DEBUG_IF(!agent.has_state(), MARISA_STATE_ERROR);

  State &state = agent.state();
  if (state.status_code() == MARISA_END_OF_PREDICTIVE_SEARCH) {
    return false;
  }

  if (state.status_code() != MARISA_READY_TO_PREDICTIVE_SEARCH) {
    state.predictive_search_init();
    while (state.query_pos() < agent.query().length()) {
      if (!predictive_find_child(agent)) {
        state.set_status_code(MARISA_END_OF_PREDICTIVE_SEARCH);
        return false;
      }
    }

    History history;
    history.set_node_id(state.node_id());
    history.set_key_pos(state.key_buf().size());
    state.history().push_back(history);
    state.set_history_pos(1);

    if (terminal_flags_[state.node_id()]) {
      agent.set_key(state.key_buf().begin(), state.key_buf().size());
      agent.set_key(terminal_flags_.rank1(state.node_id()));
      return true;
    }
  }

  for ( ; ; ) {
    if (state.history_pos() == state.history().size()) {
      const History &current = state.history().back();
      History next;
      next.set_louds_pos(louds_.select0(current.node_id()) + 1);
      next.set_node_id(next.louds_pos() - current.node_id() - 1);
      state.history().push_back(next);
    }

    History &next = state.history()[state.history_pos()];
    const bool link_flag = louds_[next.louds_pos()];
    next.set_louds_pos(next.louds_pos() + 1);
    if (link_flag) {
      state.set_history_pos(state.history_pos() + 1);
      if (link_flags_[next.node_id()]) {
        if (next.link_id() == MARISA_INVALID_LINK_ID) {
          next.set_link_id(link_flags_.rank1(next.node_id()));
        } else {
          next.set_link_id(next.link_id() + 1);
        }
        const std::size_t link = labels_[next.node_id()]
            + (links_[next.link_id()] * 256);
        if (next_trie_.get() != NULL) {
          next_trie_->restore(agent, link);
        } else {
          tail_.restore(agent, link);
        }
      } else {
        state.key_buf().push_back((char)labels_[next.node_id()]);
      }
      next.set_key_pos(state.key_buf().size());

      if (terminal_flags_[next.node_id()]) {
        if (next.key_id() == MARISA_INVALID_KEY_ID) {
          next.set_key_id(terminal_flags_.rank1(next.node_id()));
        } else {
          next.set_key_id(next.key_id() + 1);
        }
        agent.set_key(state.key_buf().begin(), state.key_buf().size());
        agent.set_key(next.key_id());
        return true;
      }
    } else if (state.history_pos() != 1) {
      History &current = state.history()[state.history_pos() - 1];
      current.set_node_id(current.node_id() + 1);
      const History &prev =
          state.history()[state.history_pos() - 2];
      state.key_buf().resize(prev.key_pos());
      state.set_history_pos(state.history_pos() - 1);
    } else {
      state.set_status_code(MARISA_END_OF_PREDICTIVE_SEARCH);
      return false;
    }
  }
}

std::size_t LoudsTrie::total_size() const {
  return louds_.total_size() + terminal_flags_.total_size()
      + labels_.total_size() + link_flags_.total_size()
      + links_.total_size() + tail_.total_size()
      + ((next_trie_.get() != NULL) ? next_trie_->total_size() : 0);
}

std::size_t LoudsTrie::io_size() const {
  return Header().io_size() + sizeof(UInt64) + louds_.io_size()
      + terminal_flags_.io_size() + labels_.io_size() + link_flags_.io_size()
      + links_.io_size() + tail_.io_size()
      + ((next_trie_.get() != NULL) ?
          (next_trie_->io_size() - Header().io_size()) : 0);
}

void LoudsTrie::clear() {
  LoudsTrie().swap(*this);
}

void LoudsTrie::swap(LoudsTrie &rhs) {
  louds_.swap(rhs.louds_);
  terminal_flags_.swap(rhs.terminal_flags_);
  labels_.swap(rhs.labels_);
  link_flags_.swap(rhs.link_flags_);
  links_.swap(rhs.links_);
  tail_.swap(rhs.tail_);
  next_trie_.swap(rhs.next_trie_);
  marisa::swap(num_l1_nodes_, rhs.num_l1_nodes_);
  config_.swap(rhs.config_);
  mapper_.swap(rhs.mapper_);
}

void LoudsTrie::build_(Keyset &keyset, const Config &config) {
  Vector<Key> keys;
  keys.resize(keyset.size());
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    keys[i].set_str(keyset[i].ptr(), keyset[i].length());
    keys[i].set_weight(keyset[i].weight());
  }

  Vector<UInt32> terminals;
  build_trie(keys, &terminals, config, 1);

  typedef std::pair<UInt32, UInt32> TerminalIdPair;

  Vector<TerminalIdPair> pairs;
  pairs.resize(terminals.size());
  for (std::size_t i = 0; i < pairs.size(); ++i) {
    pairs[i].first = terminals[i];
    pairs[i].second = (UInt32)i;
  }
  terminals.clear();
  std::sort(pairs.begin(), pairs.end());

  std::size_t node_id = 0;
  for (std::size_t i = 0; i < pairs.size(); ++i) {
    while (node_id < pairs[i].first) {
      terminal_flags_.push_back(false);
      ++node_id;
    }
    if (node_id == pairs[i].first) {
      terminal_flags_.push_back(true);
      ++node_id;
    }
  }
  while (node_id < labels_.size()) {
    terminal_flags_.push_back(false);
    ++node_id;
  }
  terminal_flags_.push_back(false);
  terminal_flags_.build(false, true);

  for (std::size_t i = 0; i < keyset.size(); ++i) {
    keyset[pairs[i].second].set_id(terminal_flags_.rank1(pairs[i].first));
  }
}

template <typename T>
void LoudsTrie::build_trie(Vector<T> &keys,
    Vector<UInt32> *terminals, const Config &config, std::size_t trie_id) {
  build_current_trie(keys, terminals, config, trie_id);

  Vector<UInt32> next_terminals;
  if (!keys.empty()) {
    build_next_trie(keys, &next_terminals, config, trie_id);
  }

  if (next_trie_.get() != NULL) {
    config_.parse((next_trie_->num_tries() + 1) |
        next_trie_->tail_mode() | next_trie_->node_order());
  } else {
    config_.parse(1 | tail_.mode() | config.node_order());
  }

  link_flags_.build(false, false);
  std::size_t node_id = 0;
  for (std::size_t i = 0; i < next_terminals.size(); ++i) {
    while (!link_flags_[node_id]) {
      ++node_id;
    }
    labels_[node_id] = (UInt8)(next_terminals[i] % 256);
    next_terminals[i] /= 256;
    ++node_id;
  }
  links_.build(next_terminals);
}

template <typename T>
void LoudsTrie::build_current_trie(Vector<T> &keys,
    Vector<UInt32> *terminals, const Config &config,
    std::size_t trie_id) try {
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keys[i].set_id(i);
  }
  Algorithm().sort(keys.begin(), keys.end());

  louds_.push_back(true);
  louds_.push_back(false);
  labels_.push_back('\0');
  link_flags_.push_back(false);

  Vector<T> next_keys;
  std::queue<Range> queue;
  Vector<WeightedRange> w_ranges;

  queue.push(make_range(0, keys.size(), 0));
  while (!queue.empty()) {
    const std::size_t node_id = link_flags_.size() - queue.size();

    Range range = queue.front();
    queue.pop();

    while ((range.begin() < range.end()) &&
        (keys[range.begin()].length() == range.key_pos())) {
      keys[range.begin()].set_terminal(node_id);
      range.set_begin(range.begin() + 1);
    }

    if (range.begin() == range.end()) {
      louds_.push_back(false);
      continue;
    }

    w_ranges.clear();
    double weight = keys[range.begin()].weight();
    for (std::size_t i = range.begin() + 1; i < range.end(); ++i) {
      if (keys[i - 1][range.key_pos()] != keys[i][range.key_pos()]) {
        w_ranges.push_back(make_weighted_range(
            range.begin(), i, range.key_pos(), (float)weight));
        range.set_begin(i);
        weight = 0.0;
      }
      weight += keys[i].weight();
    }
    w_ranges.push_back(make_weighted_range(
        range.begin(), range.end(), range.key_pos(), (float)weight));
    if (config.node_order() == MARISA_WEIGHT_ORDER) {
      std::stable_sort(w_ranges.begin(), w_ranges.end(),
          std::greater<WeightedRange>());
    }

    if (node_id == 0) {
      num_l1_nodes_ = w_ranges.size();
    }

    for (std::size_t i = 0; i < w_ranges.size(); ++i) {
      WeightedRange &w_range = w_ranges[i];
      std::size_t key_pos = w_range.key_pos() + 1;
      while (key_pos < keys[w_range.begin()].length()) {
        std::size_t j;
        for (j = w_range.begin() + 1; j < w_range.end(); ++j) {
          if (keys[j - 1][key_pos] != keys[j][key_pos]) {
            break;
          }
        }
        if (j < w_range.end()) {
          break;
        }
        ++key_pos;
      }

      if (key_pos == w_range.key_pos() + 1) {
        labels_.push_back(keys[w_range.begin()][w_range.key_pos()]);
        link_flags_.push_back(false);
      } else {
        labels_.push_back('\0');
        link_flags_.push_back(true);
        T next_key;
        next_key.set_str(keys[w_range.begin()].ptr(),
            keys[w_range.begin()].length());
        next_key.substr(w_range.key_pos(), key_pos - w_range.key_pos());
        next_key.set_weight(w_range.weight());
        next_keys.push_back(next_key);
      }
      w_range.set_key_pos(key_pos);
      queue.push(w_range.range());
      louds_.push_back(true);
    }
    louds_.push_back(false);
  }

  louds_.push_back(false);
  louds_.build(trie_id == 1, true);
  labels_.shrink();

  build_terminals(keys, terminals);
  keys.swap(next_keys);
} catch (const std::bad_alloc &) {
  MARISA_THROW(MARISA_MEMORY_ERROR, "std::bad_alloc");
}

void LoudsTrie::build_next_trie(Vector<Key> &keys,
    Vector<UInt32> *terminals, const Config &config, std::size_t trie_id) {
  if (trie_id == config.num_tries()) {
    Vector<Entry> entries;
    entries.resize(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
      entries[i].set_str(keys[i].ptr(), keys[i].length());
    }
    tail_.build(entries, terminals, config.tail_mode());
    return;
  }
  Vector<ReverseKey> reverse_keys;
  reverse_keys.resize(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    reverse_keys[i].set_str(keys[i].ptr(), keys[i].length());
    reverse_keys[i].set_weight(keys[i].weight());
  }
  keys.clear();
  next_trie_.reset(new (std::nothrow) LoudsTrie);
  MARISA_THROW_IF(next_trie_.get() == NULL, MARISA_MEMORY_ERROR);
  next_trie_->build_trie(reverse_keys, terminals, config, trie_id + 1);
}

void LoudsTrie::build_next_trie(Vector<ReverseKey> &keys,
    Vector<UInt32> *terminals, const Config &config, std::size_t trie_id) {
  if (trie_id == config.num_tries()) {
    Vector<Entry> entries;
    entries.resize(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
      entries[i].set_str(keys[i].ptr(), keys[i].length());
    }
    tail_.build(entries, terminals, config.tail_mode());
    return;
  }
  next_trie_.reset(new (std::nothrow) LoudsTrie);
  MARISA_THROW_IF(next_trie_.get() == NULL, MARISA_MEMORY_ERROR);
  next_trie_->build_trie(keys, terminals, config, trie_id + 1);
}

template <typename T>
void LoudsTrie::build_terminals(const Vector<T> &keys,
    Vector<UInt32> *terminals) const {
  Vector<UInt32> temp;
  temp.resize(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    temp[keys[i].id()] = (UInt32)keys[i].terminal();
  }
  terminals->swap(temp);
}

void LoudsTrie::map_(Mapper &mapper) {
  {
    UInt32 temp_num_l1_nodes;
    mapper.map(&temp_num_l1_nodes);
    num_l1_nodes_ = temp_num_l1_nodes;
  }
  {
    UInt32 temp_config_flags;
    mapper.map(&temp_config_flags);
    config_.parse((int)temp_config_flags);
  }
  louds_.map(mapper);
  terminal_flags_.map(mapper);
  labels_.map(mapper);
  link_flags_.map(mapper);
  links_.map(mapper);
  tail_.map(mapper);
  if ((link_flags_.num_1s() != 0) && tail_.empty()) {
    next_trie_.reset(new (std::nothrow) LoudsTrie);
    MARISA_THROW_IF(next_trie_.get() == NULL, MARISA_MEMORY_ERROR);
    next_trie_->map_(mapper);
  }
  mapper_.swap(mapper);
}

void LoudsTrie::read_(Reader &reader) {
  {
    UInt32 temp_num_l1_nodes;
    reader.read(&temp_num_l1_nodes);
    num_l1_nodes_ = temp_num_l1_nodes;
  }
  {
    UInt32 temp_config_flags;
    reader.read(&temp_config_flags);
    config_.parse((int)temp_config_flags);
  }
  louds_.read(reader);
  terminal_flags_.read(reader);
  labels_.read(reader);
  link_flags_.read(reader);
  links_.read(reader);
  tail_.read(reader);
  if ((link_flags_.num_1s() != 0) && tail_.empty()) {
    next_trie_.reset(new (std::nothrow) LoudsTrie);
    MARISA_THROW_IF(next_trie_.get() == NULL, MARISA_MEMORY_ERROR);
    next_trie_->read_(reader);
  }
}

void LoudsTrie::write_(Writer &writer) const {
  writer.write((UInt32)num_l1_nodes_);
  writer.write((UInt32)config_.flags());
  louds_.write(writer);
  terminal_flags_.write(writer);
  labels_.write(writer);
  link_flags_.write(writer);
  links_.write(writer);
  tail_.write(writer);
  if (next_trie_.get() != NULL) {
    next_trie_->write_(writer);
  }
}

bool LoudsTrie::find_child(Agent &agent) const {
  MARISA_DEBUG_IF(agent.state().query_pos() >= agent.query().length(),
      MARISA_BOUND_ERROR);

  State &state = agent.state();
  std::size_t louds_pos = louds_.select0(state.node_id()) + 1;
  if (!louds_[louds_pos]) {
    return false;
  }
  state.set_node_id(louds_pos - state.node_id() - 1);
  std::size_t link_id = MARISA_INVALID_LINK_ID;
  do {
    if (link_flags_[state.node_id()]) {
      if (link_id == MARISA_INVALID_LINK_ID) {
        link_id = link_flags_.rank1(state.node_id());
      } else {
        ++link_id;
      }
      const std::size_t prev_query_pos = state.query_pos();
      const std::size_t link = (links_[link_id] * 256)
          + labels_[state.node_id()];
      if (next_trie_.get() != NULL) {
        if (next_trie_->match(agent, link)) {
          return true;
        }
      } else if (tail_.match(agent, link)) {
        return true;
      }
      if (state.query_pos() != prev_query_pos) {
        return false;
      }
    } else if (labels_[state.node_id()] ==
        (UInt8)agent.query()[state.query_pos()]) {
      state.set_query_pos(state.query_pos() + 1);
      return true;
    }
    state.set_node_id(state.node_id() + 1);
    ++louds_pos;
  } while (louds_[louds_pos]);
  return false;
}

bool LoudsTrie::predictive_find_child(Agent &agent) const {
  MARISA_DEBUG_IF(agent.state().query_pos() >= agent.query().length(),
      MARISA_BOUND_ERROR);

  State &state = agent.state();
  std::size_t louds_pos = louds_.select0(state.node_id()) + 1;
  if (!louds_[louds_pos]) {
    return false;
  }
  state.set_node_id(louds_pos - state.node_id() - 1);
  std::size_t link_id = MARISA_INVALID_LINK_ID;
  do {
    if (link_flags_[state.node_id()]) {
      if (link_id == MARISA_INVALID_LINK_ID) {
        link_id = link_flags_.rank1(state.node_id());
      } else {
        ++link_id;
      }
      const std::size_t prev_query_pos = state.query_pos();
      const std::size_t link = (links_[link_id] * 256)
          + labels_[state.node_id()];
      if (next_trie_.get() != NULL) {
        if (next_trie_->prefix_match(agent, link)) {
          return true;
        }
      } else if (tail_.prefix_match(agent, link)) {
        return true;
      }
      if (state.query_pos() != prev_query_pos) {
        return false;
      }
    } else if (labels_[state.node_id()] ==
        (UInt8)agent.query()[state.query_pos()]) {
      state.key_buf().push_back((char)labels_[state.node_id()]);
      state.set_query_pos(state.query_pos() + 1);
      return true;
    }
    state.set_node_id(state.node_id() + 1);
    ++louds_pos;
  } while (louds_[louds_pos]);
  return false;
}

void LoudsTrie::restore(Agent &agent, std::size_t node_id) const {
  MARISA_DEBUG_IF(node_id == 0, MARISA_RANGE_ERROR);

  State &state = agent.state();
  for ( ; ; ) {
    if (link_flags_[node_id]) {
      const std::size_t link = labels_[node_id]
          + (links_[link_flags_.rank1(node_id)] * 256);
      if (next_trie_.get() != NULL) {
        next_trie_->restore(agent, link);
      } else {
        tail_.restore(agent, link);
      }
    } else {
      state.key_buf().push_back((char)labels_[node_id]);
    }

    if (node_id <= num_l1_nodes_) {
      return;
    }
    node_id = louds_.select1(node_id) - node_id - 1;
  }
}

bool LoudsTrie::match(Agent &agent, std::size_t node_id) const {
  MARISA_DEBUG_IF(agent.state().query_pos() >= agent.query().length(),
      MARISA_BOUND_ERROR);
  MARISA_DEBUG_IF(node_id == 0, MARISA_RANGE_ERROR);

  State &state = agent.state();
  for ( ; ; ) {
    if (link_flags_[node_id]) {
      const std::size_t link = labels_[node_id]
          + (links_[link_flags_.rank1(node_id)] * 256);
      if (next_trie_.get() != NULL) {
        if (!next_trie_->match(agent, link)) {
          return false;
        }
      } else if (!tail_.match(agent, link)) {
        return false;
      }
    } else if (labels_[node_id] == (UInt8)agent.query()[state.query_pos()]) {
      state.set_query_pos(state.query_pos() + 1);
    } else {
      return false;
    }

    if (node_id <= num_l1_nodes_) {
      return true;
    } else if (state.query_pos() >= agent.query().length()) {
      return false;
    }
    node_id = louds_.select1(node_id) - node_id - 1;
  }
}

bool LoudsTrie::prefix_match(Agent &agent, std::size_t node_id) const {
  MARISA_DEBUG_IF(agent.state().query_pos() >= agent.query().length(),
      MARISA_BOUND_ERROR);
  MARISA_DEBUG_IF(node_id == 0, MARISA_RANGE_ERROR);

  State &state = agent.state();
  for ( ; ; ) {
    if (link_flags_[node_id]) {
      const std::size_t link = labels_[node_id]
          + (links_[link_flags_.rank1(node_id)] * 256);
      if (next_trie_.get() != NULL) {
        if (!next_trie_->prefix_match(agent, link)) {
          return false;
        }
      } else if (!tail_.prefix_match(agent, link)) {
        return false;
      }
    } else if (labels_[node_id] == (UInt8)agent.query()[state.query_pos()]) {
      state.key_buf().push_back((char)labels_[node_id]);
      state.set_query_pos(state.query_pos() + 1);
    } else {
      return false;
    }

    if (node_id <= num_l1_nodes_) {
      return true;
    }
    node_id = louds_.select1(node_id) - node_id - 1;
    if (state.query_pos() >= agent.query().length()) {
      restore(agent, node_id);
      return true;
    }
  }
}

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa
