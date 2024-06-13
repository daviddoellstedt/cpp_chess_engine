#include "../test/unit_tests.h"
#include "bit_fns.h"
#include "move_generator.h"
#include "uci.h"
#include <iostream>
#include <string>

int main() {
  initializeMagicBitboardTables();
  // testAllPerft();
  UCIStart();
  // generate_board("ba", 3);
  return 0;
}
