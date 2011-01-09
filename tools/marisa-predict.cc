#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <marisa.h>

#include "./cmdopt.h"

namespace {

marisa::UInt32 max_num_results = 10;
bool mmap_flag = true;

void print_help(const char *cmd) {
  std::cerr << "Usage: " << cmd << " [OPTION]... DIC\n\n"
      "Options:\n"
      "  -n, --max-num-results=[N]  limits the number of outputs to N"
      " (default: 10)\n"
      "                             0: no limit\n"
      "  -m, --mmap-dictionary  use memory-mapped I/O to load a dictionary"
      " (default)\n"
      "  -r, --read-dictionary  read an entire dictionary into memory\n"
      "  -h, --help             print this help\n"
      << std::endl;
}

class PredictCallback {
 public:
  PredictCallback(std::vector<marisa::UInt32> *key_ids,
      std::vector<std::string> *keys) : key_ids_(key_ids), keys_(keys) {
    key_ids_->clear();
    keys_->clear();
  }
  PredictCallback(const PredictCallback &callback)
      : key_ids_(callback.key_ids_), keys_(callback.keys_) {}

  bool operator()(marisa::UInt32 key_id, const std::string &key) const {
    key_ids_->push_back(key_id);
    keys_->push_back(key);
    return key_ids_->size() < max_num_results;
  }

 private:
  std::vector<marisa::UInt32> *key_ids_;
  std::vector<std::string> *keys_;

  // Disallows assignment.
  PredictCallback &operator=(const PredictCallback &);
};

int predict(const char * const *args, std::size_t num_args) {
  if (num_args == 0) {
    std::cerr << "error: tries are not specified" << std::endl;
    return 10;
  } else if (num_args > 1) {
    std::cerr << "error: more than one tries are specified" << std::endl;
    return 11;
  }

  marisa::Trie trie;
  marisa::Mapper mapper;
  if (mmap_flag) {
    if (!trie.mmap(&mapper, args[0])) {
      std::cerr << "error: failed to mmap tries: " << args[0] << std::endl;
      return 20;
    }
  } else {
    if (!trie.load(args[0])) {
      std::cerr << "error: failed to load tries: " << args[0] << std::endl;
      return 21;
    }
  }

  std::vector<marisa::UInt32> key_ids;
  std::vector<std::string> keys;
  std::string str;
  while (std::getline(std::cin, str)) {
    marisa::UInt32 num_keys = trie.predict(str);
    if (num_keys != 0) {
      std::cout << num_keys << " found" << std::endl;
      num_keys = trie.predict_callback(str, PredictCallback(&key_ids, &keys));
      for (marisa::UInt32 i = 0; i < num_keys; ++i) {
        std::cout << key_ids[i] << '\t' << keys[i] << '\t' << str << '\n';
      }
    } else {
      std::cout << "not found" << std::endl;
    }
    if (!std::cout) {
      std::cerr << "error: failed to write results to standard output"
          << std::endl;
      return 30;
    }
  }

  return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);

  ::cmdopt_option long_options[] = {
    { "max-num-results", 1, NULL, 'n' },
    { "mmap-dictionary", 0, NULL, 'm' },
    { "read-dictionary", 0, NULL, 'r' },
    { "help", 0, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  ::cmdopt_t cmdopt;
  ::cmdopt_init(&cmdopt, argc, argv, "n:mrh", long_options);
  int label;
  while ((label = ::cmdopt_get(&cmdopt)) != -1) {
    switch (label) {
      case 'n': {
        char *end_of_value;
        long value = std::strtol(cmdopt.optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value < 0)) {
          std::cerr << "error: option `-n' with an invalid argument: "
              << cmdopt.optarg << std::endl;
        }
        if ((value == 0) ||
            (static_cast<unsigned long>(value) > 0xFFFFFFFFUL)) {
          max_num_results = std::numeric_limits<marisa::UInt32>::max();
        } else {
          max_num_results = static_cast<marisa::UInt32>(value);
        }
        break;
      }
      case 'm': {
        mmap_flag = true;
        break;
      }
      case 'r': {
        mmap_flag = false;
        break;
      }
      case 'h': {
        print_help(argv[0]);
        return 0;
      }
      default: {
        return 1;
      }
    }
  }
  return predict(cmdopt.argv + cmdopt.optind, cmdopt.argc - cmdopt.optind);
}
