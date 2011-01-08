#include <cassert>

#include <marisa/key.h>

int main() {
  {
    marisa::Key<marisa::String> key;
    assert(key.str().length() == 0);
    assert(key.weight() == 0.0);
    assert(key.id() == 0);
    assert(key.terminal() == 0);

    key.set_str(marisa::String("abc"));
    key.set_weight(1.0);
    key.set_id(2);
    key.set_terminal(3);

    assert(key.str() == marisa::String("abc"));
    assert(key.weight() == 1.0);
    assert(key.id() == 2);
    assert(key.terminal() == 3);
  }

  {
    marisa::String str("abc");

    marisa::Key<marisa::RString> key;
    assert(key.str().length() == 0);
    assert(key.weight() == 0.0);
    assert(key.id() == 0);
    assert(key.terminal() == 0);

    key.set_str(marisa::RString(str));
    key.set_weight(1.0);
    key.set_id(2);
    key.set_terminal(3);

    assert(key.str() == marisa::RString(str));
    assert(key.weight() == 1.0);
    assert(key.id() == 2);
    assert(key.terminal() == 3);
  }

  return 0;
}
