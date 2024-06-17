#include "board.h"
#include <stdint.h>

struct NegamaxTuple {
  Move move;
  int16_t score = 0;
  uint32_t nodes_searched = 0;
  NegamaxTuple() {}
  NegamaxTuple(Move move, int16_t score, uint32_t nodes_searched)
      : move(move), score(score), nodes_searched(nodes_searched) {}
};

// TODO DOCUMENTAITON.
NegamaxTuple negamax(GameState game_state, uint8_t depth, int8_t color,
                     int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX);
