#ifndef MARISA_TRIE_H_
#define MARISA_TRIE_H_

#include <memory>

#include "./config.h"
#include "./key.h"
#include "./query.h"
#include "./intvector.h"
#include "./bitvector.h"
#include "./tail.h"

namespace marisa {

class Trie {
 public:
  Trie();

  void build(const std::vector<const char *> &keys,
      std::vector<UInt32> *key_ids = NULL, UInt32 max_num_tries = 3,
      bool patricia = true, bool tail = true, bool freq_order = true);
  void build(const std::vector<char *> &keys,
      std::vector<UInt32> *key_ids = NULL, UInt32 max_num_tries = 3,
      bool patricia = true, bool tail = true, bool freq_order = true);
  void build(const std::vector<std::string> &keys,
      std::vector<UInt32> *key_ids = NULL, UInt32 max_num_tries = 3,
      bool patricia = true, bool tail = true, bool freq_order = true);

  void build(const std::vector<std::pair<const char *, double> > &keys,
      std::vector<UInt32> *key_ids = NULL, UInt32 max_num_tries = 3,
      bool patricia = true, bool tail = true, bool weight_order = true);
  void build(const std::vector<std::pair<char *, double> > &keys,
      std::vector<UInt32> *key_ids = NULL, UInt32 max_num_tries = 3,
      bool patricia = true, bool tail = true, bool weight_order = true);
  void build(const std::vector<std::pair<std::string, double> > &keys,
      std::vector<UInt32> *key_ids = NULL, UInt32 max_num_tries = 3,
      bool patricia = true, bool tail = true, bool weight_order = true);

  bool mmap(Mapper *mapper, const char *filename,
      long offset = 0, int whence = SEEK_SET);
  bool map(const void *ptr);
  bool map(const void *ptr, std::size_t size);
  bool map(Mapper *mapper);

  bool load(const char *filename, long offset = 0, int whence = SEEK_SET);
  bool read(int fd);
  bool read(::FILE *file);
  bool read(std::istream *stream);
  bool read(Reader *reader);

  bool save(const char *filename, bool trunc_flag = true,
      long offset = 0, int whence = SEEK_SET) const;
  bool write(int fd) const;
  bool write(::FILE *file) const;
  bool write(std::ostream *stream) const;
  bool write(Writer *writer) const;

  std::string operator[](UInt32 key_id) const;

  UInt32 operator[](const char *str) const;
  UInt32 operator[](const std::string &str) const;

  std::string restore(UInt32 key_id) const;
  void restore(UInt32 key_id, std::string *key) const;

  UInt32 lookup(const char *str) const;
  UInt32 lookup(const char *ptr, std::size_t length) const;
  UInt32 lookup(const std::string &str) const;

  UInt32 find(const char *str,
      std::vector<UInt32> *key_ids = NULL,
      std::vector<std::size_t> *key_lengths = NULL) const;
  UInt32 find(const char *ptr, std::size_t length,
      std::vector<UInt32> *key_ids = NULL,
      std::vector<std::size_t> *key_lengths = NULL) const;
  UInt32 find(const std::string &str,
      std::vector<UInt32> *key_ids = NULL,
      std::vector<std::size_t> *key_lengths = NULL) const;

  UInt32 find_first(const char *str,
      std::size_t *key_length = NULL) const;
  UInt32 find_first(const char *ptr, std::size_t length,
      std::size_t *key_length = NULL) const;
  UInt32 find_first(const std::string &str,
      std::size_t *key_length = NULL) const;

  UInt32 find_last(const char *str,
      std::size_t *key_length = NULL) const;
  UInt32 find_last(const char *ptr, std::size_t length,
      std::size_t *key_length = NULL) const;
  UInt32 find_last(const std::string &str,
      std::size_t *key_length = NULL) const;

  // bool callback(UInt32 key_id, std::size_t key_length);
  template <typename T>
  UInt32 find_callback(const char *str, T callback) const;
  template <typename T>
  UInt32 find_callback(const char *ptr, std::size_t length,
      T callback) const;
  template <typename T>
  UInt32 find_callback(const std::string &str, T callback) const;

  UInt32 predict(const char *str,
      std::vector<UInt32> *key_ids = NULL) const;
  UInt32 predict(const char *ptr, std::size_t length,
      std::vector<UInt32> *key_ids = NULL) const;
  UInt32 predict(const std::string &str,
      std::vector<UInt32> *key_ids = NULL) const;

  UInt32 predict(const char *str,
      std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const;
  UInt32 predict(const char *ptr, std::size_t length,
      std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const;
  UInt32 predict(const std::string &str,
      std::vector<UInt32> *key_ids, std::vector<std::string> *keys) const;

  // bool callback(UInt32 key_id, const std::string &key);
  template <typename T>
  UInt32 predict_callback(const char *str, T callback) const;
  template <typename T>
  UInt32 predict_callback(const char *ptr, std::size_t length,
      T callback) const;
  template <typename T>
  UInt32 predict_callback(const std::string &str, T callback) const;

  UInt32 num_keys() const;
  UInt32 num_tries() const;
  UInt32 num_nodes() const;
  std::size_t size() const;

  void clear();
  void swap(Trie *rhs);

  static UInt32 notfound();
  static std::size_t mismatch();

  static const UInt32 NOTFOUND = static_cast<UInt32>(-1);
  static const std::size_t MISMATCH = static_cast<std::size_t>(-2);

 private:
  BitVector louds_;
  Vector<UInt8> labels_;
  BitVector terminal_flags_;
  BitVector link_flags_;
  IntVector links_;
  std::auto_ptr<Trie> trie_;
  Tail tail_;
  UInt32 num_keys_;

  template <typename T>
  void build_from(const std::vector<T> &keys,
      std::vector<UInt32> *key_ids, const Config &config);
  template <typename T>
  void build_from(const std::vector<std::pair<T, double> > &keys,
      std::vector<UInt32> *key_ids, const Config &config);

  void build_trie(std::vector<Key<String> > &keys,
      std::vector<UInt32> *key_ids, const Config &config);

  template <typename T>
  void build_trie(std::vector<Key<T> > &keys,
      std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config);

  template <typename T>
  void build_cur(std::vector<Key<T> > &keys,
      std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config);

  void build_next(std::vector<Key<String> > &keys,
      std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config);
  void build_next(std::vector<Key<RString> > &rkeys,
      std::vector<UInt32> *terminals, UInt32 trie_id, const Config &config);

  template <typename T>
  UInt32 sort_keys(std::vector<Key<T> > &keys) const;

  template <typename T>
  void build_terminals(const std::vector<Key<T> > &keys,
      std::vector<UInt32> *terminals) const;

  void restore_(UInt32 key_id, std::string *key) const;

  void trie_restore(UInt32 node, std::string *key) const;
  void tail_restore(UInt32 offset, std::string *key) const;

  template <typename T>
  UInt32 lookup_(T query) const;

  template <typename T>
  UInt32 find_(T query, std::vector<UInt32> *key_ids,
      std::vector<std::size_t> *key_lengths) const;
  template <typename T>
  UInt32 find_first_(T query, std::size_t *key_length) const;
  template <typename T>
  UInt32 find_last_(T query, std::size_t *key_length) const;
  template <typename T, typename U>
  UInt32 find_callback_(T query, U callback) const;

  template <typename T>
  bool find_child(UInt32 &node, T query, std::size_t &pos) const;
  template <typename T>
  std::size_t trie_match(UInt32 node, T query, std::size_t pos) const;
  template <typename T>
  std::size_t tail_match(UInt32 offset, T query, std::size_t pos) const;

  template <typename T>
  UInt32 predict_(T query, std::vector<UInt32> *key_ids) const;
  template <typename T>
  UInt32 predict_(T query, std::vector<UInt32> *key_ids,
      std::vector<std::string> *keys) const;
  template <typename T, typename U>
  UInt32 predict_callback_(T query, U callback) const;

  template <typename T>
  bool predict_child(UInt32 &node, T query, std::size_t &pos,
      std::string *key = NULL) const;
  template <typename T>
  std::size_t trie_prefix_match(UInt32 node, T query,
      std::size_t pos, std::string *key = NULL) const;
  template <typename T>
  std::size_t tail_prefix_match(UInt32 offset, T query,
      std::size_t pos, std::string *key = NULL) const;

  UInt32 key_id_to_node(UInt32 key_id) const;
  UInt32 node_to_key_id(UInt32 node) const;
  UInt32 louds_pos_to_node(UInt32 louds_pos) const;

  UInt32 get_child(UInt32 node) const;
  UInt32 get_parent(UInt32 node) const;

  bool has_link(UInt32 node) const;
  UInt32 get_link(UInt32 node) const;

  bool has_link() const;
  bool has_trie() const;
  bool has_tail() const;

  // Disallows copy and assignment.
  Trie(const Trie &);
  Trie &operator=(const Trie &);
};

}  // namespace marisa

#include "./trie-inline.h"

#endif  // MARISA_TRIE_H_
