#include "bit_fns.h"
#include "move_generator.h"
#include "../test/unit_tests.h"
#include <iostream>
#include <string>

int main() {
  initializeMagicBitboardTables();
  testAllPerft();
  // generate_board("ba", 3);
  return 0;
}
