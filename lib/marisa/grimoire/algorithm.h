#ifndef MARISA_GRIMOIRE_ALGORITHM_H_
#define MARISA_GRIMOIRE_ALGORITHM_H_

#include "algorithm/sort.h"

namespace marisa {
namespace grimoire {

class Algorithm {
 public:
  Algorithm() {}

  template <typename Iterator>
  void sort(Iterator begin, Iterator end) {
    algorithm::sort(begin, end);
  }

 private:
  Algorithm(const Algorithm &);
  Algorithm &operator=(const Algorithm &);
};

}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_ALGORITHM_H_
