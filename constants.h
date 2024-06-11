#pragma once

#include <stdint.h>
#include <utility>

// Edge bits all set.
const uint64_t OUTER_BITS = 0xFF818181818181FF;
const uint64_t INNER_BITS = ~OUTER_BITS;

// 8 x 8 chessboard.
const uint8_t N_SQUARES = 64;

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

const uint8_t N_BISHOP_BLOCKERS = 9;
const uint8_t N_ROOK_BLOCKERS = 12;
const uint16_t N_BISHOP_BLOCKERS_PERMUTATIONS = 1 << N_BISHOP_BLOCKERS;
const uint16_t N_ROOK_BLOCKERS_PERMUTATIONS = 1 << N_ROOK_BLOCKERS;

const uint64_t bishop_magic_masks[N_SQUARES] = {
    0x40201008040200, 0x402010080400,   0x4020100A00,     0x40221400,
    0x2442800,        0x204085000,      0x20408102000,    0x2040810204000,
    0x20100804020000, 0x40201008040000, 0x4020100A0000,   0x4022140000,
    0x244280000,      0x20408500000,    0x2040810200000,  0x4081020400000,
    0x10080402000200, 0x20100804000400, 0x4020100A000A00, 0x402214001400,
    0x24428002800,    0x2040850005000,  0x4081020002000,  0x8102040004000,
    0x8040200020400,  0x10080400040800, 0x20100A000A1000, 0x40221400142200,
    0x2442800284400,  0x4085000500800,  0x8102000201000,  0x10204000402000,
    0x4020002040800,  0x8040004081000,  0x100A000A102000, 0x22140014224000,
    0x44280028440200, 0x8500050080400,  0x10200020100800, 0x20400040201000,
    0x2000204081000,  0x4000408102000,  0xA000A10204000,  0x14001422400000,
    0x28002844020000, 0x50005008040200, 0x20002010080400, 0x40004020100800,
    0x20408102000,    0x40810204000,    0xA1020400000,    0x142240000000,
    0x284402000000,   0x500804020000,   0x201008040200,   0x402010080400,
    0x2040810204000,  0x4081020400000,  0xA102040000000,  0x14224000000000,
    0x28440200000000, 0x50080402000000, 0x20100804020000, 0x40201008040200};

const uint64_t bishop_magic_numbers[N_SQUARES] = {
    0x6110003449004200, 0x808100824410410,  0x482280011881802,
    0x801230002008400,  0x8048208800000001, 0x420040A090200800,
    0x1000820800822300, 0x1100028058A01040, 0x320004808293012,
    0x4440402C9050,     0x92100042000D00,   0x8002841800326,
    0x400010882014002,  0xA00D8020020021,   0x289001048005000,
    0x8010141014100208, 0x6000440808C2A,    0x40402000820C2102,
    0x40082080088C,     0x12A210104000400,  0x298400200000,
    0x100222A04140180,  0x41040A0242106180, 0x1801000068801202,
    0x548583102002108,  0x200008012310,     0x240980020820004,
    0x3020080010820440, 0x201001001004001,  0x40403002011008,
    0x2002800882482101, 0x10040050C00C4201, 0x8804100400808380,
    0x8063010E10020,    0x66005047208D01,   0x4001040400080210,
    0x4140002020020080, 0x9000825461001000, 0x212215440020,
    0x13401004100200,   0x8C0A0A010C0242,   0xC222240842000808,
    0x4A00020850212860, 0x1802400200100,    0x80008100821C100,
    0x8840110021000209, 0x200600A801020010, 0x122888905000002,
    0x2000241001180000, 0x5070188440600201, 0x4100028010084008,
    0xAE02000C14041048, 0x890002E60402400,  0x210482001181400,
    0x408800146010,     0x80020220E2820002, 0x4650140020140200,
    0x11249D00110,      0x706A508040A01,    0x288002082008B0,
    0x100000040040820,  0x40002088010044,   0x3A0090001600,
    0x200610A00052D};

// Masks that include both horizontal and vertical lines, excluding outer
// squares and the repective bit. Used for doing magic calculations.
const uint64_t rook_magic_masks[N_SQUARES] = {
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

const uint64_t rook_magic_numbers[N_SQUARES] = {
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

const uint64_t king_moves[N_SQUARES] = {0x302,
                                        0x705,
                                        0xE0A,
                                        0x1C14,
                                        0x3828,
                                        0x7050,
                                        0xE0A0,
                                        0xC040,
                                        0x30203,
                                        0x70507,
                                        0xE0A0E,
                                        0x1C141C,
                                        0x382838,
                                        0x705070,
                                        0xE0A0E0,
                                        0xC040C0,
                                        0x3020300,
                                        0x7050700,
                                        0xE0A0E00,
                                        0x1C141C00,
                                        0x38283800,
                                        0x70507000,
                                        0xE0A0E000,
                                        0xC040C000,
                                        0x302030000,
                                        0x705070000,
                                        0xE0A0E0000,
                                        0x1C141C0000,
                                        0x3828380000,
                                        0x7050700000,
                                        0xE0A0E00000,
                                        0xC040C00000,
                                        0x30203000000,
                                        0x70507000000,
                                        0xE0A0E000000,
                                        0x1C141C000000,
                                        0x382838000000,
                                        0x705070000000,
                                        0xE0A0E0000000,
                                        0xC040C0000000,
                                        0x3020300000000,
                                        0x7050700000000,
                                        0xE0A0E00000000,
                                        0x1C141C00000000,
                                        0x38283800000000,
                                        0x70507000000000,
                                        0xE0A0E000000000,
                                        0xC040C000000000,
                                        0x302030000000000,
                                        0x705070000000000,
                                        0xE0A0E0000000000,
                                        0x1C141C0000000000,
                                        0x3828380000000000,
                                        0x7050700000000000,
                                        0xE0A0E00000000000,
                                        0xC040C00000000000,
                                        0x203000000000000,
                                        0x507000000000000,
                                        0xA0E000000000000,
                                        0x141C000000000000,
                                        0x2838000000000000,
                                        0x5070000000000000,
                                        0xA0E0000000000000,
                                        0x40C0000000000000};

const uint64_t knight_moves[N_SQUARES] = {0x20400,
                                          0x50800,
                                          0xA1100,
                                          0x142200,
                                          0x284400,
                                          0x508800,
                                          0xA01000,
                                          0x402000,
                                          0x2040004,
                                          0x5080008,
                                          0xA110011,
                                          0x14220022,
                                          0x28440044,
                                          0x50880088,
                                          0xA0100010,
                                          0x40200020,
                                          0x204000402,
                                          0x508000805,
                                          0xA1100110A,
                                          0x1422002214,
                                          0x2844004428,
                                          0x5088008850,
                                          0xA0100010A0,
                                          0x4020002040,
                                          0x20400040200,
                                          0x50800080500,
                                          0xA1100110A00,
                                          0x142200221400,
                                          0x284400442800,
                                          0x508800885000,
                                          0xA0100010A000,
                                          0x402000204000,
                                          0x2040004020000,
                                          0x5080008050000,
                                          0xA1100110A0000,
                                          0x14220022140000,
                                          0x28440044280000,
                                          0x50880088500000,
                                          0xA0100010A00000,
                                          0x40200020400000,
                                          0x204000402000000,
                                          0x508000805000000,
                                          0xA1100110A000000,
                                          0x1422002214000000,
                                          0x2844004428000000,
                                          0x5088008850000000,
                                          0xA0100010A0000000,
                                          0x4020002040000000,
                                          0x400040200000000,
                                          0x800080500000000,
                                          0x1100110A00000000,
                                          0x2200221400000000,
                                          0x4400442800000000,
                                          0x8800885000000000,
                                          0x100010A000000000,
                                          0x2000204000000000,
                                          0x4020000000000,
                                          0x8050000000000,
                                          0x110A0000000000,
                                          0x22140000000000,
                                          0x44280000000000,
                                          0x88500000000000,
                                          0x10A00000000000,
                                          0x20400000000000};
