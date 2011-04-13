#ifndef MARISA_GRIMOIRE_ALGORITHM_SORT_H_
#define MARISA_GRIMOIRE_ALGORITHM_SORT_H_

#include "../../base.h"

namespace marisa {
namespace grimoire {
namespace algorithm {
namespace details {

enum Threshold {
  MARISA_INSERTION_SORT_THRESHOLD = 10
};

template <typename Unit>
int get_label(const Unit &unit, std::size_t depth) {
  MARISA_DEBUG_IF(depth > unit.length(), MARISA_BOUND_ERROR);

  return (depth < unit.length()) ? (int)(UInt8)unit[depth] : -1;
}

template <typename Unit>
int median(const Unit &a, const Unit &b, const Unit &c, std::size_t depth) {
  const int x = get_label(a, depth);
  const int y = get_label(b, depth);
  const int z = get_label(c, depth);
  if (x < y) {
    if (y < z) {
      return y;
    } else if (x < z) {
      return z;
    }
    return x;
  } else if (x < z) {
    return x;
  } else if (y < z) {
    return z;
  }
  return y;
}

template <typename Unit>
bool less_than(const Unit &lhs, const Unit &rhs, std::size_t depth) {
  for (std::size_t i = depth; i < lhs.length(); ++i) {
    if (i == rhs.length()) {
      return false;
    }
    if (lhs[i] != rhs[i]) {
      return (UInt8)lhs[i] < (UInt8)rhs[i];
    }
  }
  return lhs.length() < rhs.length();
}

template <typename Iterator>
void insertion_sort(Iterator l, Iterator r, std::size_t depth) {
  MARISA_DEBUG_IF(l > r, MARISA_BOUND_ERROR);

  for (Iterator i = l + 1; i < r; ++i) {
    for (Iterator j = i; j > l; --j) {
      if (!less_than(*j, *(j - 1), depth)) {
        break;
      }
      marisa::swap(*(j - 1), *j);
    }
  }
}

template <typename Iterator>
void sort(Iterator l, Iterator r, std::size_t depth) {
  MARISA_DEBUG_IF(l > r, MARISA_BOUND_ERROR);

  while ((r - l) > MARISA_INSERTION_SORT_THRESHOLD) {
    Iterator pl = l;
    Iterator pr = r;
    Iterator pivot_l = l;
    Iterator pivot_r = r;

    const int pivot = median(*l, *(l + (r - l) / 2), *(r - 1), depth);
    for ( ; ; ) {
      while (pl < pr) {
        const int label = get_label(*pl, depth);
        if (label > pivot) {
          break;
        } else if (label == pivot) {
          marisa::swap(*pl, *pivot_l);
          ++pivot_l;
        }
        ++pl;
      }
      while (pl < pr) {
        const int label = get_label(*--pr, depth);
        if (label < pivot) {
          break;
        } else if (label == pivot) {
          marisa::swap(*pr, *--pivot_r);
        }
      }
      if (pl >= pr) {
        break;
      }
      marisa::swap(*pl, *pr);
      ++pl;
    }
    while (pivot_l > l) {
      marisa::swap(*--pivot_l, *--pl);
    }
    while (pivot_r < r) {
      marisa::swap(*pivot_r, *pr);
      ++pivot_r;
      ++pr;
    }

    if (((pl - l) > (pr - pl)) || ((r - pr) > (pr - pl))) {
      if (pr - pl > 1) {
        if (pivot != -1) {
          sort(pl, pr, depth + 1);
        }
      }

      if ((pl - l) < (r - pr)) {
        if ((pl - l) > 1) {
          sort(l, pl, depth);
        }
        l = pr;
      } else {
        if ((r - pr) > 1) {
          sort(pr, r, depth);
        }
        r = pl;
      }
    } else {
      if ((pl - l) > 1) {
        sort(l, pl, depth);
      }
      if ((r - pr) > 1) {
        sort(pr, r, depth);
      }
      l = pl, r = pr;
      if ((pr - pl) > 1) {
        if (pivot != -1) {
          ++depth;
        } else {
          l = r;
        }
      }
    }
  }

  if (r - l > 1) {
    insertion_sort(l, r, depth);
  }
}

}  // namespace details

template <typename Iterator>
void sort(Iterator begin, Iterator end) {
  MARISA_DEBUG_IF(begin > end, MARISA_BOUND_ERROR);
  details::sort(begin, end, 0);
};

}  // namespace algorithm
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_ALGORITHM_SORT_H_
