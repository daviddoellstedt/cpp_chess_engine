#ifndef DDS_CHESS_ENGINE_CONSTANTS_H
#define DDS_CHESS_ENGINE_CONSTANTS_H

#include <stdint.h>
#include <unordered_map>

// 8 x 8 chessboard.
const uint8_t N_SQUARES = 64;

// Mask of the possible ways a knight can move.
const uint64_t KNIGHT_MOVES = 0x5088008850;

// Mask of the possible ways a king can move.
const uint64_t KING_MOVES = 0xE0A0E0;

// Mask of entirely filled board.
const uint64_t FILLED = ~0;

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

// Down/right diagonols.
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

// Up/right diagonols.
const uint64_t diagonol_a8 = 0x100000000000000;
const uint64_t diagonol_a7b8 = 0x201000000000000;
const uint64_t diagonol_a6c8 = 0x402010000000000;
const uint64_t diagonol_a5d8 = 0x804020100000000;
const uint64_t diagonol_a4e8 = 0x1008040201000000;
const uint64_t diagonol_a3f8 = 0x2010080402010000;
const uint64_t diagonol_a2g8 = 0x4020100804020100;
const uint64_t diagonol_a1h8 = 0x8040201008040201;
const uint64_t diagonol_b1h7 = 0x80402010080402;
const uint64_t diagonol_c1h6 = 0x804020100804;
const uint64_t diagonol_d1h5 = 0x8040201008;
const uint64_t diagonol_e1h4 = 0x80402010;
const uint64_t diagonol_f1h3 = 0x804020;
const uint64_t diagonol_g1h2 = 0x8040;
const uint64_t diagonol_h1 = 0x80;

enum directional_indices {
  RANKS = 0,
  FILES = 1,
  DIAGONOLS_DOWN_RIGHT = 2,
  DIAGONOLS_UP_RIGHT = 3,
  N_DIRECTIONS = 4
};

// Stores the horizontal, vertical, and diagonol masks for each square.
const uint64_t directional_mask[N_SQUARES][N_DIRECTIONS] = {
    {rank_1, file_a, diagonal_a1, diagonol_a1h8},
    {rank_1, file_b, diagonal_a2b1, diagonol_b1h7},
    {rank_1, file_c, diagonal_a3c1, diagonol_c1h6},
    {rank_1, file_d, diagonal_a4d1, diagonol_d1h5},
    {rank_1, file_e, diagonal_a5e1, diagonol_e1h4},
    {rank_1, file_f, diagonal_a6f1, diagonol_f1h3},
    {rank_1, file_g, diagonal_a7g1, diagonol_g1h2},
    {rank_1, file_h, diagonal_a8h1, diagonol_h1},
    {rank_2, file_a, diagonal_a2b1, diagonol_a2g8},
    {rank_2, file_b, diagonal_a3c1, diagonol_a1h8},
    {rank_2, file_c, diagonal_a4d1, diagonol_b1h7},
    {rank_2, file_d, diagonal_a5e1, diagonol_c1h6},
    {rank_2, file_e, diagonal_a6f1, diagonol_d1h5},
    {rank_2, file_f, diagonal_a7g1, diagonol_e1h4},
    {rank_2, file_g, diagonal_a8h1, diagonol_f1h3},
    {rank_2, file_h, diagonal_b8h2, diagonol_g1h2},
    {rank_3, file_a, diagonal_a3c1, diagonol_a3f8},
    {rank_3, file_b, diagonal_a4d1, diagonol_a2g8},
    {rank_3, file_c, diagonal_a5e1, diagonol_a1h8},
    {rank_3, file_d, diagonal_a6f1, diagonol_b1h7},
    {rank_3, file_e, diagonal_a7g1, diagonol_c1h6},
    {rank_3, file_f, diagonal_a8h1, diagonol_d1h5},
    {rank_3, file_g, diagonal_b8h2, diagonol_e1h4},
    {rank_3, file_h, diagonal_c8h3, diagonol_f1h3},
    {rank_4, file_a, diagonal_a4d1, diagonol_a4e8},
    {rank_4, file_b, diagonal_a5e1, diagonol_a3f8},
    {rank_4, file_c, diagonal_a6f1, diagonol_a2g8},
    {rank_4, file_d, diagonal_a7g1, diagonol_a1h8},
    {rank_4, file_e, diagonal_a8h1, diagonol_b1h7},
    {rank_4, file_f, diagonal_b8h2, diagonol_c1h6},
    {rank_4, file_g, diagonal_c8h3, diagonol_d1h5},
    {rank_4, file_h, diagonal_d8h4, diagonol_e1h4},
    {rank_5, file_a, diagonal_a5e1, diagonol_a5d8},
    {rank_5, file_b, diagonal_a6f1, diagonol_a4e8},
    {rank_5, file_c, diagonal_a7g1, diagonol_a3f8},
    {rank_5, file_d, diagonal_a8h1, diagonol_a2g8},
    {rank_5, file_e, diagonal_b8h2, diagonol_a1h8},
    {rank_5, file_f, diagonal_c8h3, diagonol_b1h7},
    {rank_5, file_g, diagonal_d8h4, diagonol_c1h6},
    {rank_5, file_h, diagonal_e8h5, diagonol_d1h5},
    {rank_6, file_a, diagonal_a6f1, diagonol_a6c8},
    {rank_6, file_b, diagonal_a7g1, diagonol_a5d8},
    {rank_6, file_c, diagonal_a8h1, diagonol_a4e8},
    {rank_6, file_d, diagonal_b8h2, diagonol_a3f8},
    {rank_6, file_e, diagonal_c8h3, diagonol_a2g8},
    {rank_6, file_f, diagonal_d8h4, diagonol_a1h8},
    {rank_6, file_g, diagonal_e8h5, diagonol_b1h7},
    {rank_6, file_h, diagonal_f8h6, diagonol_c1h6},
    {rank_7, file_a, diagonal_a7g1, diagonol_a7b8},
    {rank_7, file_b, diagonal_a8h1, diagonol_a6c8},
    {rank_7, file_c, diagonal_b8h2, diagonol_a5d8},
    {rank_7, file_d, diagonal_c8h3, diagonol_a4e8},
    {rank_7, file_e, diagonal_d8h4, diagonol_a3f8},
    {rank_7, file_f, diagonal_e8h5, diagonol_a2g8},
    {rank_7, file_g, diagonal_f8h6, diagonol_a1h8},
    {rank_7, file_h, diagonal_g8h7, diagonol_b1h7},
    {rank_8, file_a, diagonal_a8h1, diagonol_a8},
    {rank_8, file_b, diagonal_b8h2, diagonol_a7b8},
    {rank_8, file_c, diagonal_c8h3, diagonol_a6c8},
    {rank_8, file_d, diagonal_d8h4, diagonol_a5d8},
    {rank_8, file_e, diagonal_e8h5, diagonol_a4e8},
    {rank_8, file_f, diagonal_f8h6, diagonol_a3f8},
    {rank_8, file_g, diagonal_g8h7, diagonol_a2g8},
    {rank_8, file_h, diagonol_h1, diagonol_a1h8}};

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

std::unordered_map<uint64_t, uint8_t> log2_map = {
    {(uint64_t)1 << 0, 0},   {(uint64_t)1 << 1, 1},   {(uint64_t)1 << 2, 2},
    {(uint64_t)1 << 3, 3},   {(uint64_t)1 << 4, 4},   {(uint64_t)1 << 5, 5},
    {(uint64_t)1 << 6, 6},   {(uint64_t)1 << 7, 7},   {(uint64_t)1 << 8, 8},
    {(uint64_t)1 << 9, 9},   {(uint64_t)1 << 10, 10}, {(uint64_t)1 << 11, 11},
    {(uint64_t)1 << 12, 12}, {(uint64_t)1 << 13, 13}, {(uint64_t)1 << 14, 14},
    {(uint64_t)1 << 15, 15}, {(uint64_t)1 << 16, 16}, {(uint64_t)1 << 17, 17},
    {(uint64_t)1 << 18, 18}, {(uint64_t)1 << 19, 19}, {(uint64_t)1 << 20, 20},
    {(uint64_t)1 << 21, 21}, {(uint64_t)1 << 22, 22}, {(uint64_t)1 << 23, 23},
    {(uint64_t)1 << 24, 24}, {(uint64_t)1 << 25, 25}, {(uint64_t)1 << 26, 26},
    {(uint64_t)1 << 27, 27}, {(uint64_t)1 << 28, 28}, {(uint64_t)1 << 29, 29},
    {(uint64_t)1 << 30, 30}, {(uint64_t)1 << 31, 31}, {(uint64_t)1 << 32, 32},
    {(uint64_t)1 << 33, 33}, {(uint64_t)1 << 34, 34}, {(uint64_t)1 << 35, 35},
    {(uint64_t)1 << 36, 36}, {(uint64_t)1 << 37, 37}, {(uint64_t)1 << 38, 38},
    {(uint64_t)1 << 39, 39}, {(uint64_t)1 << 40, 40}, {(uint64_t)1 << 41, 41},
    {(uint64_t)1 << 42, 42}, {(uint64_t)1 << 43, 43}, {(uint64_t)1 << 44, 44},
    {(uint64_t)1 << 45, 45}, {(uint64_t)1 << 46, 46}, {(uint64_t)1 << 47, 47},
    {(uint64_t)1 << 48, 48}, {(uint64_t)1 << 49, 49}, {(uint64_t)1 << 50, 50},
    {(uint64_t)1 << 51, 51}, {(uint64_t)1 << 52, 52}, {(uint64_t)1 << 53, 53},
    {(uint64_t)1 << 54, 54}, {(uint64_t)1 << 55, 55}, {(uint64_t)1 << 56, 56},
    {(uint64_t)1 << 57, 57}, {(uint64_t)1 << 58, 58}, {(uint64_t)1 << 59, 59},
    {(uint64_t)1 << 60, 60}, {(uint64_t)1 << 61, 61}, {(uint64_t)1 << 62, 62},
    {(uint64_t)1 << 63, 63}};

#endif // DDS_CHESS_ENGINE_CONSTANTS_H
