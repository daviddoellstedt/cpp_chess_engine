#pragma once

#include "board.h"
#include "constants.h"
#include "helper_functions.h"
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

// struct Move {

//   uint16_t data = 0;
// };

void generate_board(std::string name, uint8_t diff);

void perft(uint32_t &nodes, GameState &gamestate, uint8_t depth,
           uint8_t orig_depth, bool total);

void fenToGameState(const std::string fen, GameState &gamestate);
