#include "search.h"
#include "board.h"
#include "constants.h"
#include "evaluate.h"
#include "move_generator.h"

NegamaxTuple negamax(GameState game_state, uint8_t depth, int8_t color,
                     int16_t alpha, int16_t beta) {
  // Terminal Node.
  if (depth == 0) {
    return NegamaxTuple(Move(), evaluatePosition(game_state) * color, 1);
  }

  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(game_state, moves, check);

  // Terminal node, Checkmate/Stalemate.
  if (n_moves == 0) {
    return NegamaxTuple(Move(), check ? -INT16_MAX : 0, 1);
  }

  NegamaxTuple node_max = NegamaxTuple(Move(), INT16_MIN, 1);

  for (uint8_t i = 0; i < n_moves; i++) {
    GameState game_state_temp;
    memcpy(&game_state_temp, &game_state, sizeof(GameState));
    applyMove(moves[i], game_state_temp);
    NegamaxTuple node_temp =
        negamax(game_state_temp, depth - 1, -color, -beta, -alpha);
    node_temp.score *= -1;
    node_max.nodes_searched += node_temp.nodes_searched;

    if (node_temp.score > node_max.score) {
      node_max.score = node_temp.score;
      node_max.move = moves[i];
    }

    alpha = std::max(alpha, node_temp.score);
    if (alpha >= beta) {
      return NegamaxTuple(node_max.move, alpha, 1);
    }
  }
  return node_max;
}
