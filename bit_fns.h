#pragma once

#include "board.h"
#include <stdint.h>
#include <string>

enum SpecialMove : uint8_t {
  NONE = 0,
  CASTLE_KINGSIDE = 1,
  CASTLE_QUEENSIDE = 2,
  PROMOTION_QUEEN = 3,
  PROMOTION_ROOK = 4,
  PROMOTION_KNIGHT = 5,
  PROMOTION_BISHOP = 6,
  EN_PASSANT = 7,
  PAWN_PUSH_2 = 8,
};

struct Move {
  // Moves are stored as a 16-bit integer. Kept as lightweight as possible to
  // allow deeper search. Bits: 0 - 2:   Initial x position 3 - 5:   Initial y
  // position 6 - 8:   Final x position 9 - 11:  Final y position 12 - 15:
  // Special move flags (see SpecialMove enum)
  uint16_t data = 0;
};

struct MoveGameStateScore {
  Move move;
  GameState gamestate;
  int16_t score = 0;

  // Overloaded comparator. Used for sorting in descending order.
  bool operator<(const MoveGameStateScore &move_gamestate_score) const {
    return score > move_gamestate_score.score;
  }
};

void generate_board(std::string name, uint8_t diff);

void perft(uint32_t &nodes, GameState &gamestate, uint8_t depth,
           uint8_t orig_depth, bool total);

void fenToGameState(const std::string fen, GameState &gamestate);
