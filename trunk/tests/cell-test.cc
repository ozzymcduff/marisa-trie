#include <cassert>

#include <marisa/cell.h>

int main() {
  marisa::Cell cell;

  assert(cell.louds_pos() == 0);
  assert(cell.node() == 0);
  assert(cell.key_id() == 0);
  assert(cell.length() == 0);

  cell.set_louds_pos(1);
  cell.set_node(2);
  cell.set_key_id(3);
  cell.set_length(4);

  assert(cell.louds_pos() == 1);
  assert(cell.node() == 2);
  assert(cell.key_id() == 3);
  assert(cell.length() == 4);

  return 0;
}
