#include <cstdlib>
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

marisa::UInt32 max_num_tries = 3;
bool patricia_flag = true;
bool tail_flag = true;
bool weight_order_flag = true;
const char *output_filename = NULL;

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
      "  -o, --output=[FILE]  write tries to FILE (default: stdout)\n"
      "  -h, --help           print this help\n"
      << std::endl;
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

int build(const char * const *args, std::size_t num_args) {
  std::vector<Key> keys;
  if (num_args == 0) {
    read_keys(&std::cin, &keys);
  }

  for (std::size_t i = 0; i < num_args; ++i) {
    std::ifstream input_file(args[i], std::ios::binary);
    if (!input_file) {
      std::cerr << "error: failed to open a keyset file: "
          << args[i] << std::endl;
      return 10;
    }
    read_keys(&input_file, &keys);
  }

  marisa::Trie trie;
  trie.build(keys, NULL, max_num_tries,
      patricia_flag, tail_flag, weight_order_flag);

  std::cerr << "#keys: " << trie.num_keys() << std::endl;
  std::cerr << "#tries: " << trie.num_tries() << std::endl;
  std::cerr << "#nodes: " << trie.num_nodes() << std::endl;
  std::cerr << "size: " << trie.size() << std::endl;

  if (output_filename != NULL) {
    if (!trie.save(output_filename)) {
      std::cerr << "error: failed to write tries to file: "
          << output_filename << std::endl;
      return 20;
    }
  } else {
    if (!trie.write(&std::cout)) {
      std::cerr << "error: failed to write tries to standard output"
          << std::endl;
      return 21;
    }
  }
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
    { "output", 1, NULL, 'o' },
    { "help", 0, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  ::cmdopt_t cmdopt;
  ::cmdopt_init(&cmdopt, argc, argv, "n:PpTtwlo:h", long_options);
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
      case 'o': {
        output_filename = cmdopt.optarg;
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
  return build(cmdopt.argv + cmdopt.optind, cmdopt.argc - cmdopt.optind);
}
