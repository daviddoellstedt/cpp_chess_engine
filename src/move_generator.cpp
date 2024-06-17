#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include "move.h"

#include <iostream>
#include <stdint.h>

uint64_t bishopMagicTable[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS] = {0};
uint64_t rookMagicTable[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS] = {0};

/** Returns a bitboard of the line between two pieces.
 *
 * @param p1: First piece
 * @param p2: Second piece
 * @return Bitboard mask of rank/file/diagonal connection between the two
 * pieces.
 */
uint64_t getColinearMask(uint64_t p1, uint64_t p2) {
  uint8_t k_bit = getSetBit(p2);
  uint8_t p_bit = getSetBit(p1);

  for (uint8_t dir = 0; dir < N_DIRECTIONS; dir++) {
    if (directional_mask[k_bit][dir] == directional_mask[p_bit][dir]) {
      return directional_mask[k_bit][dir];
    }
  }
  logErrorAndExit("ERROR: The two arguments provided are not colinear.");
  return 0;
}

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

/** Gets the horizontal and vertical moves a sliding piece can make, using
 * setwise operations.
 *
 * @param piece: Vertical/horizontal sliding piece bitboard. Only one bit should
 * be set.
 * @param occupied: Bitboard of occupied spaces on the board.
 * @param unsafe_calculation: Flag used to see if we need to remove the enemy
 * king from the occupied bitboard (see inline comment for more details).
 * @param K: Bitboard representing king location (see inline comment for more
 * details).
 * @return Bitboard of horizontal/vertical moves.
 */
uint64_t getSetwiseHorizontalAndVerticalMoves(uint64_t piece, uint64_t occupied,
                                              bool unsafe_calculation = false,
                                              uint64_t K = 0) {
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

/** Gets the diagonal moves a sliding piece can make, using setwise operations.
 *
 * @param piece: Diaganol sliding piece bitboard. Only one bit should be set.
 * @param occupied: Bitboard of occupied spaces on the board.
 * @param unsafe_calculation: Flag used to see if we need to remove the enemy
 * king from the occupied bitboard (see inline comment for more details).
 * @param K: Bitboard representing king location (see inline comment for more
 * details).
 * @return Bitboard of diagonal moves.
 */
uint64_t getSetwiseDiagonalMoves(uint64_t piece, uint64_t occupied,
                                 bool unsafe_calculation = false,
                                 uint64_t K = 0) {

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

/** Finds a rook magic number for a specific bit.
 *
 * @param bit: Square/bit on the board to do the search for.
 * @param blockers_array: Rook blockers array.
 * @return Magic number, if found, else 0.
 */
uint64_t generateRookMagicNumber(
    uint8_t bit,
    uint64_t blockers_array[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS]) {
  uint64_t rookMagicTableTemp[N_ROOK_BLOCKERS_PERMUTATIONS] = {0};

  while (true) {
    // Sparse bits produce much better magic number candidates.
    uint64_t magic_num =
        generateRandom64() & generateRandom64() & generateRandom64();

    if (countSetBits((magic_num * rook_magic_masks[bit]) &
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
    if (!fail) {
      return magic_num;
    }
  }
  return 0;
}

/** Finds a bishop magic number for a specific bit.
 *
 * @param bit: Square/bit on the board to do the search for.
 * @param blockers_array: Bishop blockers array.
 * @return Magic number, if found, else 0.
 */
uint64_t generateBishopMagicNumber(
    uint8_t bit,
    uint64_t blockers_array[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS]) {
  // TODO: Refator. A lot fo similar code with the rook function above.
  uint64_t bishopMagicTableTemp[N_BISHOP_BLOCKERS_PERMUTATIONS] = {0};

  while (true) {
    // Sparse bits produce much better magic number candidates.
    uint64_t magic_num =
        generateRandom64() & generateRandom64() & generateRandom64();

    if (countSetBits((magic_num * bishop_magic_masks[bit]) &
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
    if (!fail) {
      return magic_num;
    }
  }
  return 0;
}

/** Populates the global magic rook table used for generating
 * horizontal/vertical moves.
 *
 * @param blockers_array: Rook blockers array.
 */
void initializeRookMagicTable(
    uint64_t blockers_array[N_SQUARES][N_ROOK_BLOCKERS_PERMUTATIONS]) {
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    uint64_t magic_number = rook_magic_numbers[bit];
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
    uint64_t magic_number = bishop_magic_numbers[bit];
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
      uint64_t blockers_bitboard = generateBlockersBitboardFromIndex(
          blocker_index, rook_magic_masks[bit]);
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
          blocker_index, bishop_magic_masks[bit]);
      blockers_array[bit][blocker_index] = blockers_bitboard;
    }
  }
}

/** Initializes the bishop magic bitboard table.
 */
void initializeBishopMagicBitboardTable(void) {
  uint64_t bishop_blockers[N_SQUARES][N_BISHOP_BLOCKERS_PERMUTATIONS];
  initializeBishopBlockers(bishop_blockers);
  initializeBishopMagicTable(bishop_blockers);
}

/** Initializes the rook magic bitboard table.
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

/** Generates horizontal/vertical moves. For pieces such as rook/queen.
 *
 * @param piece: Bitboard slider piece.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param unsafe_calculation: Flag denoting if we are caluculating safe paths
 * for king. (Optional).
 * @param K: Bitboard of the active player's king. (Optional).
 */
uint64_t horizontalAndVerticalMoves(uint64_t piece, uint64_t OCCUPIED,
                                    bool unsafe_calculation = false,
                                    uint64_t K = 0) {
  if (unsafe_calculation) {
    OCCUPIED &= ~K;
  }
  uint8_t piece_bit = getSetBit(piece);
  uint64_t blockers = OCCUPIED &= rook_magic_masks[piece_bit];
  uint64_t magic_moves =
      rookMagicTable[piece_bit][(blockers * rook_magic_numbers[piece_bit]) >>
                                (64 - N_ROOK_BLOCKERS)];
  return magic_moves;
}

/** Generates diagonal moves. For pieces such as bishop/queen.
 *
 * @param piece: Bitboard slider piece.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param unsafe_calculation: Flag denoting if we are caluculating safe paths
 * for king. (Optional).
 * @param K: Bitboard of the active player's king. (Optional).
 */
uint64_t diagonalMoves(uint64_t piece, uint64_t OCCUPIED,
                       bool unsafe_calculation = false, uint64_t K = 0) {
  if (unsafe_calculation) {
    OCCUPIED &= ~K;
  }
  uint8_t piece_bit = getSetBit(piece);
  uint64_t blockers = OCCUPIED &= bishop_magic_masks[piece_bit];
  uint64_t magic_moves =
      bishopMagicTable[piece_bit]
                      [(blockers * bishop_magic_numbers[piece_bit]) >>
                       (64 - N_BISHOP_BLOCKERS)];
  return magic_moves;
}

/** Generates and adds rook moves to the move list.
 *
 * @param R: Bitboard of the active player's rooks.
 * @param K: Bitboard of the active player's king.
 * @param PIECES: Bitboard of the active player's pieces.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateRookMoves(uint64_t R, uint64_t K, uint64_t PIECES,
                       uint64_t OCCUPIED, uint64_t PINNED,
                       uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (R) {

    uint64_t bb = getLowestSetBitValue(R);
    uint8_t bit = getSetBit(bb);

    uint64_t mask = bb & PINNED ? getColinearMask(bb, K) : FILLED;

    uint64_t possible_moves = horizontalAndVerticalMoves(bb, OCCUPIED) &
                              ~PIECES & mask & checker_zone;
    while (possible_moves) {
      uint64_t final_bb = getLowestSetBitValue(possible_moves);
      uint8_t final_bit = getSetBit(final_bb);
      moves[n_moves++] =
          Move(bitToX(bit), bitToY(bit), bitToX(final_bit), bitToY(final_bit));
      clearLowestSetBit(possible_moves);
    }
    clearLowestSetBit(R);
  }
}

/** Generates and adds bishop moves to the move list.
 *
 * @param B: Bitboard of the active player's bishops.
 * @param K: Bitboard of the active player's king.
 * @param PIECES: Bitboard of the active player's pieces.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateBishopMoves(uint64_t B, uint64_t K, uint64_t PIECES,
                         uint64_t OCCUPIED, uint64_t PINNED,
                         uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (B) {
    uint64_t bb = getLowestSetBitValue(B);
    uint8_t bit = getSetBit(bb);
    uint64_t mask = bb & PINNED ? getColinearMask(bb, K) : FILLED;
    uint64_t possible_moves =
        diagonalMoves(bb, OCCUPIED) & ~PIECES & mask & checker_zone;

    while (possible_moves) {
      uint64_t bb_final = getLowestSetBitValue(possible_moves);
      uint8_t final_bit = getSetBit(bb_final);
      moves[n_moves++] =
          Move(bitToX(bit), bitToY(bit), bitToX(final_bit), bitToY(final_bit));

      clearLowestSetBit(possible_moves);
    }
    clearLowestSetBit(B);
  }
}

/** Generates and adds queen moves to the move list.
 *
 * @param Q: Bitboard of the active player's queens.
 * @param K: Bitboard of the active player's king.
 * @param PIECES: Bitboard of the active player's pieces.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateQueenMoves(uint64_t Q, uint64_t K, uint64_t PIECES,
                        uint64_t OCCUPIED, uint64_t PINNED,
                        uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (Q) {
    uint64_t bb = getLowestSetBitValue(Q);
    uint8_t bit = getSetBit(bb);
    uint64_t mask = bb & PINNED ? getColinearMask(bb, K) : FILLED;
    uint64_t possible_moves = (horizontalAndVerticalMoves(bb, OCCUPIED) |
                               diagonalMoves(bb, OCCUPIED)) &
                              ~PIECES & mask & checker_zone;

    while (possible_moves) {
      uint64_t bb_final = getLowestSetBitValue(possible_moves);
      uint8_t final_bit = getSetBit(bb_final);
      moves[n_moves++] =
          Move(bitToX(bit), bitToY(bit), bitToX(final_bit), bitToY(final_bit));

      clearLowestSetBit(possible_moves);
    }
    clearLowestSetBit(Q);
  }
}

/** Generates and adds knight moves to the move list.
 *
 * @param N: Bitboard of the active player's knights.
 * @param PIECES: Bitboard of the active player's pieces.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateKnightMoves(uint64_t N, uint64_t PIECES, uint64_t PINNED,
                         uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (N) {
    uint64_t bb = getLowestSetBitValue(N);
    uint8_t kn_bit = getSetBit(bb);

    if (!(bb & PINNED)) { // only check for moves if it's not pinned.
                          // pinned knights cannot move.
      uint64_t pos_moves = knight_moves[kn_bit] & ~PIECES & checker_zone;
      while (pos_moves) {
        uint64_t bb_final = getLowestSetBitValue(pos_moves);
        uint8_t final_bit = getSetBit(bb_final);
        moves[n_moves++] = Move(bitToX(kn_bit), bitToY(kn_bit),
                                bitToX(final_bit), bitToY(final_bit));

        clearLowestSetBit(pos_moves);
      }
    }
    clearLowestSetBit(N);
  }
}

/** Generates and adds king moves to the move list.
 *
 * @param K: Bitboard of the active player's king.
 * @param PIECES: Bitboard of the active player's pieces.
 * @param DZ: Bitboard of the danger zone, where the king cannot pass through.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateKingMoves(uint64_t K, uint64_t PIECES, uint64_t DZ, Move *moves,
                       uint8_t &n_moves) {
  uint8_t k_bit = getSetBit(K);
  uint64_t pos_moves = king_moves[k_bit] & ~PIECES & ~DZ;
  while (pos_moves) {
    uint64_t bb_final = getLowestSetBitValue(pos_moves);
    uint8_t final_bit = getSetBit(bb_final);
    moves[n_moves++] = Move(bitToX(k_bit), bitToY(k_bit), bitToX(final_bit),
                            bitToY(final_bit));
    clearLowestSetBit(pos_moves);
  }
}

/** Gets the bitboard of the squares the pawns are attacking.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param P: Bitboard of the pawns.
 * @return Bitboard of the squares the pawns are attacking.
 */
uint64_t getPawnAttackZone(bool white_to_move, uint64_t P) {
  return white_to_move ? ((P >> 7) & ~file_a) | ((P >> 9) & ~file_h)
                       : ((P << 9) & ~file_a) | ((P << 7) & ~file_h);
}

/** Gets the bitboard of the squares the rooks/queens are attacking
 * horizontally/vertically.
 *
 * @param K: Bitboard of the active player's king.
 * @param ER: Bitboard of the enemy player's rooks.
 * @param EQ: Bitboard of the enemy player's queens.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @return Bitboard of the squares the rooks/queens are attacking diagonally.
 */
uint64_t getRookQueenAttackZone(uint64_t K, uint64_t ER, uint64_t EQ,
                                uint64_t OCCUPIED) {
  uint64_t EHV = ER | EQ;
  uint64_t DZ = 0;
  while (EHV) {
    uint64_t hv_piece = getLowestSetBitValue(EHV);
    DZ |= horizontalAndVerticalMoves(hv_piece, OCCUPIED, true, K);
    clearLowestSetBit(EHV);
  }
  return DZ;
}

/** Gets the bitboard of the squares the bishops/queens are attacking
 * diagonally.
 *
 * @param K: Bitboard of the active player's king.
 * @param EB: Bitboard of the enemy player's bishops.
 * @param EQ: Bitboard of the enemy player's queens.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @return Bitboard of the squares the bishops/queens are attacking diagonally.
 */
uint64_t getBishopQueenAttackZone(uint64_t K, uint64_t EB, uint64_t EQ,
                                  uint64_t OCCUPIED) {
  uint64_t ED = EB | EQ;
  uint64_t DZ = 0;
  while (ED) {
    uint64_t diag_piece = getLowestSetBitValue(ED);
    DZ |= diagonalMoves(diag_piece, OCCUPIED, true, K);

    clearLowestSetBit(ED);
  }
  return DZ;
}

/** Gets the bitboard of the squares the knight is attacking.
 *
 * @param N: Bitboard of the knight.
 * @return Bitboard of the squares the knight is attacking.
 */
uint64_t getKnightAttackZone(uint64_t N) {
  uint64_t DZ = 0;
  while (N) {
    DZ |= knight_moves[getSetBit(getLowestSetBitValue(N))];
    clearLowestSetBit(N);
  }
  return DZ;
}

/** Gets the bitboard of the squares the king is attacking.
 *
 * @param K: Bitboard of the king.
 * @return Bitboard of the squares the king is attacking.
 */
uint64_t getKingAttackZone(uint64_t K) { return king_moves[getSetBit(K)]; }

/** Determines if the bishops/queens are putting the king in check
 * horizontally/vertically.
 *
 * @param K: Bitboard of the active player's king.
 * @param ER: Bitboard of the enemy player's rooks.
 * @param EQ: Bitboard of the enemy player's queens.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param checker_zone: Bitboard of zone of the checkers, if applicable.
 * @return Number of rooks/queens putting the king in check (1 or 2), else 0.
 */
uint8_t getHorizontalAndVerticalChecker(uint64_t K, uint64_t ER, uint64_t EQ,
                                        uint64_t OCCUPIED,
                                        uint64_t &checker_zone) {
  uint64_t EHV = ER | EQ;
  uint64_t K_exposure = horizontalAndVerticalMoves(K, OCCUPIED);
  uint64_t new_checker = K_exposure & EHV;
  if (countSetBits(new_checker) == 2) {
    return 2;
  }
  if (new_checker) {
    checker_zone |= new_checker;
    checker_zone |=
        horizontalAndVerticalMoves(new_checker, OCCUPIED) & K_exposure;
    return 1;
  }
  return 0;
}

/** Determines if the bishops/queens are putting the king in check diagonally.
 *
 * @param K: Bitboard of the active player's king.
 * @param EB: Bitboard of the enemy player's bishops.
 * @param EQ: Bitboard of the enemy player's queens.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param checker_zone: Bitboard of zone of the checkers, if applicable.
 * @return 1 if the bishop/queen is putting the king in check, else 0.
 */
uint8_t getDiagonalChecker(uint64_t K, uint64_t EB, uint64_t EQ,
                           uint64_t OCCUPIED, uint64_t &checker_zone) {
  uint64_t ED = EB | EQ;
  uint64_t K_exposure = diagonalMoves(K, OCCUPIED);
  uint64_t new_checker = K_exposure & ED;
  if (new_checker) {
    checker_zone |= new_checker;
    checker_zone |= diagonalMoves(new_checker, OCCUPIED) & K_exposure;
    return 1;
  }
  return 0;
}

/** Determines if the knights are putting the king in check.
 *
 * @param K: Bitboard of the active player's king.
 * @param EN: Bitboard of the enemy player's knights.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param checker_zone: Bitboard of zone of the checkers, if applicable.
 * @return 1 if the knight is putting the king in check, else 0.
 */
uint8_t getKnightChecker(uint64_t K, uint64_t EN, uint64_t OCCUPIED,
                         uint64_t &checker_zone) {
  // Check for knight attacks.
  uint64_t K_exposure = knight_moves[getSetBit(K)];

  uint64_t new_checker = K_exposure & EN;
  if (new_checker) {
    checker_zone |= new_checker;
    return 1;
  }
  return 0;
}

/** Determines if the pawns are putting the king in check.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param K: Bitboard of the active player's king.
 * @param EP: Bitboard of the enemy player's pawns.
 * @param checker_zone: Bitboard of zone of the checkers, if applicable.
 * @return 1 if the pawn is putting the king in check, else 0.
 */
uint8_t getPawnChecker(bool white_to_move, uint64_t K, uint64_t EP,
                       uint64_t &checker_zone) {

  if (white_to_move) {
    // Check for pawn right attack (from pawns perspective).
    uint64_t K_exposure = (K << 7) & ~file_h;
    uint64_t new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }

    // check for pawn left attack (from pawns perspective)
    K_exposure = (K << 9) & ~file_a;
    new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }

  } else {
    // Check for pawn right attack (from pawns perspective).
    uint64_t K_exposure = (K >> 9) & ~file_h;
    uint64_t new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }

    // check for pawn left attack (from pawns perspective)
    K_exposure = (K >> 7) & ~file_a;
    new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }
  }

  return 0;
}

/** Determines if the king is in check.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param K: Bitboard of the active player's king.
 * @param enemy_player_state: Enemy player state.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param DZ: Bitboard of the danger zone, where the king cannot pass through.
 * @param checker_zone: Bitboard of zone of the checkers, if applicable.
 * @param n_checkers: The number of pieces putting the king in check.
 * @return true if in check, else false.
 */
bool isInCheck(bool white_to_move, uint64_t K, ColorState enemy_player_state,
               uint64_t OCCUPIED, uint64_t &DZ, uint64_t &checker_zone,
               uint8_t &n_checkers) {
  DZ |= getPawnAttackZone(white_to_move, enemy_player_state.pawn);
  DZ |= getRookQueenAttackZone(K, enemy_player_state.rook,
                               enemy_player_state.queen, OCCUPIED);
  DZ |= getBishopQueenAttackZone(K, enemy_player_state.bishop,
                                 enemy_player_state.queen, OCCUPIED);
  DZ |= getKnightAttackZone(enemy_player_state.knight);
  DZ |= getKingAttackZone(enemy_player_state.king);

  bool check = K & DZ;
  if (check) {
    n_checkers +=
        getPawnChecker(white_to_move, K, enemy_player_state.pawn, checker_zone);
    n_checkers += getHorizontalAndVerticalChecker(K, enemy_player_state.rook,
                                                  enemy_player_state.queen,
                                                  OCCUPIED, checker_zone);
    n_checkers +=
        getDiagonalChecker(K, enemy_player_state.bishop,
                           enemy_player_state.queen, OCCUPIED, checker_zone);
    n_checkers +=
        getKnightChecker(K, enemy_player_state.knight, OCCUPIED, checker_zone);
  }
  return check;
}

/** Returns a bitboard of pieces that are pinned.
 *
 * @param K: Bitboard of the active player's king.
 * @param P: Bitboard of the active player's pawns.
 * @param EQ: Bitboard of the enemy player's queens.
 * @param EB: Bitboard of the enemy player's bishops.
 * @param ER: Bitboard of the enemy player's rooks.
 * @param OCCUPIED: Bitboard of all the occupied spaces on the board.
 * @param en_passant: The en passant bit, if applicable.
 * @param white_to_move: Flag denoting the turn.
 * @return Bitboard of all pinned pieces.
 */
uint64_t getPinnedPieces(uint64_t K, uint64_t P, uint64_t EQ, uint64_t EB,
                         uint64_t ER, uint64_t OCCUPIED, int8_t &en_passant,
                         bool white_to_move) {
  uint64_t PINNED = 0;
  uint8_t k_bit = getSetBit(K);

  // Horizontal check.
  uint64_t K_h_v_slider = horizontalAndVerticalMoves(K, OCCUPIED);
  uint64_t K_slider = K_h_v_slider & directional_mask[k_bit][RANKS];

  uint64_t EHV = EQ | ER;
  while (EHV) {
    uint64_t bb = getLowestSetBitValue(EHV);
    uint64_t H_moves = horizontalAndVerticalMoves(bb, OCCUPIED) &
                       directional_mask[getSetBit(bb)][RANKS];

    uint64_t E_P = getEnPassantBitboard(en_passant);
    // Check for special en passant pins.
    uint64_t ep_pawn = white_to_move ? E_P >> 8 : E_P << 8;
    uint64_t p_can_capture_ep_r =
        P & (white_to_move ? (E_P >> 9) & ~file_h : (E_P << 7) & ~file_a);
    uint64_t p_can_capture_ep_l =
        P & (white_to_move ? (E_P >> 7) & ~file_a : (E_P << 9) & ~file_h);
    bool none_between_king_and_ep_p = ep_pawn & K_slider;
    bool none_between_ep_p_and_attacker = ep_pawn & H_moves;

    if ((none_between_king_and_ep_p && (p_can_capture_ep_r & H_moves)) ||
        (none_between_ep_p_and_attacker && (p_can_capture_ep_r & K_slider)) ||
        (none_between_king_and_ep_p && (p_can_capture_ep_l & H_moves)) ||
        (none_between_ep_p_and_attacker && (p_can_capture_ep_l & K_slider))) {
      en_passant = -1;
    }
    PINNED |= K_slider & H_moves;
    clearLowestSetBit(EHV);
  }

  // Vertical check.
  EHV = EQ | ER;
  K_slider = K_h_v_slider & directional_mask[k_bit][FILES];

  while (EHV) {
    uint64_t bb = getLowestSetBitValue(EHV);
    PINNED |= K_slider & horizontalAndVerticalMoves(bb, OCCUPIED) &
              directional_mask[getSetBit(bb)][FILES];
    clearLowestSetBit(EHV);
  }

  // Down right diagonal check.
  uint64_t K_slider_diag = diagonalMoves(K, OCCUPIED);
  uint64_t ED = EQ | EB;
  K_slider = K_slider_diag & directional_mask[k_bit][DIAGONALS_DOWN_RIGHT];
  while (ED) {
    uint64_t bb = getLowestSetBitValue(ED);
    PINNED |= K_slider & diagonalMoves(bb, OCCUPIED) &
              directional_mask[getSetBit(bb)][DIAGONALS_DOWN_RIGHT];

    clearLowestSetBit(ED);
  }

  // Upper right diagonal check.
  ED = EQ | EB;
  K_slider = K_slider_diag & directional_mask[k_bit][DIAGONALS_UP_RIGHT];
  while (ED) {
    uint64_t bb = getLowestSetBitValue(ED);
    PINNED |= K_slider & diagonalMoves(bb, OCCUPIED) &
              directional_mask[getSetBit(bb)][DIAGONALS_UP_RIGHT];
    clearLowestSetBit(ED);
  }
  return PINNED;
}

/** Adds the kingside castle move to the move list, if applicable.
 *
 * @param CK: Flag denoting if the kingside castle is still available.
 * @param K: Bitboard of the active player's king.
 * @param EMPTY: Bitboard of the empty squares.
 * @param DZ: Bitboard of the danger zone, where the king is not allowed to pass
 * through.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateKingsideCastleMove(bool CK, uint64_t K, uint64_t EMPTY,
                                uint64_t DZ, Move *moves, uint8_t &n_moves) {
  if (!CK) {
    return;
  }

  if ((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)) {
    uint8_t k_bit =
        getSetBit((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1));

    uint8_t x = bitToX(k_bit);
    uint8_t y = bitToY(k_bit);
    moves[n_moves++] = Move(x, y - 2, x, y, CASTLE_KINGSIDE);
  }
}

/** Adds the queenside castle move to the move list, if applicable.
 *
 * @param QK: Flag denoting if the queenside castle is still available.
 * @param K: Bitboard of the active player's king.
 * @param EMPTY: Bitboard of the empty squares.
 * @param DZ: Bitboard of the danger zone, where the king is not allowed to pass
 * through.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateQueensideCastleMove(bool QK, uint64_t K, uint64_t EMPTY,
                                 uint64_t DZ, Move *moves, uint8_t &n_moves) {
  if (!QK) {
    return;
  }

  if ((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)) !=
      0u) {
    uint8_t k_bit = getSetBit(((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) &
                              ~DZ & ~(DZ >> 1));
    uint8_t x = bitToX(k_bit);
    uint8_t y = bitToY(k_bit);
    moves[n_moves++] = Move(x, y + 2, x, y, CASTLE_QUEENSIDE);
  }
}

/** Helper function that adds all 4 promotion moves to the move list.
 *
 * @param move: Move.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void addAllPromotionMoves(Move move, Move *moves, uint8_t &n_moves){
  move.setMoveType(PROMOTION_QUEEN);
  moves[n_moves++] = move;
  move.setMoveType(PROMOTION_ROOK);
  moves[n_moves++] = move;
  move.setMoveType(PROMOTION_BISHOP);
  moves[n_moves++] = move;
  move.setMoveType(PROMOTION_KNIGHT);
  moves[n_moves++] = move;
}

/** Generates the possible/legal moves for pinned pawns.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param PINNED_MASK: The line of pinning, if applicable.
 * @param P: Bitboard of active player's pawns.
 * @param K: Bitboard of active player's king.
 * @param en_passant: The en passant bit, if applicable.
 * @param EMPTY: Bitboard of empty squares.
 * @param ENEMY_PIECES: Bitboard of enemy pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generatePawnMoves(bool white_to_move, uint64_t PINNED_MASK, uint64_t P,
                       uint64_t K, int8_t en_passant, uint64_t EMPTY,
                       uint64_t ENEMY_PIECES, uint64_t checker_zone,
                       Move *moves, uint8_t &n_moves) {
  // TODO REFACTOR?
  uint64_t P_FORWARD_1 =
      EMPTY & PINNED_MASK & checker_zone &
      (white_to_move ? (P << 8) & ~rank_8 : (P >> 8) & ~rank_1);
  uint64_t P_FORWARD_2 = EMPTY & PINNED_MASK & checker_zone &
                         (white_to_move ? (P << 16) & (EMPTY << 8) & rank_4
                                        : (P >> 16) & (EMPTY >> 8) & rank_5);
  uint64_t P_ATTACK_L =
      ENEMY_PIECES & ~file_h & PINNED_MASK & checker_zone &
      (white_to_move ? (P << 7) & ~rank_8 : (P >> 9) & ~rank_1);
  uint64_t P_ATTACK_R =
      ENEMY_PIECES & ~file_a & PINNED_MASK & checker_zone &
      (white_to_move ? (P << 9) & ~rank_8 : (P >> 7) & ~rank_1);
  uint64_t P_PROMO_1 = EMPTY & PINNED_MASK & checker_zone &
                       (white_to_move ? (P << 8) & rank_8 : (P >> 8) & rank_1);
  uint64_t P_PROMO_L = ENEMY_PIECES & ~file_h & PINNED_MASK & checker_zone &
                       (white_to_move ? (P << 7) & rank_8 : (P >> 9) & rank_1);
  uint64_t P_PROMO_R = ENEMY_PIECES & ~file_a & PINNED_MASK & checker_zone &
                       (white_to_move ? (P << 9) & rank_8 : (P >> 7) & rank_1);

  uint64_t E_P = getEnPassantBitboard(en_passant);
  checker_zone |= white_to_move && ((E_P >> 8) & checker_zone) ? E_P : 0;
  checker_zone |= !white_to_move && ((E_P << 8) & checker_zone) ? E_P : 0;
  uint64_t P_EP_L = E_P & ~file_h & PINNED_MASK & checker_zone &
                    (white_to_move ? P << 7 : P >> 9);
  uint64_t P_EP_R = E_P & ~file_a & PINNED_MASK & checker_zone &
                    (white_to_move ? P << 9 : P >> 7);

  // CHECK TO SEE IF WE CAN MOVE 1 SPACE FORWARD
  while (P_FORWARD_1) {
    uint64_t bb = getLowestSetBitValue(P_FORWARD_1);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    moves[n_moves++] = Move(x + (white_to_move ? -1 : 1), y, x, y);
    clearLowestSetBit(P_FORWARD_1);
  }

  // check to see if you can move 2
  while (P_FORWARD_2) {
    uint64_t bb = getLowestSetBitValue(P_FORWARD_2);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    moves[n_moves++] = Move(x + (white_to_move ? -2 : 2), y, x, y, PAWN_PUSH_2);
    clearLowestSetBit(P_FORWARD_2);
  }

  // check for attacks left
  while (P_ATTACK_L) {
    uint64_t bb = getLowestSetBitValue(P_ATTACK_L);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    moves[n_moves++] = Move(x + (white_to_move ? -1 : 1), y + 1, x, y);
    clearLowestSetBit(P_ATTACK_L);
  }

  // check for attacks right
  while (P_ATTACK_R) {
    uint64_t bb = getLowestSetBitValue(P_ATTACK_R);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    moves[n_moves++] = Move(x + (white_to_move ? -1 : 1), y - 1, x, y);
    clearLowestSetBit(P_ATTACK_R);
  }

  // check for promotion straight
  while (P_PROMO_1) {
    uint64_t bb = getLowestSetBitValue(P_PROMO_1);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    addAllPromotionMoves(Move(x + (white_to_move ? -1 : 1), y, x, y), moves,
                         n_moves);
    clearLowestSetBit(P_PROMO_1);
  }

  // check for promotion left
  while (P_PROMO_L) {
    uint64_t bb = getLowestSetBitValue(P_PROMO_L);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    addAllPromotionMoves(Move(x + (white_to_move ? -1 : 1), y + 1, x, y), moves,
                         n_moves);
    clearLowestSetBit(P_PROMO_L);
  }

  // check for promotion attack right
  while (P_PROMO_R) {
    uint64_t bb = getLowestSetBitValue(P_PROMO_R);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    addAllPromotionMoves(Move(x + (white_to_move ? -1 : 1), y - 1, x, y), moves,
                         n_moves);
    clearLowestSetBit(P_PROMO_R);
  }

  // check for en passant left
  while (P_EP_L) {
    uint64_t bb = getLowestSetBitValue(P_EP_L);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    moves[n_moves++] = Move(x + (white_to_move ? -1 : 1), y + 1, x, y);
    clearLowestSetBit(P_EP_L);
  }

  // check for en passant right
  while (P_EP_R) {
    uint64_t bb = getLowestSetBitValue(P_EP_R);
    uint8_t final_bit = getSetBit(bb);
    uint8_t x = bitToX(final_bit);
    uint8_t y = bitToY(final_bit);
    moves[n_moves++] = Move(x + (white_to_move ? -1 : 1), y - 1, x, y);
    clearLowestSetBit(P_EP_R);
  }
}

/** Generates the possible/legal moves for pinned pawns.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param P: Bitboard of active player's pawns.
 * @param K: Bitboard of active player's king.
 * @param en_passant: The en passant bit, if applicable.
 * @param EMPTY: Bitboard of empty squares.
 * @param ENEMY_PIECES: Bitboard of enemy pieces.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generatePinnedPawnMoves(bool white_to_move, uint64_t &P, uint64_t K,
                             int8_t en_passant, uint64_t EMPTY,
                             uint64_t ENEMY_PIECES, uint64_t PINNED,
                             uint64_t checker_zone, Move *moves,
                             uint8_t &n_moves) {
  uint64_t pinned_pawns = P & PINNED;
  while (pinned_pawns) {
    uint64_t bb = getLowestSetBitValue(pinned_pawns);
    uint64_t mask = getColinearMask(bb, K);
    generatePawnMoves(white_to_move, mask, bb, K, en_passant, EMPTY,
                      ENEMY_PIECES, checker_zone, moves, n_moves);
    clearLowestSetBit(pinned_pawns);
  }
  // Clear pinned pawns from pawn bitboard.
  P &= ~PINNED;
}

/** Generates the possible/legal moves for black pawns.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param BP: Bitboard of black pawns.
 * @param BK: Bitboard of black king.
 * @param en_passant: The en passant bit, if applicable.
 * @param EMPTY: Bitboard of empty squares.
 * @param WHITE_PIECES: Bitboard of white pieces.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateBlackPawnMoves(bool white_to_move, uint64_t BP, uint64_t BK,
                            int8_t en_passant, uint64_t EMPTY,
                            uint64_t WHITE_PIECES, uint64_t PINNED,
                            uint64_t checker_zone, Move *moves,
                            uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  generatePinnedPawnMoves(white_to_move, BP, BK, en_passant, EMPTY,
                          WHITE_PIECES, PINNED, checker_zone, moves, n_moves);

  if (BP) { // we have at least 1 non-pinned pawn.
    generatePawnMoves(white_to_move, FILLED, BP, BK, en_passant, EMPTY,
                      WHITE_PIECES, checker_zone, moves, n_moves);
  }
}

/** Generates the possible/legal moves for white pawns.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param WP: Bitboard of white pawns.
 * @param WK: Bitboard of white king.
 * @param en_passant: The en passant bit, if applicable.
 * @param EMPTY: Bitboard of empty squares.
 * @param BLACK_PIECES: Bitboard of black pieces.
 * @param PINNED: Bitboard of pinned pieces.
 * @param checker_zone: Bitboard of the zone of the checkers, if applicable.
 * @param moves: Move list.
 * @param n_moves: Running total number of moves.
 */
void generateWhitePawnMoves(bool white_to_move, uint64_t WP, uint64_t WK,
                            int8_t en_passant, uint64_t EMPTY,
                            uint64_t BLACK_PIECES, uint64_t PINNED,
                            uint64_t checker_zone, Move *moves,
                            uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  generatePinnedPawnMoves(white_to_move, WP, WK, en_passant, EMPTY,
                          BLACK_PIECES, PINNED, checker_zone, moves, n_moves);

  if (WP) { // we have at least 1 non-pinned pawn
    generatePawnMoves(white_to_move, FILLED, WP, WK, en_passant, EMPTY,
                      BLACK_PIECES, checker_zone, moves, n_moves);
  }
}

/** Generates the possible/legal moves for white.
 *
 * @param game_state: Game state.
 * @param moves: Move list.
 * @param check: Returns true if the player is in check.
 */
uint8_t generateBlackMoves(GameState &game_state, Move *moves, bool &check) {
  uint64_t WHITE_PIECES = game_state.getWhiteOccupiedBitboard();
  uint64_t BLACK_PIECES = game_state.getBlackOccupiedBitboard();
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  uint64_t DZ = 0;
  uint8_t n_checkers = 0;
  uint64_t checker_zone = 0;
  check = isInCheck(game_state.whites_turn, game_state.black.king,
                    game_state.white, OCCUPIED, DZ, checker_zone, n_checkers);

  uint64_t PINNED = getPinnedPieces(
      game_state.black.king, game_state.black.pawn, game_state.white.queen,
      game_state.white.bishop, game_state.white.rook, OCCUPIED,
      game_state.en_passant, game_state.whites_turn);

  uint8_t n_moves = 0;
  if (!check) {
    generateKingsideCastleMove(game_state.black.can_king_side_castle,
                               game_state.black.king, ~OCCUPIED, DZ, moves,
                               n_moves);
    generateQueensideCastleMove(game_state.black.can_queen_side_castle,
                                game_state.black.king, ~OCCUPIED, DZ, moves,
                                n_moves);
  }

  if (n_checkers < 2) {
    generateBlackPawnMoves(game_state.whites_turn, game_state.black.pawn,
                           game_state.black.king, game_state.en_passant,
                           ~OCCUPIED, WHITE_PIECES, PINNED, checker_zone, moves,
                           n_moves);
    generateRookMoves(game_state.black.rook, game_state.black.king,
                      BLACK_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                      n_moves);
    generateBishopMoves(game_state.black.bishop, game_state.black.king,
                        BLACK_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                        n_moves);
    generateQueenMoves(game_state.black.queen, game_state.black.king,
                       BLACK_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                       n_moves);
    generateKnightMoves(game_state.black.knight, BLACK_PIECES, PINNED,
                        checker_zone, moves, n_moves);
  }
  generateKingMoves(game_state.black.king, BLACK_PIECES, DZ, moves, n_moves);

  return n_moves;
}

/** Generates the possible/legal moves for white.
 *
 * @param game_state: Game state.
 * @param moves: Move list.
 * @param check: Returns true if the player is in check.
 */
uint8_t generateWhiteMoves(GameState &game_state, Move *moves, bool &check) {
  uint64_t WHITE_PIECES = game_state.getWhiteOccupiedBitboard();
  uint64_t BLACK_PIECES = game_state.getBlackOccupiedBitboard();
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  uint64_t DZ = 0;
  uint8_t n_checkers = 0;
  uint64_t checker_zone = 0;
  check = isInCheck(game_state.whites_turn, game_state.white.king,
                    game_state.black, OCCUPIED, DZ, checker_zone, n_checkers);

  uint64_t PINNED = getPinnedPieces(
      game_state.white.king, game_state.white.pawn, game_state.black.queen,
      game_state.black.bishop, game_state.black.rook, OCCUPIED,
      game_state.en_passant, game_state.whites_turn);

  uint8_t n_moves = 0;
  if (!check) {
    generateKingsideCastleMove(game_state.white.can_king_side_castle,
                               game_state.white.king, ~OCCUPIED, DZ, moves,
                               n_moves);
    generateQueensideCastleMove(game_state.white.can_queen_side_castle,
                                game_state.white.king, ~OCCUPIED, DZ, moves,
                                n_moves);
  }

  if (n_checkers < 2) {
    generateWhitePawnMoves(game_state.whites_turn, game_state.white.pawn,
                           game_state.white.king, game_state.en_passant,
                           ~OCCUPIED, BLACK_PIECES, PINNED, checker_zone, moves,
                           n_moves);
    generateRookMoves(game_state.white.rook, game_state.white.king,
                      WHITE_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                      n_moves);
    generateBishopMoves(game_state.white.bishop, game_state.white.king,
                        WHITE_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                        n_moves);
    generateQueenMoves(game_state.white.queen, game_state.white.king,
                       WHITE_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                       n_moves);
    generateKnightMoves(game_state.white.knight, WHITE_PIECES, PINNED,
                        checker_zone, moves, n_moves);
  }
  generateKingMoves(game_state.white.king, WHITE_PIECES, DZ, moves, n_moves);

  return n_moves;
}

uint8_t generateMoves(GameState &game_state, Move *moves, bool &check) {
  return game_state.whites_turn ? generateWhiteMoves(game_state, moves, check)
                                : generateBlackMoves(game_state, moves, check);
}

void print_moves(bool white_to_move, Move *moves, uint8_t n_moves) {
  std::cout << (white_to_move ? "WHITE" : "BLACK") << "'S MOVE: " << std::endl;
  for (uint8_t i = 0; i < n_moves; i++) {
    std::cout << i + 1 << ": " + moves[i].toString() << std::endl;
  }
}
