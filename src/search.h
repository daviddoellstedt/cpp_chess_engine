#include "board.h"
#include <stdint.h>

struct negamaxTuple {
  Move move;
  int16_t value = 0;
  uint32_t nodes_searched = 0;
};

// TODO DOCUMENTAITON.
negamaxTuple negamax(GameState game_state, uint8_t depth, int8_t color = 1,
                     int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX);
