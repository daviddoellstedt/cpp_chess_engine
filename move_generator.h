#pragma once

#include "board.h"
#include "move.h"
#include <stdint.h>
#include <string>

/** Initializes the magic bitboard tables.
 */
void initializeMagicBitboardTables(void);

// TODO add documentation.
uint8_t generateMoves(GameState &game_state, Move *moves, bool &check);

// TODO Add documentation.
void print_moves(bool white_to_move, Move *moves, uint8_t n_moves);

// TODO Add documentation.
void perft(uint32_t &nodes, GameState &game_state, uint8_t depth,
           uint8_t orig_depth, bool total);