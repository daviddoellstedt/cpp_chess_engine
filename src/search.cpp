#include "search.h"
#include "board.h"
#include "evaluate.h"
#include "move_generator.h"

negamaxTuple negamax(GameState game_state, uint8_t depth, int8_t color,
                     int16_t alpha, int16_t beta) {
  negamaxTuple node_max;
  node_max.value = INT16_MIN;
  node_max.nodes_searched++;

  // Terminal Node.
  if (depth == 0) {
    Move leaf_move;
    negamaxTuple leaf = {leaf_move,
                         (int16_t)(evaluatePosition(game_state) * color), 1};
    return leaf;
  }

  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(game_state, moves, check);

  // Terminal node, Checkmate/Stalemate.
  if (n_moves == 0) {
    negamaxTuple leaf_node = {Move(), (int16_t)(check ? INT16_MAX * -color : 0),
                              1};
    return leaf_node;
  }

  for (uint8_t i = 0; i < n_moves; i++) {
    GameState game_state_temp;
    memcpy(&game_state_temp, &game_state, sizeof(GameState));
    applyMove(moves[i], game_state_temp);
    negamaxTuple node_temp =
        negamax(game_state_temp, depth - 1, -color, -beta, -alpha);
    node_temp.value *= -color;
    node_max.nodes_searched += node_temp.nodes_searched;

    if (node_temp.value > node_max.value) {
      node_max.value = node_temp.value;
      node_max.move = moves[i];
    }

    alpha = std::max(alpha, node_max.value);
    if (false or alpha >= beta) {
      break;
    }
  }
  return node_max;
}
