#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <marisa.h>

#include "./cmdopt.h"

namespace {

typedef std::pair<std::string, double> Key;

marisa::UInt32 max_num_tries = 10;
bool patricia_flag = true;
bool tail_flag = true;
bool weight_order_flag = true;
bool speed_flag = true;

class Clock {
 public:
  Clock() : cl_(std::clock()) {}

  void reset() {
    cl_ = std::clock();
  }

  double elasped() const {
    std::clock_t cur = std::clock();
    return (cur == cl_) ? 0.01 : (1.0 * (cur - cl_) / CLOCKS_PER_SEC);
  }

 private:
  std::clock_t cl_;
};

void print_help(const char *cmd) {
  std::cerr << "Usage: " << cmd << " [OPTION]... [FILE]...\n\n"
      "Options:\n"
      "  -n, --max-num-tries=[N]  limits the number of tries to N"
      " (default: 3)\n"
      "  -P, --patricia-trie  build patricia tries (default)\n"
      "  -p, --prefix-trie    build prefix tries\n"
      "  -T, --with-tail      build a dictionary with TAIL (default)\n"
      "  -t, --without-tail   build a dictionary without TAIL\n"
      "  -w, --weight-order   arranges siblings in weight order (default)\n"
      "  -l, --label-order    arranges siblings in label order\n"
      "  -S, --print-speed    print speed [K/s] (default)\n"
      "  -s, --print-time     print time [ms]\n"
      "  -h, --help           print this help\n"
      << std::endl;
}

void print_config() {
  std::cout << "max. #tries: " << max_num_tries << std::endl;

  if (patricia_flag) {
    std::cout << "patricia: on" << std::endl;
  } else {
    std::cout << "patricia: off" << std::endl;
  }

  if (tail_flag) {
    std::cout << "tail: on" << std::endl;
  } else {
    std::cout << "tail: off" << std::endl;
  }

  if (weight_order_flag) {
    std::cout << "weight order: on" << std::endl;
  } else {
    std::cout << "weight order: off" << std::endl;
  }
}

void print_time_info(std::size_t num_keys, double elasped) {
  if (speed_flag) {
    if (elasped == 0.0) {
      std::printf(" %7s", "-");
    } else {
      std::printf(" %7.2f", num_keys / elasped / 1000.0);
    }
  } else {
    if (num_keys == 0) {
      std::printf(" %7s", "-");
    } else {
      std::printf(" %7.3f", 1000000.0 * elasped / num_keys);
    }
  }
}

void read_keys(std::istream *input, std::vector<Key> *keys) {
  Key key;
  std::string line;
  while (std::getline(*input, line)) {
    std::string::size_type delim_pos = line.find_last_of('\t');
    if (delim_pos != line.npos) {
      char *end_of_value;
      key.second = std::strtod(&line[delim_pos + 1], &end_of_value);
      if (*end_of_value == '\0') {
        line.resize(delim_pos);
      } else {
        key.second = 1.0;
      }
    } else {
      key.second = 1.0;
    }
    key.first = line;
    keys->push_back(key);
  }
}

int read_keys(const char * const *args, std::size_t num_args,
    std::vector<Key> *keys) {
  if (num_args == 0) {
    read_keys(&std::cin, keys);
  }
  for (std::size_t i = 0; i < num_args; ++i) {
    std::ifstream input_file(args[i], std::ios::binary);
    if (!input_file) {
      std::cerr << "error: failed to open a keyset file: "
          << args[i] << std::endl;
      return 10;
    }
    read_keys(&input_file, keys);
  }
  std::cout << "#keys: " << keys->size() << std::endl;
  std::size_t total_length = 0;
  for (std::size_t i = 0; i < keys->size(); ++i) {
    total_length += (*keys)[i].first.length();
  }
  std::cout << "total length: " << total_length << std::endl;
  return 0;
}

void benchmark_build(const std::vector<Key> &keys, marisa::UInt32 num_tries,
    marisa::Trie *trie, std::vector<marisa::UInt32> *key_ids) {
  Clock cl;
  trie->build(keys, key_ids, num_tries,
      patricia_flag, tail_flag, weight_order_flag);
  std::printf(" %8lu", static_cast<unsigned long>(trie->num_nodes()));
  std::printf(" %9lu", static_cast<unsigned long>(trie->size()));
  print_time_info(keys.size(), cl.elasped());
}

void benchmark_restore(const marisa::Trie &trie,
    const std::vector<Key> &keys,
    const std::vector<marisa::UInt32> &key_ids) {
  Clock cl;
  std::string key;
  for (std::size_t i = 0; i < key_ids.size(); ++i) {
    key.clear();
    trie.restore(key_ids[i], &key);
    if (key != keys[i].first) {
      std::cerr << "error: restore() failed" << std::endl;
      return;
    }
  }
  print_time_info(key_ids.size(), cl.elasped());
}

void benchmark_lookup(const marisa::Trie &trie,
    const std::vector<Key> &keys,
    const std::vector<marisa::UInt32> &key_ids) {
  Clock cl;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    marisa::UInt32 key_id = trie.lookup(keys[i].first);
    if (key_id != key_ids[i]) {
      std::cerr << "error: lookup() failed" << std::endl;
      return;
    }
  }
  print_time_info(keys.size(), cl.elasped());
}

void benchmark_find(const marisa::Trie &trie,
    const std::vector<Key> &keys,
    const std::vector<marisa::UInt32> &key_ids) {
  Clock cl;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    marisa::UInt32 num_keys = trie.find(keys[i].first);
    if (num_keys == 0) {
      std::cerr << "error: find() failed" << std::endl;
      return;
    }
  }
  print_time_info(keys.size(), cl.elasped());
}

void benchmark_predict_breadth_first(const marisa::Trie &trie,
    const std::vector<Key> &keys,
    const std::vector<marisa::UInt32> &key_ids) {
  Clock cl;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    marisa::UInt32 num_keys = trie.predict(keys[i].first);
    if (num_keys == 0) {
      std::cerr << "error: predict() failed" << std::endl;
      return;
    }
  }
  print_time_info(keys.size(), cl.elasped());
}

void benchmark_predict_depth_first(const marisa::Trie &trie,
    const std::vector<Key> &keys,
    const std::vector<marisa::UInt32> &key_ids) {
  Clock cl;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    marisa::UInt32 num_keys = trie.predict(keys[i].first, NULL, NULL);
    if (num_keys == 0) {
      std::cerr << "error: predict() failed" << std::endl;
      return;
    }
  }
  print_time_info(keys.size(), cl.elasped());
}

void benchmark(const std::vector<Key> &keys, marisa::UInt32 num_tries) {
  std::printf("%6d", num_tries);
  marisa::Trie trie;
  std::vector<marisa::UInt32> key_ids;
  benchmark_build(keys, num_tries, &trie, &key_ids);
  benchmark_restore(trie, keys, key_ids);
  benchmark_lookup(trie, keys, key_ids);
  benchmark_find(trie, keys, key_ids);
  benchmark_predict_breadth_first(trie, keys, key_ids);
  benchmark_predict_depth_first(trie, keys, key_ids);
  std::printf("\n");
}

int benchmark(const char * const *args, std::size_t num_args) {
  std::vector<Key> keys;
  int ret = read_keys(args, num_args, &keys);
  if (ret != 0) {
    return ret;
  }
  std::printf("------+--------+---------+-------+"
      "-------+-------+-------+-------+-------\n");
  std::printf("%6s %8s %9s %7s %7s %7s %7s %7s %7s\n",
      "#tries", "#nodes", "size",
      "build", "restore", "lookup", "find", "predict", "predict");
  std::printf("%6s %8s %9s %7s %7s %7s %7s %7s %7s\n",
      "", "", "", "", "", "", "", "breadth", "depth");
  if (speed_flag) {
    std::printf("%6s %8s %9s %7s %7s %7s %7s %7s %7s\n",
        "", "", "[bytes]",
        "[K/s]", "[K/s]", "[K/s]", "[K/s]", "[K/s]", "[K/s]");
  } else {
    std::printf("%6s %8s %9s %7s %7s %7s %7s %7s %7s\n",
        "", "", "[bytes]", "[us]", "[us]", "[us]", "[us]", "[us]", "[us]");
  }
  std::printf("------+--------+---------+-------+"
      "-------+-------+-------+-------+-------\n");
  for (marisa::UInt32 i = 1; i <= max_num_tries; ++i) {
    benchmark(keys, i);
  }
  std::printf("------+--------+---------+-------+"
      "-------+-------+-------+-------+-------\n");
  return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);

  ::cmdopt_option long_options[] = {
    { "max-num-tries", 1, NULL, 'n' },
    { "patricia-trie", 0, NULL, 'P' },
    { "prefix-trie", 0, NULL, 'p' },
    { "with-tail", 0, NULL, 'T' },
    { "without-tail", 0, NULL, 't' },
    { "weight-order", 0, NULL, 'w' },
    { "label-order", 0, NULL, 'l' },
    { "print-speed", 0, NULL, 'S' },
    { "print-time", 0, NULL, 's' },
    { "output", 1, NULL, 'o' },
    { "help", 0, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  ::cmdopt_t cmdopt;
  ::cmdopt_init(&cmdopt, argc, argv, "n:PpTtwlSso:h", long_options);
  int label;
  while ((label = ::cmdopt_get(&cmdopt)) != -1) {
    switch (label) {
      case 'n': {
        char *end_of_value;
        long value = std::strtol(cmdopt.optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value <= 0) ||
            (static_cast<unsigned long>(value) > 0xFFFFFFFFUL)) {
          std::cerr << "error: option `-n' with an invalid argument: "
              << cmdopt.optarg << std::endl;
        }
        max_num_tries = static_cast<marisa::UInt32>(value);
        break;
      }
      case 'P': {
        patricia_flag = true;
        break;
      }
      case 'p': {
        patricia_flag = false;
        break;
      }
      case 'T': {
        tail_flag = true;
        break;
      }
      case 't': {
        tail_flag = false;
        break;
      }
      case 'w': {
        weight_order_flag = true;
        break;
      }
      case 'l': {
        weight_order_flag = false;
        break;
      }
      case 'S': {
        speed_flag = true;
        break;
      }
      case 's': {
        speed_flag = false;
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
  print_config();
  return benchmark(cmdopt.argv + cmdopt.optind, cmdopt.argc - cmdopt.optind);
}
