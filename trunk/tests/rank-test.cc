#include <cassert>

#include <marisa/rank.h>

int main() {
  marisa::Rank rank;

  assert(rank.abs() == 0);
  assert(rank.rel1() == 0);
  assert(rank.rel2() == 0);
  assert(rank.rel3() == 0);
  assert(rank.rel4() == 0);
  assert(rank.rel5() == 0);
  assert(rank.rel6() == 0);
  assert(rank.rel7() == 0);

  rank.set_abs(0xFFFFFFFFU);
  rank.set_rel1(64);
  rank.set_rel2(128);
  rank.set_rel3(192);
  rank.set_rel4(256);
  rank.set_rel5(320);
  rank.set_rel6(384);
  rank.set_rel7(448);

  assert(rank.abs() == 0xFFFFFFFFU);
  assert(rank.rel1() == 64);
  assert(rank.rel2() == 128);
  assert(rank.rel3() == 192);
  assert(rank.rel4() == 256);
  assert(rank.rel5() == 320);
  assert(rank.rel6() == 384);
  assert(rank.rel7() == 448);

  return 0;
}
