#pragma once

#include <stdint.h>

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
                                              uint64_t K = 0);

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
                                 bool unsafe_calc = false, uint64_t K = 0);

/** Initializes the magic bitboard tables.
 */
void initializeMagicBitboardTables(void);

// Temp, move back once all refactored.
uint64_t h_v_moves(uint64_t piece, uint64_t OCCUPIED, bool unsafe_calc = false,
                   uint64_t K = 0);

// Temp, move back once all refactored.
uint64_t diag_moves(uint64_t piece, uint64_t OCCUPIED, bool unsafe_calc = false,
                    uint64_t K = 0);