#pragma once

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

/** Negamax algorithm, finds the best possible move for the active player.
 *
 * @param game_state: Game state.
 * @param depth: Depth to search the game tree.
 * @param color: 1 for white, -1 for black.
 * @param alpha: A/B pruning parameter, leave default.
 * @param beta: A/B pruning parameter, leave default.
 * @return Negamax tuple of the best move and score.
 */
NegamaxTuple negamax(GameState game_state, uint8_t depth, int8_t color,
                     int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX);
