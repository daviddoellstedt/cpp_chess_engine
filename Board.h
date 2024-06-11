#pragma once

#include <stdint.h>

typedef struct PlayerState {
  uint64_t rook = 0;
  uint64_t knight = 0;
  uint64_t bishop = 0;
  uint64_t queen = 0;
  uint64_t king = 0;
  uint64_t pawn = 0;
  bool can_king_side_castle = false;
  bool can_queen_side_castle = false;
} PlayerState;

typedef struct GameState {
  // TODO: add total moves.
  // TODO: add half moves.

  // Player state of the white pieces.
  PlayerState white;

  // Player state of the black pieces.
  PlayerState black;

  // If true -> white's turn, false -> black's turn.
  bool whites_turn = true;

  uint64_t en_passant = 0;
} GameState;

/** Prints board to std out.
 *
 * @param game_state: Game state.
 */
void printBoard(const GameState &game_state);

/** Generates occupied bitboard, specific to the player.
 *
 * @param player_state: Player state.
 * @return Bitboard of occupied pieces by player.
 */
uint64_t generatePlayerOccupiedBitboard(const PlayerState &player_state);

/** Generates white occupied bitboard.
 *
 * @param game_state: Game state.
 * @return White occupied bitboard.
 */
uint64_t generateWhiteOccupiedBitboard(const GameState &game_state);

/** Generates black occupied bitboard.
 *
 * @param game_state: Game state.
 * @return Black occupied bitboard.
 */
uint64_t generateBlackOccupiedBitboard(const GameState &game_state);
