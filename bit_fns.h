//
// Created by David Doellstedt on 5/12/20.
//

#ifndef DDS_CHESS_ENGINE_BIT_FNS_H
#define DDS_CHESS_ENGINE_BIT_FNS_H

#include <stdint.h>
#include <string>

enum SpecialMove : uint8_t {
  NONE = 0,
  CASTLE_KINGSIDE = 1,
  CASTLE_QUEENSIDE = 2,
  PROMOTION_QUEEN = 3,
  PROMOTION_ROOK = 4,
  PROMOTION_KNIGHT = 5,
  PROMOTION_BISHOP = 6,
  EN_PASSANT = 7,
  PAWN_PUSH_2 = 8,
};

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

struct MoveGameStateScore {
  Move move;
  GameState gamestate;
  int16_t score = 0;

  // Overloaded comparator. Used for sorting in descending order.
  bool operator<(const MoveGameStateScore &move_gamestate_score) const {
    return score > move_gamestate_score.score;
  }
};

uint64_t h_moves(uint64_t piece, uint64_t OCCUPIED);

void initializeRookAttacks(void);

void generateRookMagicNumber(uint8_t bit);

void initializeRookMagicTable(void);

void initializeBishopAttacks(void);

void generateBishopMagicNumber(uint8_t bit);

void initializeBishopMagicTable(void);

void generate_board(std::string name, uint8_t diff);

void perft(uint32_t &nodes, GameState &gamestate, uint8_t depth,
           uint8_t orig_depth, bool total);

void fenToGameState(const std::string fen, GameState &gamestate);

void print_board(const GameState gamestate);

#endif // DDS_CHESS_ENGINE_BIT_FNS_H