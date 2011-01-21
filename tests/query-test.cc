#include <cassert>

#include <marisa/query.h>

int main() {
  {
    marisa::CQuery query("abc");
    assert(query[0] == 'a');
    assert(!query.ends_at(0));
    assert(query[1] == 'b');
    assert(!query.ends_at(1));
    assert(query[2] == 'c');
    assert(!query.ends_at(2));
    assert(query.ends_at(3));

    std::string str("str");
    query.insert(&str);
    assert(str == "abcstr");
  }

  {
    marisa::Query query("abc", 3);
    assert(query[0] == 'a');
    assert(!query.ends_at(0));
    assert(query[1] == 'b');
    assert(!query.ends_at(1));
    assert(query[2] == 'c');
    assert(!query.ends_at(2));
    assert(query.ends_at(3));

    std::string str("str");
    query.insert(&str);
    assert(str == "abcstr");
  }

  return 0;
}
