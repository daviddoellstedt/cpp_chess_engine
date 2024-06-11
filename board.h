#pragma once

#include "move.h"
#include <stdint.h>
#include <string>

class ColorState {
public:
  uint64_t rook = 0;
  uint64_t knight = 0;
  uint64_t bishop = 0;
  uint64_t queen = 0;
  uint64_t king = 0;
  uint64_t pawn = 0;
  bool can_king_side_castle = false;
  bool can_queen_side_castle = false;

  uint64_t getOccupiedBitboard(void) {
    return pawn | rook | knight | bishop | queen | king;
  }
};

class GameState {
public:
  // TODO: add total moves.
  // TODO: add half moves.

  // State of the white pieces.
  ColorState white;

  // State of the black pieces.
  ColorState black;

  // If true -> white's turn, false -> black's turn.
  bool whites_turn = true;

  uint64_t en_passant = 0;
  uint64_t getWhiteOccupiedBitboard(void) {
    return white.getOccupiedBitboard();
  }
  uint64_t getBlackOccupiedBitboard(void) {
    return black.getOccupiedBitboard();
  }
};

/** Prints board to std out.
 *
 * @param game_state: Game state.
 */
void printBoard(const GameState &game_state);

// TODO ADD DOCUMENTATION.
void fenToGameState(const std::string fen, GameState &game_state);

// TODO ADD DOCUMENTAITON
void applyMove(Move move, GameState &game_state);
