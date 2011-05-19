#include <cstring>
#include <new>

#include "marisa-swig.h"

namespace marisa_swig {

void Key::str(const char **ptr, std::size_t *length) const {
  *ptr = key_.ptr();
  *length = key_.length();
}

std::size_t Key::length() const {
  return key_.length();
}

std::size_t Key::id() const {
  return key_.id();
}

float Key::weight() const {
  return key_.weight();
}

void Query::str(const char **ptr, std::size_t *length) const {
  *ptr = query_.ptr();
  *length = query_.length();
}

std::size_t Query::length() const {
  return query_.length();
}

std::size_t Query::key_id() const {
  return query_.key_id();
}

Keyset::Keyset() : keyset_(new (std::nothrow) marisa::Keyset) {
  MARISA_THROW_IF(keyset_ == NULL, ::MARISA_MEMORY_ERROR);
}

Keyset::~Keyset() {
  delete keyset_;
}

void Keyset::push_back(const marisa::Key &key) {
  keyset_->push_back(key);
}

void Keyset::push_back(const marisa::Key &key, char end_marker) {
  keyset_->push_back(key, end_marker);
}

void Keyset::push_back(const char *ptr, std::size_t length, float weight) {
  keyset_->push_back(ptr, length, weight);
}

const Key &Keyset::key(std::size_t i) const {
  return reinterpret_cast<const Key &>((*keyset_)[i]);
}

std::size_t Keyset::num_keys() const {
  return keyset_->num_keys();
}

bool Keyset::empty() const {
  return keyset_->empty();
}

std::size_t Keyset::size() const {
  return keyset_->size();
}

std::size_t Keyset::total_length() const {
  return keyset_->total_length();
}

void Keyset::reset() {
  keyset_->reset();
}

void Keyset::clear() {
  keyset_->clear();
}

Agent::Agent()
    : agent_(new (std::nothrow) marisa::Agent), buf_(NULL), size_(0) {
  MARISA_THROW_IF(agent_ == NULL, ::MARISA_MEMORY_ERROR);
}

Agent::~Agent() {
  delete agent_;
  delete [] buf_;
}

void Agent::set_query(const char *ptr, std::size_t length) {
  if (length > size_) {
    std::size_t new_size = (size_ != 0) ? (size_ * 2) : 1;
    if (length > (MARISA_SIZE_MAX / 2)) {
      new_size = MARISA_SIZE_MAX;
    } else {
      while (length > new_size) {
        new_size *= 2;
      }
    }
    char * const new_buf = new (std::nothrow) char[new_size];
    MARISA_THROW_IF(new_buf == NULL, ::MARISA_MEMORY_ERROR);
    delete [] buf_;
    buf_ = new_buf;
    size_ = new_size;
  }
  std::memcpy(buf_, ptr, length);
  agent_->set_query(buf_, length);
}

void Agent::set_query(std::size_t key_id) {
  agent_->set_query(key_id);
}

const Key &Agent::key() const {
  return reinterpret_cast<const Key &>(agent_->key());
}

const Query &Agent::query() const {
  return reinterpret_cast<const Query &>(agent_->query());
}

Trie::Trie() : trie_(new (std::nothrow) marisa::Trie) {
  MARISA_THROW_IF(trie_ == NULL, ::MARISA_MEMORY_ERROR);
}

Trie::~Trie() {
  delete trie_;
}

void Trie::build(Keyset &keyset, int config_flags) {
  trie_->build(*keyset.keyset_, config_flags);
}

void Trie::mmap(const char *filename) {
  trie_->mmap(filename);
}

void Trie::load(const char *filename) {
  trie_->load(filename);
}

void Trie::save(const char *filename) const {
  trie_->save(filename);
}

bool Trie::lookup(Agent &agent) const {
  return trie_->lookup(*agent.agent_);
}

void Trie::reverse_lookup(Agent &agent) const {
  trie_->reverse_lookup(*agent.agent_);
}

bool Trie::common_prefix_search(Agent &agent) const {
  return trie_->common_prefix_search(*agent.agent_);
}

bool Trie::predictive_search(Agent &agent) const {
  return trie_->predictive_search(*agent.agent_);
}

std::size_t Trie::num_tries() const {
  return trie_->num_tries();
}

std::size_t Trie::num_keys() const {
  return trie_->num_keys();
}

std::size_t Trie::num_nodes() const {
  return trie_->num_nodes();
}

TailMode Trie::tail_mode() const {
  if (trie_->tail_mode() == ::MARISA_TEXT_TAIL) {
    return MARISA_TEXT_TAIL;
  } else {
    return MARISA_BINARY_TAIL;
  }
}

NodeOrder Trie::node_order() const {
  if (trie_->node_order() == ::MARISA_LABEL_ORDER) {
    return MARISA_LABEL_ORDER;
  } else {
    return MARISA_WEIGHT_ORDER;
  }
}

bool Trie::empty() const {
  return trie_->empty();
}

std::size_t Trie::size() const {
  return trie_->size();
}

std::size_t Trie::total_size() const {
  return trie_->total_size();
}

std::size_t Trie::io_size() const {
  return trie_->io_size();
}

void Trie::clear() {
  trie_->clear();
}

}  // namespace marisa_swig
