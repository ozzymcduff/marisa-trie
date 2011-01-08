#include <cassert>

#include <marisa/range.h>

int main() {
  {
    marisa::Range range;
    assert(range.begin() == 0);
    assert(range.end() == 0);
    assert(range.pos() == 0);

    range.set_begin(1);
    range.set_end(2);
    range.set_pos(3);

    assert(range.begin() == 1);
    assert(range.end() == 2);
    assert(range.pos() == 3);

    range = marisa::Range(4, 5, 6);

    assert(range.begin() == 4);
    assert(range.end() == 5);
    assert(range.pos() == 6);
  }

  {
    marisa::WRange range;
    assert(range.begin() == 0);
    assert(range.end() == 0);
    assert(range.pos() == 0);
    assert(range.weight() == 0.0);

    range.set_begin(1);
    range.set_end(2);
    range.set_pos(3);
    range.set_weight(4.0);

    assert(range.range().begin() == 1);
    assert(range.range().end() == 2);
    assert(range.range().pos() == 3);

    assert(range.begin() == 1);
    assert(range.end() == 2);
    assert(range.pos() == 3);
    assert(range.weight() == 4.0);

    range = marisa::WRange(5, 6, 7, 8.0);

    assert(range.begin() == 5);
    assert(range.end() == 6);
    assert(range.pos() == 7);
    assert(range.weight() == 8.0);

    range = marisa::WRange(marisa::Range(1, 2, 3), 4.0);

    assert(range.begin() == 1);
    assert(range.end() == 2);
    assert(range.pos() == 3);
    assert(range.weight() == 4.0);
  }

  return 0;
}
