#pragma once

#include "board.h"
#include "move.h"
#include <stdint.h>
#include <string>

/** Initializes the magic bitboard tables.
 */
void initializeMagicBitboardTables(void);

/** Generates the possible/legal moves.
 *
 * @param game_state: Game state.
 * @param moves: Move list.
 * @param check: Returns true if the player is in check.
 * @return Number of moves.
 */
uint8_t generateMoves(GameState &game_state, Move *moves, bool &check);

/** Prints the move list.
 *
 * @param white_to_move: Flag denoting the turn.
 * @param moves: Move list.
 * @param n_moves: Number of moves.
 */
void print_moves(bool white_to_move, Move *moves, uint8_t n_moves);
