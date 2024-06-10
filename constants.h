#ifndef DDS_CHESS_ENGINE_CONSTANTS_H
#define DDS_CHESS_ENGINE_CONSTANTS_H

#include <stdint.h>
#include <unordered_map>

// Edge bits all set.
const uint64_t OUTER_BITS = 0xFF818181818181FF;
const uint64_t INNER_BITS = ~OUTER_BITS;

// 8 x 8 chessboard.
const uint8_t N_SQUARES = 64;

// Mask of the possible ways a knight can move.
const uint64_t KNIGHT_MOVES = 0x5088008850;
const uint8_t KNIGHT_MASK_BIT_POSITION = 21;

// Mask of the possible ways a king can move.
const uint64_t KING_MOVES = 0xE0A0E0;
const uint8_t KING_MASK_BIT_POSITION = 14;

// Mask of entirely filled board.
const uint64_t FILLED = ~0;

// Max number of moves per position.
// https://www.chessprogramming.org/Chess_Position#:~:text=The%20maximum%20number%20of%20moves%20per%20chess%20position%20seems%20218.
const uint8_t MAX_POSSIBLE_MOVES_PER_POSITION = 218;

// Castling constants.
const uint8_t WHITE_ROOK_STARTING_POSITION_KINGSIDE = 0x80;
const uint8_t WHITE_ROOK_STARTING_POSITION_QUEENSIDE = 0x1;
const uint64_t BLACK_ROOK_STARTING_POSITION_KINGSIDE = 0x8000000000000000;
const uint64_t BLACK_ROOK_STARTING_POSITION_QUEENSIDE = 0x100000000000000;

const uint8_t WHITE_ROOK_POST_KINGSIDE_CASTLE_POSITION = 0x20;
const uint8_t WHITE_ROOK_POST_QUEENSIDE_CASTLE_POSITION = 0x8;
const uint64_t BLACK_ROOK_POST_KINGSIDE_CASTLE_POSITION = 0x2000000000000000;
const uint64_t BLACK_ROOK_POST_QUEENSIDE_CASTLE_POSITION = 0x800000000000000;

// Masks for reading/writing to move.
const uint16_t X_INITIAL = 0x7;
const uint16_t Y_INITIAL = 0x38;
const uint16_t X_FINAL = 0x1C0;
const uint16_t Y_FINAL = 0xE00;
const uint16_t SPECIAL = 0xF000;

// Ranks.
const uint64_t rank_1 = 0xFF;
const uint64_t rank_2 = 0xFF00;
const uint64_t rank_3 = 0xFF0000;
const uint64_t rank_4 = 0xFF000000;
const uint64_t rank_5 = 0xFF00000000;
const uint64_t rank_6 = 0xFF0000000000;
const uint64_t rank_7 = 0xFF000000000000;
const uint64_t rank_8 = 0xFF00000000000000;

// Files.
const uint64_t file_a = 0x101010101010101;
const uint64_t file_b = 0x202020202020202;
const uint64_t file_c = 0x404040404040404;
const uint64_t file_d = 0x808080808080808;
const uint64_t file_e = 0x1010101010101010;
const uint64_t file_f = 0x2020202020202020;
const uint64_t file_g = 0x4040404040404040;
const uint64_t file_h = 0x8080808080808080;
const uint64_t file_ab = file_a | file_b;
const uint64_t file_gh = file_g | file_h;

// Down/right diagonals.
const uint64_t diagonal_a1 = 0x1;
const uint64_t diagonal_a2b1 = 0x102;
const uint64_t diagonal_a3c1 = 0x10204;
const uint64_t diagonal_a4d1 = 0x1020408;
const uint64_t diagonal_a5e1 = 0x102040810;
const uint64_t diagonal_a6f1 = 0x10204081020;
const uint64_t diagonal_a7g1 = 0x1020408102040;
const uint64_t diagonal_a8h1 = 0x102040810204080;
const uint64_t diagonal_b8h2 = 0x204081020408000;
const uint64_t diagonal_c8h3 = 0x408102040800000;
const uint64_t diagonal_d8h4 = 0x810204080000000;
const uint64_t diagonal_e8h5 = 0x1020408000000000;
const uint64_t diagonal_f8h6 = 0x2040800000000000;
const uint64_t diagonal_g8h7 = 0x4080000000000000;
const uint64_t diagonal_h8 = 0x8000000000000000;

// Up/right diagonals.
const uint64_t diagonal_a8 = 0x100000000000000;
const uint64_t diagonal_a7b8 = 0x201000000000000;
const uint64_t diagonal_a6c8 = 0x402010000000000;
const uint64_t diagonal_a5d8 = 0x804020100000000;
const uint64_t diagonal_a4e8 = 0x1008040201000000;
const uint64_t diagonal_a3f8 = 0x2010080402010000;
const uint64_t diagonal_a2g8 = 0x4020100804020100;
const uint64_t diagonal_a1h8 = 0x8040201008040201;
const uint64_t diagonal_b1h7 = 0x80402010080402;
const uint64_t diagonal_c1h6 = 0x804020100804;
const uint64_t diagonal_d1h5 = 0x8040201008;
const uint64_t diagonal_e1h4 = 0x80402010;
const uint64_t diagonal_f1h3 = 0x804020;
const uint64_t diagonal_g1h2 = 0x8040;
const uint64_t diagonal_h1 = 0x80;

enum directional_indices {
  RANKS = 0,
  FILES = 1,
  DIAGONALS_DOWN_RIGHT = 2,
  DIAGONALS_UP_RIGHT = 3,
  N_DIRECTIONS = 4
};

// Stores the horizontal, vertical, and diagonal masks for each square.
const uint64_t directional_mask[N_SQUARES][N_DIRECTIONS] = {
    {rank_1, file_a, diagonal_a1, diagonal_a1h8},
    {rank_1, file_b, diagonal_a2b1, diagonal_b1h7},
    {rank_1, file_c, diagonal_a3c1, diagonal_c1h6},
    {rank_1, file_d, diagonal_a4d1, diagonal_d1h5},
    {rank_1, file_e, diagonal_a5e1, diagonal_e1h4},
    {rank_1, file_f, diagonal_a6f1, diagonal_f1h3},
    {rank_1, file_g, diagonal_a7g1, diagonal_g1h2},
    {rank_1, file_h, diagonal_a8h1, diagonal_h1},
    {rank_2, file_a, diagonal_a2b1, diagonal_a2g8},
    {rank_2, file_b, diagonal_a3c1, diagonal_a1h8},
    {rank_2, file_c, diagonal_a4d1, diagonal_b1h7},
    {rank_2, file_d, diagonal_a5e1, diagonal_c1h6},
    {rank_2, file_e, diagonal_a6f1, diagonal_d1h5},
    {rank_2, file_f, diagonal_a7g1, diagonal_e1h4},
    {rank_2, file_g, diagonal_a8h1, diagonal_f1h3},
    {rank_2, file_h, diagonal_b8h2, diagonal_g1h2},
    {rank_3, file_a, diagonal_a3c1, diagonal_a3f8},
    {rank_3, file_b, diagonal_a4d1, diagonal_a2g8},
    {rank_3, file_c, diagonal_a5e1, diagonal_a1h8},
    {rank_3, file_d, diagonal_a6f1, diagonal_b1h7},
    {rank_3, file_e, diagonal_a7g1, diagonal_c1h6},
    {rank_3, file_f, diagonal_a8h1, diagonal_d1h5},
    {rank_3, file_g, diagonal_b8h2, diagonal_e1h4},
    {rank_3, file_h, diagonal_c8h3, diagonal_f1h3},
    {rank_4, file_a, diagonal_a4d1, diagonal_a4e8},
    {rank_4, file_b, diagonal_a5e1, diagonal_a3f8},
    {rank_4, file_c, diagonal_a6f1, diagonal_a2g8},
    {rank_4, file_d, diagonal_a7g1, diagonal_a1h8},
    {rank_4, file_e, diagonal_a8h1, diagonal_b1h7},
    {rank_4, file_f, diagonal_b8h2, diagonal_c1h6},
    {rank_4, file_g, diagonal_c8h3, diagonal_d1h5},
    {rank_4, file_h, diagonal_d8h4, diagonal_e1h4},
    {rank_5, file_a, diagonal_a5e1, diagonal_a5d8},
    {rank_5, file_b, diagonal_a6f1, diagonal_a4e8},
    {rank_5, file_c, diagonal_a7g1, diagonal_a3f8},
    {rank_5, file_d, diagonal_a8h1, diagonal_a2g8},
    {rank_5, file_e, diagonal_b8h2, diagonal_a1h8},
    {rank_5, file_f, diagonal_c8h3, diagonal_b1h7},
    {rank_5, file_g, diagonal_d8h4, diagonal_c1h6},
    {rank_5, file_h, diagonal_e8h5, diagonal_d1h5},
    {rank_6, file_a, diagonal_a6f1, diagonal_a6c8},
    {rank_6, file_b, diagonal_a7g1, diagonal_a5d8},
    {rank_6, file_c, diagonal_a8h1, diagonal_a4e8},
    {rank_6, file_d, diagonal_b8h2, diagonal_a3f8},
    {rank_6, file_e, diagonal_c8h3, diagonal_a2g8},
    {rank_6, file_f, diagonal_d8h4, diagonal_a1h8},
    {rank_6, file_g, diagonal_e8h5, diagonal_b1h7},
    {rank_6, file_h, diagonal_f8h6, diagonal_c1h6},
    {rank_7, file_a, diagonal_a7g1, diagonal_a7b8},
    {rank_7, file_b, diagonal_a8h1, diagonal_a6c8},
    {rank_7, file_c, diagonal_b8h2, diagonal_a5d8},
    {rank_7, file_d, diagonal_c8h3, diagonal_a4e8},
    {rank_7, file_e, diagonal_d8h4, diagonal_a3f8},
    {rank_7, file_f, diagonal_e8h5, diagonal_a2g8},
    {rank_7, file_g, diagonal_f8h6, diagonal_a1h8},
    {rank_7, file_h, diagonal_g8h7, diagonal_b1h7},
    {rank_8, file_a, diagonal_a8h1, diagonal_a8},
    {rank_8, file_b, diagonal_b8h2, diagonal_a7b8},
    {rank_8, file_c, diagonal_c8h3, diagonal_a6c8},
    {rank_8, file_d, diagonal_d8h4, diagonal_a5d8},
    {rank_8, file_e, diagonal_e8h5, diagonal_a4e8},
    {rank_8, file_f, diagonal_f8h6, diagonal_a3f8},
    {rank_8, file_g, diagonal_g8h7, diagonal_a2g8},
    {rank_8, file_h, diagonal_h8, diagonal_a1h8}};

// Bit to coordiantes lookup table. This is a simple calculation, but lookup
// table saves on runtime.
const std::pair<uint8_t, uint8_t> bitToCoordinates[N_SQUARES] = {
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7},
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7},
    {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7},
    {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7},
    {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7},
    {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7},
    {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7},
};

// Horizontal slider moves based on bit position. This is faster than
// dynamically calculating it. Cost: ~1.6KB RAM. TODO: populate at runtime.
const uint8_t horizontal_moves[8][256] = {
    {0xFE, 0xFE, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0x1E, 0x1E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0x3E, 0x3E, 0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0x1E, 0x1E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0x7E, 0x7E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0x1E, 0x1E, 0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0x3E, 0x3E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0x1E, 0x1E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xFE, 0xFE, 0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0x1E, 0x1E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0x3E, 0x3E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0x1E, 0x1E, 0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0x7E, 0x7E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0x1E, 0x1E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0x3E, 0x3E, 0x2, 0x2,
     0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2, 0x6,  0x6,  0x2, 0x2,
     0x1E, 0x1E, 0x2, 0x2, 0x6,  0x6,  0x2, 0x2, 0xE,  0xE,  0x2, 0x2,
     0x6,  0x6,  0x2, 0x2},
    {0xFD, 0xFD, 0xFD, 0xFD, 0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,
     0x5,  0x5,  0x5,  0x5,  0x1D, 0x1D, 0x1D, 0x1D, 0x5,  0x5,  0x5,  0x5,
     0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,  0x3D, 0x3D, 0x3D, 0x3D,
     0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,
     0x1D, 0x1D, 0x1D, 0x1D, 0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,
     0x5,  0x5,  0x5,  0x5,  0x7D, 0x7D, 0x7D, 0x7D, 0x5,  0x5,  0x5,  0x5,
     0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,  0x1D, 0x1D, 0x1D, 0x1D,
     0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,
     0x3D, 0x3D, 0x3D, 0x3D, 0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,
     0x5,  0x5,  0x5,  0x5,  0x1D, 0x1D, 0x1D, 0x1D, 0x5,  0x5,  0x5,  0x5,
     0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,  0xFD, 0xFD, 0xFD, 0xFD,
     0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,
     0x1D, 0x1D, 0x1D, 0x1D, 0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,
     0x5,  0x5,  0x5,  0x5,  0x3D, 0x3D, 0x3D, 0x3D, 0x5,  0x5,  0x5,  0x5,
     0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,  0x1D, 0x1D, 0x1D, 0x1D,
     0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,
     0x7D, 0x7D, 0x7D, 0x7D, 0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,
     0x5,  0x5,  0x5,  0x5,  0x1D, 0x1D, 0x1D, 0x1D, 0x5,  0x5,  0x5,  0x5,
     0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,  0x3D, 0x3D, 0x3D, 0x3D,
     0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,  0x5,  0x5,  0x5,  0x5,
     0x1D, 0x1D, 0x1D, 0x1D, 0x5,  0x5,  0x5,  0x5,  0xD,  0xD,  0xD,  0xD,
     0x5,  0x5,  0x5,  0x5},
    {0xFB, 0xFB, 0xFA, 0xFA, 0xFB, 0xFB, 0xFA, 0xFA, 0xB,  0xB,  0xA,  0xA,
     0xB,  0xB,  0xA,  0xA,  0x1B, 0x1B, 0x1A, 0x1A, 0x1B, 0x1B, 0x1A, 0x1A,
     0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,  0x3B, 0x3B, 0x3A, 0x3A,
     0x3B, 0x3B, 0x3A, 0x3A, 0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,
     0x1B, 0x1B, 0x1A, 0x1A, 0x1B, 0x1B, 0x1A, 0x1A, 0xB,  0xB,  0xA,  0xA,
     0xB,  0xB,  0xA,  0xA,  0x7B, 0x7B, 0x7A, 0x7A, 0x7B, 0x7B, 0x7A, 0x7A,
     0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,  0x1B, 0x1B, 0x1A, 0x1A,
     0x1B, 0x1B, 0x1A, 0x1A, 0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,
     0x3B, 0x3B, 0x3A, 0x3A, 0x3B, 0x3B, 0x3A, 0x3A, 0xB,  0xB,  0xA,  0xA,
     0xB,  0xB,  0xA,  0xA,  0x1B, 0x1B, 0x1A, 0x1A, 0x1B, 0x1B, 0x1A, 0x1A,
     0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,  0xFB, 0xFB, 0xFA, 0xFA,
     0xFB, 0xFB, 0xFA, 0xFA, 0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,
     0x1B, 0x1B, 0x1A, 0x1A, 0x1B, 0x1B, 0x1A, 0x1A, 0xB,  0xB,  0xA,  0xA,
     0xB,  0xB,  0xA,  0xA,  0x3B, 0x3B, 0x3A, 0x3A, 0x3B, 0x3B, 0x3A, 0x3A,
     0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,  0x1B, 0x1B, 0x1A, 0x1A,
     0x1B, 0x1B, 0x1A, 0x1A, 0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,
     0x7B, 0x7B, 0x7A, 0x7A, 0x7B, 0x7B, 0x7A, 0x7A, 0xB,  0xB,  0xA,  0xA,
     0xB,  0xB,  0xA,  0xA,  0x1B, 0x1B, 0x1A, 0x1A, 0x1B, 0x1B, 0x1A, 0x1A,
     0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,  0x3B, 0x3B, 0x3A, 0x3A,
     0x3B, 0x3B, 0x3A, 0x3A, 0xB,  0xB,  0xA,  0xA,  0xB,  0xB,  0xA,  0xA,
     0x1B, 0x1B, 0x1A, 0x1A, 0x1B, 0x1B, 0x1A, 0x1A, 0xB,  0xB,  0xA,  0xA,
     0xB,  0xB,  0xA,  0xA},
    {0xF7, 0xF7, 0xF6, 0xF6, 0xF4, 0xF4, 0xF4, 0xF4, 0xF7, 0xF7, 0xF6, 0xF6,
     0xF4, 0xF4, 0xF4, 0xF4, 0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14,
     0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14, 0x37, 0x37, 0x36, 0x36,
     0x34, 0x34, 0x34, 0x34, 0x37, 0x37, 0x36, 0x36, 0x34, 0x34, 0x34, 0x34,
     0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14, 0x17, 0x17, 0x16, 0x16,
     0x14, 0x14, 0x14, 0x14, 0x77, 0x77, 0x76, 0x76, 0x74, 0x74, 0x74, 0x74,
     0x77, 0x77, 0x76, 0x76, 0x74, 0x74, 0x74, 0x74, 0x17, 0x17, 0x16, 0x16,
     0x14, 0x14, 0x14, 0x14, 0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14,
     0x37, 0x37, 0x36, 0x36, 0x34, 0x34, 0x34, 0x34, 0x37, 0x37, 0x36, 0x36,
     0x34, 0x34, 0x34, 0x34, 0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14,
     0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14, 0xF7, 0xF7, 0xF6, 0xF6,
     0xF4, 0xF4, 0xF4, 0xF4, 0xF7, 0xF7, 0xF6, 0xF6, 0xF4, 0xF4, 0xF4, 0xF4,
     0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14, 0x17, 0x17, 0x16, 0x16,
     0x14, 0x14, 0x14, 0x14, 0x37, 0x37, 0x36, 0x36, 0x34, 0x34, 0x34, 0x34,
     0x37, 0x37, 0x36, 0x36, 0x34, 0x34, 0x34, 0x34, 0x17, 0x17, 0x16, 0x16,
     0x14, 0x14, 0x14, 0x14, 0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14,
     0x77, 0x77, 0x76, 0x76, 0x74, 0x74, 0x74, 0x74, 0x77, 0x77, 0x76, 0x76,
     0x74, 0x74, 0x74, 0x74, 0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14,
     0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14, 0x37, 0x37, 0x36, 0x36,
     0x34, 0x34, 0x34, 0x34, 0x37, 0x37, 0x36, 0x36, 0x34, 0x34, 0x34, 0x34,
     0x17, 0x17, 0x16, 0x16, 0x14, 0x14, 0x14, 0x14, 0x17, 0x17, 0x16, 0x16,
     0x14, 0x14, 0x14, 0x14},
    {0xEF, 0xEF, 0xEE, 0xEE, 0xEC, 0xEC, 0xEC, 0xEC, 0xE8, 0xE8, 0xE8, 0xE8,
     0xE8, 0xE8, 0xE8, 0xE8, 0xEF, 0xEF, 0xEE, 0xEE, 0xEC, 0xEC, 0xEC, 0xEC,
     0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0x2F, 0x2F, 0x2E, 0x2E,
     0x2C, 0x2C, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
     0x2F, 0x2F, 0x2E, 0x2E, 0x2C, 0x2C, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28,
     0x28, 0x28, 0x28, 0x28, 0x6F, 0x6F, 0x6E, 0x6E, 0x6C, 0x6C, 0x6C, 0x6C,
     0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x6F, 0x6F, 0x6E, 0x6E,
     0x6C, 0x6C, 0x6C, 0x6C, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
     0x2F, 0x2F, 0x2E, 0x2E, 0x2C, 0x2C, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28,
     0x28, 0x28, 0x28, 0x28, 0x2F, 0x2F, 0x2E, 0x2E, 0x2C, 0x2C, 0x2C, 0x2C,
     0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0xEF, 0xEF, 0xEE, 0xEE,
     0xEC, 0xEC, 0xEC, 0xEC, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8, 0xE8,
     0xEF, 0xEF, 0xEE, 0xEE, 0xEC, 0xEC, 0xEC, 0xEC, 0xE8, 0xE8, 0xE8, 0xE8,
     0xE8, 0xE8, 0xE8, 0xE8, 0x2F, 0x2F, 0x2E, 0x2E, 0x2C, 0x2C, 0x2C, 0x2C,
     0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x2F, 0x2F, 0x2E, 0x2E,
     0x2C, 0x2C, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
     0x6F, 0x6F, 0x6E, 0x6E, 0x6C, 0x6C, 0x6C, 0x6C, 0x68, 0x68, 0x68, 0x68,
     0x68, 0x68, 0x68, 0x68, 0x6F, 0x6F, 0x6E, 0x6E, 0x6C, 0x6C, 0x6C, 0x6C,
     0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x2F, 0x2F, 0x2E, 0x2E,
     0x2C, 0x2C, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
     0x2F, 0x2F, 0x2E, 0x2E, 0x2C, 0x2C, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28,
     0x28, 0x28, 0x28, 0x28},
    {0xDF, 0xDF, 0xDE, 0xDE, 0xDC, 0xDC, 0xDC, 0xDC, 0xD8, 0xD8, 0xD8, 0xD8,
     0xD8, 0xD8, 0xD8, 0xD8, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
     0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xDF, 0xDF, 0xDE, 0xDE,
     0xDC, 0xDC, 0xDC, 0xDC, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
     0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
     0xD0, 0xD0, 0xD0, 0xD0, 0x5F, 0x5F, 0x5E, 0x5E, 0x5C, 0x5C, 0x5C, 0x5C,
     0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x50, 0x50, 0x50, 0x50,
     0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50,
     0x5F, 0x5F, 0x5E, 0x5E, 0x5C, 0x5C, 0x5C, 0x5C, 0x58, 0x58, 0x58, 0x58,
     0x58, 0x58, 0x58, 0x58, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50,
     0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0xDF, 0xDF, 0xDE, 0xDE,
     0xDC, 0xDC, 0xDC, 0xDC, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8,
     0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
     0xD0, 0xD0, 0xD0, 0xD0, 0xDF, 0xDF, 0xDE, 0xDE, 0xDC, 0xDC, 0xDC, 0xDC,
     0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD0, 0xD0, 0xD0, 0xD0,
     0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,
     0x5F, 0x5F, 0x5E, 0x5E, 0x5C, 0x5C, 0x5C, 0x5C, 0x58, 0x58, 0x58, 0x58,
     0x58, 0x58, 0x58, 0x58, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50,
     0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x5F, 0x5F, 0x5E, 0x5E,
     0x5C, 0x5C, 0x5C, 0x5C, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
     0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50,
     0x50, 0x50, 0x50, 0x50},
    {0xBF, 0xBF, 0xBE, 0xBE, 0xBC, 0xBC, 0xBC, 0xBC, 0xB8, 0xB8, 0xB8, 0xB8,
     0xB8, 0xB8, 0xB8, 0xB8, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,
     0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xBF, 0xBF, 0xBE, 0xBE, 0xBC, 0xBC, 0xBC, 0xBC,
     0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB0, 0xB0, 0xB0, 0xB0,
     0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xBF, 0xBF, 0xBE, 0xBE,
     0xBC, 0xBC, 0xBC, 0xBC, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8,
     0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,
     0xB0, 0xB0, 0xB0, 0xB0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xBF, 0xBF, 0xBE, 0xBE, 0xBC, 0xBC, 0xBC, 0xBC, 0xB8, 0xB8, 0xB8, 0xB8,
     0xB8, 0xB8, 0xB8, 0xB8, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0,
     0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0,
     0xA0, 0xA0, 0xA0, 0xA0},
    {0x7F, 0x7F, 0x7E, 0x7E, 0x7C, 0x7C, 0x7C, 0x7C, 0x78, 0x78, 0x78, 0x78,
     0x78, 0x78, 0x78, 0x78, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70,
     0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x60, 0x60, 0x60, 0x60,
     0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
     0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
     0x60, 0x60, 0x60, 0x60, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7F, 0x7F, 0x7E, 0x7E,
     0x7C, 0x7C, 0x7C, 0x7C, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
     0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70,
     0x70, 0x70, 0x70, 0x70, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
     0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
     0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
     0x40, 0x40, 0x40, 0x40}};

// Masks that include both horizontal and vertical lines, excluding outer
// squares and the repective bit. Used for doing magic calculations.
const uint64_t rookMagicMasks[N_SQUARES] = {
    0x101010101017E,    0x202020202027C,    0x404040404047A,
    0x8080808080876,    0x1010101010106E,   0x2020202020205E,
    0x4040404040403E,   0x8080808080807E,   0x1010101017E00,
    0x2020202027C00,    0x4040404047A00,    0x8080808087600,
    0x10101010106E00,   0x20202020205E00,   0x40404040403E00,
    0x80808080807E00,   0x10101017E0100,    0x20202027C0200,
    0x40404047A0400,    0x8080808760800,    0x101010106E1000,
    0x202020205E2000,   0x404040403E4000,   0x808080807E8000,
    0x101017E010100,    0x202027C020200,    0x404047A040400,
    0x8080876080800,    0x1010106E101000,   0x2020205E202000,
    0x4040403E404000,   0x8080807E808000,   0x1017E01010100,
    0x2027C02020200,    0x4047A04040400,    0x8087608080800,
    0x10106E10101000,   0x20205E20202000,   0x40403E40404000,
    0x80807E80808000,   0x17E0101010100,    0x27C0202020200,
    0x47A0404040400,    0x8760808080800,    0x106E1010101000,
    0x205E2020202000,   0x403E4040404000,   0x807E8080808000,
    0x7E010101010100,   0x7C020202020200,   0x7A040404040400,
    0x76080808080800,   0x6E101010101000,   0x5E202020202000,
    0x3E404040404000,   0x7E808080808000,   0x7E01010101010100,
    0x7C02020202020200, 0x7A04040404040400, 0x7608080808080800,
    0x6E10101010101000, 0x5E20202020202000, 0x3E40404040404000,
    0x7E80808080808000};

const uint64_t rookMagicNumbers[N_SQUARES] = {
    0x5080008020400014, 0x6004012000081100, 0x8800A8020001001,
    0x200024005920020,  0x8A000216011A0008, 0x8024010482000400,
    0x48008008020122,   0x100025225000086,  0x9080080C80204100,
    0x140020024003,     0x2080008114080,    0x1001702009001001,
    0x4000180402100100, 0x148040008140,     0x202101008240440,
    0x400900804C00020,  0x8004880010048,    0x80106000204000,
    0x806001102208014,  0x94020C800040008,  0x8000604801080082,
    0x108022008208900,  0x14102E021030,     0x2A000410001,
    0x20012008001004,   0x400028040848,     0x80880404046058,
    0x98400420022030,   0x208200011004,     0x1000804060020020,
    0x200060806002004,  0x400024820110182,  0x40081020800080,
    0x80081405200A010C, 0x81100801020020,   0x220200201340404,
    0x4200080040500548, 0x80040001002000A0, 0x2200140081104,
    0x111010074821148,  0xA050088100100400, 0x890022101012,
    0x40100290584800,   0x21100004601100,   0x4000408224011,
    0x9008320011,       0x200008511002042,  0x20808020410002,
    0x80002400400848,   0x40811028220010,   0x280104009200100,
    0x9088010004800210, 0x108090521005050,  0x120054002080,
    0xA0800CA202004100, 0x5200916800444,    0x30A081C2010812,
    0x202008008204902,  0x2040902100094441, 0x4004184200041002,
    0x3000210010082183, 0x18104900040001C1, 0x304081000810044,
    0x2290010424009142};

#endif // DDS_CHESS_ENGINE_CONSTANTS_H
