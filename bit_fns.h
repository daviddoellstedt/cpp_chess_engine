//
// Created by David Doellstedt on 5/12/20.
//

#ifndef DDS_CHESS_ENGINE_BIT_FNS_H
#define DDS_CHESS_ENGINE_BIT_FNS_H

#include <string>

struct Move {
  // Moves are stored as a 16-bit integer. Kept as lightweight as possible to
  // allow deeper search. Bits: 0 - 2:   Initial x position 3 - 5:   Initial y
  // position 6 - 8:   Final x position 9 - 11:  Final y position 12 - 15:
  // Special move flags (see SpecialMove enum)
  uint16_t data = 0;
};

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
  // todo: add possible EP.
  // todo: add total moves.
  // todo: add half moves.

  // Player state of the white pieces.
  PlayerState white;

  // Player state of the black pieces.
  PlayerState black;

  // If true -> white's turn, false -> black's turn.
  bool whites_turn = true;

  uint64_t en_passant = 0;
} GameState;

void generate_board(std::string name, uint8_t diff);

void perft(uint32_t &nodes, GameState &gamestate, Move *moves, uint8_t &n_moves,
           bool CM, bool SM, uint8_t depth, uint8_t orig_depth, bool total);

void fenToGameState(const std::string fen, GameState &gamestate);

void print_board(const GameState gamestate);

#endif // DDS_CHESS_ENGINE_BIT_FNS_H