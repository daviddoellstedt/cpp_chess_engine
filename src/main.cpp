#include "../test/test.h"
#include "evaluate.h"
#include "move_generator.h"
#include "uci.h"
#include <iostream>
#include <string>

int main() {
  initializeMagicBitboardTables();
  initializePositionTables();
  testAllPerft();
  UCIStart();
  return 0;
}
