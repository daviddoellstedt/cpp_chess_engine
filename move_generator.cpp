#include "move_generator.h"
#include "constants.h"
#include "helper_functions.h"

#include <iostream>
#include <stdint.h>

uint64_t bishopMagicTable[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS] = {0};
uint64_t rookMagicTable[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS] = {0};

/** Gets the horizontal moves a slider piece can make, using setwise operations.
 * https://www.chessprogramming.org/General_Setwise_Operations.
 *
 * @param piece: Horizontal sliding piece bitboard. Only one bit should be set.
 * @param occupied: Bitboard of occupied spaces on the board.
 * @return Bitboard of horizontal moves.
 */
uint64_t getSetwiseHorizontalMoves(uint64_t piece, uint64_t occupied) {
  uint64_t horiz_moves = (((occupied)-2 * piece) ^
                          reverse(reverse(occupied) - 2 * reverse(piece))) &
                         directional_mask[getSetBit(piece)][RANKS];
  return horiz_moves;
}

/** Gets the vertical moves a slider piece can make, using setwise operations.
 * https://www.chessprogramming.org/General_Setwise_Operations.
 *
 * @param piece: Vertical sliding piece bitboard. Only one bit should be set.
 * @param occupied: Bitboard of occupied spaces on the board.
 * @return Bitboard of vertical moves.
 */
uint64_t getSetwiseVerticalMoves(uint64_t piece, uint64_t occupied) {
  uint8_t sl_bit = getSetBit(piece);
  uint64_t vert_moves =
      (((occupied & directional_mask[sl_bit][FILES]) - 2 * piece) ^
       reverse(reverse(occupied & directional_mask[sl_bit][FILES]) -
               2 * reverse(piece))) &
      directional_mask[sl_bit][FILES];
  return vert_moves;
}

uint64_t getSetwiseHorizontalAndVerticalMoves(uint64_t piece, uint64_t occupied,
                                              bool unsafe_calculation,
                                              uint64_t K) {
  /** Used in the case where we need to generate zones for the king that are
   * unsafe. If the king is in the attack zone of a horizontal/vertical slider,
   * we want to remove the king from the occupied zone. Because a move of the
   * king that still lies in the path of attack, is not a valid move.
   */
  if (unsafe_calculation) {
    occupied &= ~K;
  }
  return getSetwiseHorizontalMoves(piece, occupied) |
         getSetwiseVerticalMoves(piece, occupied);
}

/** Gets the down right diagonal moves a slider piece can make, using setwise
 * operations. https://www.chessprogramming.org/General_Setwise_Operations.
 *
 * @param piece: Diagonal sliding piece bitboard. Only one bit should be set.
 * @param occupied: Bitboard of occupied spaces on the board.
 * @return Bitboard of down right diagonal moves.
 */
uint64_t getSetwiseDownRightDiagonalMoves(uint64_t piece, uint64_t occupied) {
  uint8_t sl_bit = getSetBit(piece);
  uint64_t ddr_moves =
      (((occupied & directional_mask[sl_bit][DIAGONALS_DOWN_RIGHT]) -
        2 * piece) ^
       reverse(
           reverse(occupied & directional_mask[sl_bit][DIAGONALS_DOWN_RIGHT]) -
           2 * reverse(piece))) &
      directional_mask[sl_bit][DIAGONALS_DOWN_RIGHT];
  return ddr_moves;
}

/** Gets the up right diagonal moves a slider piece can make, using setwise
 * operations. https://www.chessprogramming.org/General_Setwise_Operations.
 *
 * @param piece: Diagonal sliding piece bitboard. Only one bit should be set.
 * @param occupied: Bitboard of occupied spaces on the board.
 * @return Bitboard of up right diagonal moves.
 */
uint64_t getSetwiseUpRightDiagonalMoves(uint64_t piece, uint64_t occupied) {
  uint8_t sl_bit = getSetBit(piece);
  uint64_t dur_moves =
      (((occupied & directional_mask[sl_bit][DIAGONALS_UP_RIGHT]) - 2 * piece) ^
       reverse(
           reverse(occupied & directional_mask[sl_bit][DIAGONALS_UP_RIGHT]) -
           2 * reverse(piece))) &
      directional_mask[sl_bit][DIAGONALS_UP_RIGHT];
  return dur_moves;
}

uint64_t getSetwiseDiagonalMoves(uint64_t piece, uint64_t occupied,
                                 bool unsafe_calculation, uint64_t K) {

  /** Used in the case where we need to generate zones for the king that are
   * unsafe. If the king is in the attack zone of a horizontal/vertical slider,
   * we want to remove the king from the occupied zone. Because a move of the
   * king that still lies in the path of attack, is not a valid move.
   */
  if (unsafe_calculation) {
    occupied &= ~K;
  }
  return getSetwiseDownRightDiagonalMoves(piece, occupied) |
         getSetwiseUpRightDiagonalMoves(piece, occupied);
}

void generateRookMagicNumber(
    uint8_t bit,
    uint64_t blockers_array[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS]) {
  uint64_t rookMagicTableTemp[N_ROOK_BLOCKERS_PERMUTATIONS] = {0};

  while (true) {
    // Sparse bits produce much better magic number candidates.
    uint64_t magic_num =
        generateRandom64() & generateRandom64() & generateRandom64();

    if (countSetBits((magic_num * rookMagicMasks[bit]) &
                     0xFF00000000000000ull) < 6) {
      continue;
    }

    bool fail = false;
    uint64_t i;
    for (i = 0; i < N_ROOK_BLOCKERS_PERMUTATIONS; i++) {
      uint64_t blockers = blockers_array[bit][i];
      uint64_t magic_product = blockers * magic_num;
      uint16_t index = magic_product >> 52;

      uint64_t h_v_moves =
          getSetwiseHorizontalAndVerticalMoves(1ull << bit, blockers);

      if (rookMagicTableTemp[index] == 0) {
        rookMagicTableTemp[index] = h_v_moves;
        continue;
      }
      // There is already an entry, but we are lucky and it matches, continue;
      if (h_v_moves == rookMagicTableTemp[index]) {
        // std::cout << " HEY HEY HEY" << std::endl;
        continue;
      }
      // Else there is a conflict. Clear the table and try the next magic number
      memset(rookMagicTableTemp, 0,
             N_ROOK_BLOCKERS_PERMUTATIONS * sizeof(uint64_t));
      fail = true;
      break;
    }
    // MAgic number found. Dump to std out.
    if (!fail) {
      std::cout << "0x" << std::hex << std::uppercase << magic_num + 0 << ", "
                << std::endl;
      break;
    }
  }
}

void generateBishopMagicNumber(
    uint8_t bit,
    uint64_t blockers_array[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS]) {
  uint64_t bishopMagicTableTemp[N_BISHOP_BLOCKERS_PERMUTATIONS] = {0};

  while (true) {
    // Sparse bits produce much better magic number candidates.
    uint64_t magic_num =
        generateRandom64() & generateRandom64() & generateRandom64();

    if (countSetBits((magic_num * bishopMagicMasks[bit]) &
                     0xFF00000000000000ull) < 6) {
      continue;
    }

    bool fail = false;
    uint64_t i;
    for (i = 0; i < N_BISHOP_BLOCKERS_PERMUTATIONS; i++) {
      uint64_t blockers = blockers_array[bit][i];
      uint64_t magic_product = blockers * magic_num;
      uint16_t index = magic_product >> (64 - N_BISHOP_BLOCKERS);
      uint64_t diag_moves = getSetwiseDiagonalMoves(1ull << bit, blockers);

      if (bishopMagicTableTemp[index] == 0) {
        bishopMagicTableTemp[index] = diag_moves;
        continue;
      }
      // There is already an entry, but we are lucky and it matches, continue;
      if (diag_moves == bishopMagicTableTemp[index]) {
        continue;
      }
      // Else there is a conflict. Clear the table and try the next magic number
      memset(bishopMagicTableTemp, 0,
             N_BISHOP_BLOCKERS_PERMUTATIONS * sizeof(uint64_t));
      fail = true;
      break;
    }
    // Magic number found. Dump to std out.
    if (!fail) {
      std::cout << "0x" << std::hex << std::uppercase << magic_num + 0 << ", "
                << std::endl;
      break;
    }
  }
}

/** Populates the global magic rook table used for generating
 * horizontal/vertical moves.
 *
 * @param blockers_array: Rook blockers array.
 */
void initializeRookMagicTable(
    uint64_t blockers_array[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS]) {
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    uint64_t magic_number = rookMagicNumbers[bit];
    for (uint16_t blockers_index = 0;
         blockers_index < N_ROOK_BLOCKERS_PERMUTATIONS; blockers_index++) {
      uint64_t blockers_bitboard = blockers_array[bit][blockers_index];
      rookMagicTable[bit][(magic_number * blockers_bitboard) >>
                          (64 - N_ROOK_BLOCKERS)] =
          getSetwiseHorizontalAndVerticalMoves(1ULL << bit, blockers_bitboard);
    }
  }
}

/** Populates the global magic bishop table used for generating diagonal moves.
 *
 * @param blockers_array: Bishop blockers array.
 */
void initializeBishopMagicTable(
    uint64_t blockers_array[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS]) {
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    uint64_t magic_number = bishopMagicNumbers[bit];
    for (uint16_t blockers_index = 0;
         blockers_index < N_BISHOP_BLOCKERS_PERMUTATIONS; blockers_index++) {
      uint64_t blockers_bitboard = blockers_array[bit][blockers_index];
      bishopMagicTable[bit][(magic_number * blockers_bitboard) >>
                            (64 - N_BISHOP_BLOCKERS)] =
          getSetwiseDiagonalMoves(1ULL << bit, blockers_bitboard);
    }
  }
}

/** Generates a blockers bitboard from the respective blocker index.
 *
 * @param blocker_index: Blocker index. Each set bit represents a blocker.
 * @param all_blockers: Bitboard of all possible blockers.
 * @return Bitboard of blockers.
 */
uint64_t generateBlockersBitboardFromIndex(uint16_t blocker_index,
                                           uint64_t all_blockers) {
  uint64_t blockers_bitboard = all_blockers;
  uint8_t i_blocker = 0;
  while (all_blockers) {
    uint64_t blocker_bb = getLowestSetBitValue(all_blockers);
    uint8_t blocker_bit = getSetBit(blocker_bb);

    // Check if we need to clear the blocker bit.
    if (!(blocker_index & (1 << i_blocker))) {
      blockers_bitboard &= ~(1ull << blocker_bit);
    }
    clearLowestSetBit(all_blockers);
    i_blocker++;
  }
  return blockers_bitboard;
}

/** Initializes the rook blockers array.
 *
 * @param blockers_array: Rook blockers array.
 */
void initializeRookBlockers(
    uint64_t blockers_array[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS]) {
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    for (uint16_t blocker_index = 0;
         blocker_index < N_ROOK_BLOCKERS_PERMUTATIONS; blocker_index++) {
      uint64_t blockers_bitboard =
          generateBlockersBitboardFromIndex(blocker_index, rookMagicMasks[bit]);
      blockers_array[bit][blocker_index] = blockers_bitboard;
    }
  }
}

/** Initializes the bishop blockers array.
 *
 * @param blockers_array: Bishop blockers array.
 */
void initializeBishopBlockers(
    uint64_t blockers_array[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS]) {
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    for (uint16_t blocker_index = 0;
         blocker_index < N_BISHOP_BLOCKERS_PERMUTATIONS; blocker_index++) {
      uint64_t blockers_bitboard = generateBlockersBitboardFromIndex(
          blocker_index, bishopMagicMasks[bit]);
      blockers_array[bit][blocker_index] = blockers_bitboard;
    }
  }
}

/** Initializes the bishop magic bitboard table.
 *
 * @param rook_blockers: Rook blockers table.
 */
void initializeBishopMagicBitboardTable(void) {
  uint64_t bishop_blockers[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS];
  initializeBishopBlockers(bishop_blockers);
  initializeBishopMagicTable(bishop_blockers);
}

/** Initializes the rook magic bitboard table.
 *
 * @param rook_blockers: Rook blockers table.
 */
void initializeRookMagicBitboardTable(void) {
  uint64_t rook_blockers[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS];
  initializeRookBlockers(rook_blockers);
  initializeRookMagicTable(rook_blockers);
}

void initializeMagicBitboardTables(void) {
  initializeBishopMagicBitboardTable();
  initializeRookMagicBitboardTable();
}

// Add documentation
uint64_t h_v_moves(uint64_t piece, uint64_t OCCUPIED, bool unsafe_calc,
                   uint64_t K) {
  if (unsafe_calc) {
    OCCUPIED &= ~K;
  }
  uint8_t piece_bit = getSetBit(piece);
  uint64_t blockers = OCCUPIED &= rookMagicMasks[piece_bit];
  uint64_t magic_moves =
      rookMagicTable[piece_bit][(blockers * rookMagicNumbers[piece_bit]) >>
                                (64 - N_ROOK_BLOCKERS)];
  return magic_moves;
}

// Add documentation
uint64_t diag_moves(uint64_t piece, uint64_t OCCUPIED, bool unsafe_calc,
                    uint64_t K) {
  if (unsafe_calc) {
    OCCUPIED &= ~K;
  }
  uint8_t piece_bit = getSetBit(piece);
  uint64_t blockers = OCCUPIED &= bishopMagicMasks[piece_bit];
  uint64_t magic_moves =
      bishopMagicTable[piece_bit][(blockers * bishopMagicNumbers[piece_bit]) >>
                                  (64 - N_BISHOP_BLOCKERS)];
  return magic_moves;
}
