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

/** Runs the perft test to check accuracy of the move generator.
 *
 * @param nodes: Stores the total number of nodes explored.
 * @param game_state: Game state.
 * @param depth: Depth to test to.
 * @param orig_depth: Depth to test to. Used for printing status.
 * @param total: Flag to denote if the search is for total nodes, or nodes per
 * move at depth 1.
 */
void perft(uint32_t &nodes, GameState &game_state, uint8_t depth,
           uint8_t orig_depth, bool total);
