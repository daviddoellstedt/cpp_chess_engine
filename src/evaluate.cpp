#include "board.h"
#include "constants.h"
#include <stdint.h>

void initializePositionTables(void) {
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    black_pawn_position_adjustment[bit] =
        white_pawn_position_adjustment[63 - bit];
    black_knight_position_adjustment[bit] =
        white_knight_position_adjustment[63 - bit];
    black_bishop_position_adjustment[bit] =
        white_bishop_position_adjustment[63 - bit];
    black_rook_position_adjustment[bit] =
        white_rook_position_adjustment[63 - bit];
    black_queen_position_adjustment[bit] =
        white_queen_position_adjustment[63 - bit];
    black_king_position_adjustment[bit] =
        white_king_position_adjustment[63 - bit];
  }
}

// TODO documentation
int16_t getIndividualBitboardScoreMaterialAndPosition(
    uint64_t bitboard_w, uint64_t bitboard_b,
    const int8_t position_adjustment[64], const uint16_t material_value) {
  int16_t counter = 0;
  while (bitboard_w) {
    counter += material_value;
    counter += position_adjustment[getSetBit(getLowestSetBitValue(bitboard_w))];
    clearLowestSetBit(bitboard_w);
  }

  while (bitboard_b) {
    counter -= material_value;
    counter -=
        position_adjustment[63 - getSetBit(getLowestSetBitValue(bitboard_b))];
    clearLowestSetBit(bitboard_b);
  }
  return counter;
}

// TODO documentation
int16_t getScoreMaterialAndPosition(GameState game_state) {
  int16_t counter = 0;
  counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.pawn, game_state.white.pawn,
      white_pawn_position_adjustment, PAWN_VALUE);
  counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.knight, game_state.white.knight,
      white_knight_position_adjustment, KNIGHT_VALUE);
  counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.bishop, game_state.white.bishop,
      white_bishop_position_adjustment, BISHOP_VALUE);
  counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.rook, game_state.white.rook,
      white_rook_position_adjustment, ROOK_VALUE);
  counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.queen, game_state.white.queen,
      white_queen_position_adjustment, QUEEN_VALUE);
  counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.king, game_state.white.king,
      white_king_position_adjustment, 0);
  return counter;
}

int16_t evaluatePosition(const GameState game_state) {
  return getScoreMaterialAndPosition(game_state);
}