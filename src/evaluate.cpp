#include "board.h"
#include "constants.h"
#include <stdint.h>

const int16_t PAWN_VALUE = 100;
const int16_t KNIGHT_VALUE = 300;
const int16_t BISHOP_VALUE = 300;
const int16_t ROOK_VALUE = 500;
const int16_t QUEEN_VALUE = 900;

// clang-format off
const int8_t white_pawn_position_adjustment[N_SQUARES] = {
   0,   0,   0,   0,    0,    0,   0,   0,
   5,   10,  10, -20,  -20,   10,  10,  5,
   5,  -5,  -10,  0,    0,   -10, -5,   5,
   0,   0,   0,   20,   20,   0,   0,   0,
   5,   5,   10,  25,   25,   10,  5,   5,
   10,  10,  20,  30,   30,   20,  10,  10,
   50,  50,  50,  50,   50,   50,  50,  50,
   0,   0,   0,   0,    0,    0,   0,   0,
};

const int8_t white_knight_position_adjustment[N_SQUARES] = {
  -50, -40, -30, -30, -30, -30, -40, -50,
  -40, -20,  0,   5,   5,   0,  -20, -40,
  -30,  5,   10,  15,  15,  10,  5,  -30,
  -30,  0,   15,  20,  20,  15,  0,  -30, 
  -30,  5,   15,  20,  20,  15,  5,  -30, 
  -30,  0,   10,  15,  15,  10,  0,  -30,
  -40, -20,  0,   0,   0,   0,  -20, -40,
  -50, -40, -30, -30, -30, -30, -40, -50,
};

const int8_t white_rook_position_adjustment[N_SQUARES] = {
   0,   0,   5,   10,  10,  5,   0,   0,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
   5,   10,  10,  10,  10,  10,  10,  5,
   0,   0,   0,   0,   0,   0,   0,   0,
};

const int8_t white_bishop_position_adjustment[N_SQUARES] = {
  -20, -10, -10, -10, -10, -10, -10, -20,
  -10,  5,   0,   0,   0,   0,   5,  -10,
  -10,  10,  10,  10,  10,  10,  10, -10,
  -10,  0,   10,  10,  10,  10,  0,  -10,
  -10,  5,   5,   10,  10,  5,   5,  -10, 
  -10,  0,   5,   10,  10,  5,   0,  -10,
  -10,  0,   0,   0,   0,   0,   0,  -10,
  -20, -10, -10, -10, -10, -10, -10, -20,
};

const int8_t white_queen_position_adjustment[N_SQUARES] = {
  -20, -10, -10, -5,  -5,  -10, -10, -20
  -10,  0,   5,   0,   0,   0,   0,  -10, 
  -10,  5,   5,   5,   5,   5,   0,  -10, 
   0,   0,   5,   5,   5,   5,   0,  -5,  
  -5,   0,   5,   5,   5,   5,   0,  -5,
  -10,  0,   5,   5,   5,   5,   0,  -10,
  -10,  0,   0,   0,   0,   0,   0,  -10, 
  -20, -10, -10, -5,  -5,  -10, -10, -20,
};

const int8_t white_king_position_adjustment[N_SQUARES] = {
   20,  30,  10,  0,   0,   10,  30,  20,
   20,  20,  0,   0,   0,   0,   20,  20, 
  -10, -20, -20, -20, -20, -20, -20, -10, 
  -20, -30, -30, -40, -40, -30, -30, -20,
  -30, -40, -40, -50, -50, -40, -40, -30, 
  -30, -40, -40, -50, -50, -40, -40, -30, 
  -30, -40, -40, -50, -50, -40, -40, -30, 
  -30, -40, -40, -50, -50, -40, -40, -30,
};
// clang-format on

int8_t black_pawn_position_adjustment[N_SQUARES];
int8_t black_knight_position_adjustment[N_SQUARES];
int8_t black_rook_position_adjustment[N_SQUARES];
int8_t black_bishop_position_adjustment[N_SQUARES];
int8_t black_queen_position_adjustment[N_SQUARES];
int8_t black_king_position_adjustment[N_SQUARES];

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

/** Returns a score value of the respective material and the position on the
 *  board.
 *
 * @param bitboard: Piece type bitboard.
 * @param position_adjustment: Array of positional bonuses/penalties.
 * @param material_value: Material value of the piece type.
 * @return Score value.
 */
int16_t getIndividualBitboardScoreMaterialAndPosition(
    uint64_t bitboard, const int8_t position_adjustment[64],
    const uint16_t material_value) {
  int16_t counter = 0;
  while (bitboard) {
    counter += material_value;
    counter += position_adjustment[getSetBit(getLowestSetBitValue(bitboard))];
    clearLowestSetBit(bitboard);
  }
  return counter;
}

/** Returns a total score based on all the material and the positional bonuses.
 *
 * @param game_state: Game state.
 * @return Score value.
 */
int16_t getScoreMaterialAndPosition(GameState game_state) {
  int16_t w_counter = 0;
  w_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.pawn, white_pawn_position_adjustment, PAWN_VALUE);
  w_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.knight, white_knight_position_adjustment, KNIGHT_VALUE);
  w_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.bishop, white_bishop_position_adjustment, BISHOP_VALUE);
  w_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.rook, white_rook_position_adjustment, ROOK_VALUE);
  w_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.queen, white_queen_position_adjustment, QUEEN_VALUE);
  w_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.white.king, white_king_position_adjustment, 0);

  int16_t b_counter = 0;
  b_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.black.pawn, black_pawn_position_adjustment, PAWN_VALUE);
  b_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.black.knight, black_knight_position_adjustment, KNIGHT_VALUE);
  b_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.black.bishop, black_bishop_position_adjustment, BISHOP_VALUE);
  b_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.black.rook, black_rook_position_adjustment, ROOK_VALUE);
  b_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.black.queen, black_queen_position_adjustment, QUEEN_VALUE);
  b_counter += getIndividualBitboardScoreMaterialAndPosition(
      game_state.black.king, black_king_position_adjustment, 0);

  return w_counter - b_counter;
}

int16_t evaluatePosition(const GameState game_state) {
  return getScoreMaterialAndPosition(game_state);
}