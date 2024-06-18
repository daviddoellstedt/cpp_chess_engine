#include "../test/test.h"
#include "evaluate.h"
#include "move_generator.h"
#include "uci.h"

int main() {
  initializeMagicBitboardTables();
  initializePositionTables();
  // testAllPerft();
  UCIStart();
  return 0;
}
