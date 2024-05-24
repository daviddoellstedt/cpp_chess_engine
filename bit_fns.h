//
// Created by David Doellstedt on 5/12/20.
//

#ifndef DDS_CHESS_ENGINE_BIT_FNS_H
#define DDS_CHESS_ENGINE_BIT_FNS_H

#include <string>

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

    //Player state of the white pieces.
    PlayerState white;

    // Player state of the black pieces.
    PlayerState black;

    // If true -> white's turn, false -> black's turn.
    bool whites_turn = true;
} GameState;

void generate_board(std::string name, int diff);

void perft(uint32_t &nodes, uint32_t& cap_counter, GameState& gamestate, std::vector<std::string> moves, uint64_t& E_P, bool CM, bool SM, int depth, int orig_depth, std::string n);

void fenToGameState(const std::string fen, GameState& gamestate);

void print_board(const GameState gamestate);



#endif //DDS_CHESS_ENGINE_BIT_FNS_H

