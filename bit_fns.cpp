//
// Created by David Doellstedt on 5/12/20.
//
#include "bit_fns.h"
#include "Helper_functions.h"
#include "Players.h"
#include "constants.h"
#include <bitset>
#include <chrono> // for high_resolution_clock
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <regex>
#include <stdint.h>
#include <string>

struct AI_return {
  Move move;
  double value;
};

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

void logErrorAndExit(std::string error_message) {
  std::cout << error_message << std::endl;
  // exit(1);
}

uint8_t moveToX1(Move move) { return move.data & X_INITIAL; }

uint8_t moveToY1(Move move) { return (move.data & Y_INITIAL) >> 3; }

uint8_t moveToX2(Move move) { return (move.data & X_FINAL) >> 6; }

uint8_t moveToY2(Move move) { return (move.data & Y_FINAL) >> 9; }

SpecialMove moveToSpecial(Move move) {
  return (SpecialMove)((move.data & SPECIAL) >> 12);
}

std::string specialMoveToString(SpecialMove special_move) {
  switch (special_move) {
  case NONE:
  case PAWN_PUSH_2:
    return "";
  case CASTLE_KINGSIDE:
    return "Kingside Castle";
  case CASTLE_QUEENSIDE:
    return "Queenside Castle";
  case PROMOTION_QUEEN:
    return "Promotion (Queen)";
  case PROMOTION_ROOK:
    return "Promotion (Rook)";
  case PROMOTION_KNIGHT:
    return "Promotion (Knight)";
  case PROMOTION_BISHOP:
    return "Promotion (Bishop)";
  case EN_PASSANT:
    return "En Passant";
  default:
    logErrorAndExit("ERROR: Unknown value received for special_move.");
    return "";
  }
}

std::string moveToString(Move move) {
  std::string result = "";
  result += (char)('a' + moveToY1(move)) + std::to_string(moveToX1(move) + 1) +
            (char)('a' + moveToY2(move)) + std::to_string(moveToX2(move) + 1) +
            " " + specialMoveToString(moveToSpecial(move));
  return result;
}

Move coordinatesToMove(std::pair<uint8_t, uint8_t> initial,
                       std::pair<uint8_t, uint8_t> final) {
  Move move;
  move.data = initial.first;
  move.data |= (initial.second << 3);
  move.data |= (final.first << 6);
  move.data |= (final.second << 9);
  return move;
}

void updateSpecialMove(Move &move, SpecialMove special_move) {
  move.data &= ~SPECIAL;
  move.data |= (special_move << 12);
}

uint64_t generateWhiteOccupiedBitboard(const GameState &gamestate){
  return gamestate.white.pawn | gamestate.white.rook | gamestate.white.knight |
         gamestate.white.bishop | gamestate.white.queen | gamestate.white.king;
}

uint64_t generateBlackOccupiedBitboard(const GameState &gamestate){
  return gamestate.black.pawn | gamestate.black.rook | gamestate.black.knight |
         gamestate.black.bishop | gamestate.black.queen | gamestate.black.king;
}

uint64_t findLowestSetBitValue(uint64_t bitboard) {
  return bitboard & ~(bitboard - 1);
}

void clearLowestSetBit(uint64_t &bitboard) { bitboard &= (bitboard - 1); }

// Defined behavior for arguments that only have 1 set bit.
uint8_t findSetBit(uint64_t bitboard) {
  return 63 - (bitboard ? __builtin_clzll(bitboard) : 255);
}

uint8_t countSetBits(uint64_t bitboard) {
  uint8_t count = 0;
  while (bitboard) {
    clearLowestSetBit(bitboard);
    count++;
  }
  return count;
}

/** Printing the board to the command line.
 *
 * arguments: the 12 bitboards for the all the pieces
 */
void print_board(const GameState gamestate) {
  char grid[8][8];

  for (int8_t row = 7; row >= 0; row--) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t bit = ((7 - row) * 8) + col;
      if (gamestate.white.pawn & (1ULL << bit)) {
        grid[row][col] = 'P';
      } else if (gamestate.white.rook & (1ULL << bit)) {
        grid[row][col] = 'R';
      } else if (gamestate.white.knight & (1ULL << bit)) {
        grid[row][col] = 'N';
      } else if (gamestate.white.bishop & (1ULL << bit)) {
        grid[row][col] = 'B';
      } else if (gamestate.white.queen & (1ULL << bit)) {
        grid[row][col] = 'Q';
      } else if (gamestate.white.king & (1ULL << bit)) {
        grid[row][col] = 'K';
      } else if (gamestate.black.pawn & (1ULL << bit)) {
        grid[row][col] = 'p';
      } else if (gamestate.black.rook & (1ULL << bit)) {
        grid[row][col] = 'r';
      } else if (gamestate.black.knight & (1ULL << bit)) {
        grid[row][col] = 'n';
      } else if (gamestate.black.bishop & (1ULL << bit)) {
        grid[row][col] = 'b';
      } else if (gamestate.black.queen & (1ULL << bit)) {
        grid[row][col] = 'q';
      } else if (gamestate.black.king & (1ULL << bit)) {
        grid[row][col] = 'k';
      } else {
        grid[row][col] = ' ';
      }
    }
  }

  std::string dividing_line =
      "|---|-----|-----|-----|-----|-----|-----|-----|-----|";

  std::string line;
  for (uint8_t i = 0; i <= 7; i++) {
    std::cout << dividing_line << std::endl;
    line = "| " + std::to_string(8 - i) + " |  ";
    for (uint8_t j = 0; j < 8; j++) {
      line.push_back(grid[i][j]);
      if (j != 7) {
        line += "  |  ";
      }
    }
    std::cout << line + "  |" << std::endl;
  }
  std::cout << dividing_line << std::endl;

  std::cout << "|   |  a  |  b  |  c  |  d  |  e  |  f  |  g  |  h  |"
            << std::endl;
  std::cout << dividing_line << std::endl;
}

/** Function that can generate the possible moves a slider piece can make in the
 * horizontal direction
 *
 * @param piece: bitboard representing a horizontal sliding piece
 * @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @return horiz_moves: bitboard of horizontal moves
 */
uint64_t h_moves(uint64_t piece, uint8_t sl_bit, uint64_t OCCUPIED) {
  uint64_t horiz_moves =
      (((OCCUPIED)-2 * piece) ^ rev(rev(OCCUPIED) - 2 * rev(piece))) &
      directional_mask[sl_bit][RANKS];
  return horiz_moves;
}

/** Function that can generate the possible moves a slider piece can make in the
 * vertical direction
 *
 * @param piece: bitboard representing a vertical sliding piece
 * @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @return vert_moves: bitboard of vertical moves
 */
uint64_t v_moves(uint64_t piece, uint8_t sl_bit, uint64_t OCCUPIED) {
  uint64_t vert_moves =
      (((OCCUPIED & directional_mask[sl_bit][FILES]) - 2 * piece) ^
       rev(rev(OCCUPIED & directional_mask[sl_bit][FILES]) - 2 * rev(piece))) &
      directional_mask[sl_bit][FILES];
  return vert_moves;
}

/** Function that compiles the horizontal and vertical moves bitboards and
 * handles a case where we check for unsafe moves for the king.
 *
 * @param piece: bitboard representing a horizontal/vertical sliding piece
 * @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @param unsafe_calc: flag used to see if we need to remove the enemy king from
 * the occupied spaces (see inline comment for more details)
 * @param K: bitboard representing king location. (see inline comment in the
 * function for more details)
 * @return bitboard of horizontal and vertical moves
 */
uint64_t h_v_moves(uint64_t piece, uint8_t sl_bit, uint64_t OCCUPIED,
                   bool unsafe_calc, uint64_t K) {

  // this line is used in the case where we need to generate zones for the king
  // that are unsafe. If the king is in the attack zone of a horizontal/vertical
  // slider, we want to remove the king from the calculation. Because a move of
  // the king, that still lies in the path of attack (further away from the
  // slider) is still an "unsafe" move.
  if (unsafe_calc) {
    OCCUPIED &= ~K;
  }

  return h_moves(piece, sl_bit, OCCUPIED) | v_moves(piece, sl_bit, OCCUPIED);
}

/** Function that can generate the possible moves a slider piece can make in the
 * (down, right) and (up, left) diagonol
 *
 * @param piece: bitboard representing a diagonal sliding piece
 * @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @return ddr_moves: bitboard of (down, right) and (up, left) moves
 */
uint64_t ddr_moves(uint64_t piece, uint8_t sl_bit, uint64_t OCCUPIED) {
  uint64_t ddr_moves =
      (((OCCUPIED & directional_mask[sl_bit][DIAGONOLS_DOWN_RIGHT]) -
        2 * piece) ^
       rev(rev(OCCUPIED & directional_mask[sl_bit][DIAGONOLS_DOWN_RIGHT]) -
           2 * rev(piece))) &
      directional_mask[sl_bit][DIAGONOLS_DOWN_RIGHT];
  return ddr_moves;
}

/** Function that can generate the possible moves a slider piece can make in the
 * (up, right) and (down, left) diagonol
 *
 * @param piece: bitboard representing a diagonal sliding piece
 * @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @return dur_moves: bitboard of (up, right) and (down, left) moves
 */
uint64_t dur_moves(uint64_t piece, uint8_t sl_bit, uint64_t OCCUPIED) {
  uint64_t dur_moves =
      (((OCCUPIED & directional_mask[sl_bit][DIAGONOLS_UP_RIGHT]) - 2 * piece) ^
       rev(rev(OCCUPIED & directional_mask[sl_bit][DIAGONOLS_UP_RIGHT]) -
           2 * rev(piece))) &
      directional_mask[sl_bit][DIAGONOLS_UP_RIGHT];
  return dur_moves;
}

/** Function that compiles the diagonal moves bitboards and handles a case where
 * we check for unsafe moves for the king.
 *
 * @param piece: bitboard representing a diagonal sliding piece
 * @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @param unsafe_calc: flag used to see if we need to remove the enemy king from
 * the occupied spaces (see inline comment for more details)
 * @param K: bitboard representing king location. (see inline comment in the
 * function for more details)
 * @return bitboard of all diagonal moves
 */
uint64_t diag_moves(uint64_t piece, uint8_t sl_bit, uint64_t OCCUPIED,
                    bool unsafe_calc, uint64_t K) {

  // this line is used in the case where we need to generate zones for the king
  // that are unsafe. If the king is in the attack zone of a diagonal slider, we
  // want to remove the king from the calculation. Because a move of the king,
  // that still lies in the path of attack (further away from the slider) is
  // still an "unsafe" move.
  if (unsafe_calc) {
    OCCUPIED &= ~K;
  }
  return ddr_moves(piece, sl_bit, OCCUPIED) |
         dur_moves(piece, sl_bit, OCCUPIED);
}

/** Function that returns a bitboard mask of the straight line between two
 * pieces. Inputs are guaranteed to be colinear at a diagonal or orthogonal
 * perspective. More or less a lookup table.
 *
 * @param p1: first piece
 * @param p2: second piece
 * @return bitboard mask of rank/file/diagonal connection between the two pieces
 */
uint64_t get_mask(uint64_t p1, uint64_t p2) {
  uint8_t k_bit = findSetBit(p2);
  uint8_t p_bit = findSetBit(p1);

  for (uint8_t dir = 0; dir < N_DIRECTIONS; dir++) {
    if (directional_mask[k_bit][dir] == directional_mask[p_bit][dir]) {
      return directional_mask[k_bit][dir];
    }
  }

  logErrorAndExit("ERROR: The two arguments provided are not colinear.");
  return 0;
}

/** Function that returns a bitboard of pieces that are pinned. Pieces that if
 * you were to move them, would place your own King in check (an illegal move).
 *
 * @params Piece bitboards (note that the E preceding a Piece letter denotes the
 * enemies piece, ex: EB = enemy bishop)
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @return PINNED: bitboard of all pinned pieces for a color
 */
uint64_t get_pinned_pieces(uint64_t K, uint64_t P, uint64_t EQ, uint64_t EB,
                           uint64_t ER, uint64_t OCCUPIED, uint64_t &E_P,
                           bool white_to_move) {
  uint64_t PINNED = 0;
  uint64_t K_slider = 0;
  uint8_t k_bit = findSetBit(K);

  // Horizontal check.
  K_slider = h_moves(K, k_bit, OCCUPIED);
  uint64_t EHV = EQ | ER;
  while (EHV) {
    uint64_t bb = findLowestSetBitValue(EHV);
    uint64_t H_moves = h_moves(bb, findSetBit(bb), OCCUPIED);

    // Check for special en passant pins.
    uint64_t ep_pawn = white_to_move ? E_P >> 8 : E_P << 8;
    uint64_t p_can_capture_ep_r =
        P & (white_to_move ? (E_P >> 9) & ~file_h : (E_P << 7) & ~file_a);
    uint64_t p_can_capture_ep_l =
        P & (white_to_move ? (E_P << 7) & ~file_a : (E_P << 9) & ~file_h);
    bool none_between_king_and_ep_p = ep_pawn & K_slider;
    bool none_between_ep_p_and_attacker = ep_pawn & H_moves;

    if ((none_between_king_and_ep_p && (p_can_capture_ep_r & H_moves)) ||
        (none_between_ep_p_and_attacker && (p_can_capture_ep_r & K_slider)) ||
        (none_between_king_and_ep_p && (p_can_capture_ep_l & H_moves)) ||
        (none_between_ep_p_and_attacker && (p_can_capture_ep_l & K_slider))) {
      E_P = 0;
    }
    PINNED |= K_slider & H_moves;
    clearLowestSetBit(EHV);
  }

  // Vertical check.
  EHV = EQ | ER;
  K_slider = v_moves(K, k_bit, OCCUPIED);
  while (EHV) {
    uint64_t bb = findLowestSetBitValue(EHV);
    PINNED |= K_slider & v_moves(bb, findSetBit(bb), OCCUPIED);
    clearLowestSetBit(EHV);
  }

  // Down right diagonol check.
  uint64_t ED = EQ | EB;
  K_slider = ddr_moves(K, k_bit, OCCUPIED);
  while (ED) {
    uint64_t bb = findLowestSetBitValue(ED);
    PINNED |= K_slider & ddr_moves(bb, findSetBit(bb), OCCUPIED);
    clearLowestSetBit(ED);
  }

  // Upper right diagonol check.
  ED = EQ | EB;
  K_slider = dur_moves(K, k_bit, OCCUPIED);
  while (ED) {
    uint64_t bb = findLowestSetBitValue(ED);
    PINNED |= K_slider & dur_moves(bb, findSetBit(bb), OCCUPIED);
    clearLowestSetBit(ED);
  }
  return PINNED;
}

/** Function that adds Rook moves to the move list
 *
 * @params Piece bitboards (Rook, King)
 * @param PIECES: bitboard representing occupied spaces by the input player
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @param PINNED: bitboard of all pinned pieces for a color
 * @param checker_zone: bitboard of check areas for the current king (enemy
 * attacker piece(s) included).
 * @param wb_moves: list of all possible moves for the input player. Output
 * will be appended to this list.
 */
void get_rook_moves(uint64_t R, uint64_t K, uint64_t PIECES, uint64_t OCCUPIED,
                    uint64_t PINNED, uint64_t checker_zone, Move *wb_moves,
                    uint8_t &n_wb_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (R) {

    uint64_t bb = findLowestSetBitValue(R);
    uint8_t bit = findSetBit(bb);

    uint64_t mask = bb & PINNED ? get_mask(bb, K) : FILLED;
  
    uint64_t moves =
        h_v_moves(bb, bit, OCCUPIED, false, 0u) & ~PIECES & mask & checker_zone;

    // Loop through moves and append to list.
    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[bit];
    while (moves) {
      uint64_t final_bb = findLowestSetBitValue(moves);
      uint8_t final_bit = findSetBit(final_bb);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[final_bit];
      wb_moves[n_wb_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(moves);
    }
    clearLowestSetBit(R);
  }
}

/** Function that adds Bishop moves to the move list
 *
 * @params Piece bitboards (Bishop, King)
 * @param PIECES: bitboard representing occupied spaces by the input player
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @param PINNED: bitboard of all pinned pieces for a color
 * @param checker_zone: bitboard of check areas for the current king (enemy
 * attacker piece(s) included).
 * @param wb_moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void get_bishop_moves(uint64_t B, uint64_t K, uint64_t PIECES,
                      uint64_t OCCUPIED, uint64_t PINNED, uint64_t checker_zone,
                      Move *wb_moves, uint8_t &n_wb_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (B) {
    uint64_t bb = findLowestSetBitValue(B);
    uint8_t bit = findSetBit(bb);
    uint64_t mask = bb & PINNED ? get_mask(bb, K) : FILLED;
    uint64_t moves = diag_moves(bb, bit, OCCUPIED, false, 0u) & ~PIECES & mask &
                     checker_zone;

    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[bit];
    while (moves) {
      uint64_t bb_final = findLowestSetBitValue(moves);
      std::pair<uint8_t, uint8_t> final =
          bitToCoordinates[findSetBit(bb_final)];
      wb_moves[n_wb_moves++] = coordinatesToMove(initial, final);

      clearLowestSetBit(moves);
    }
    clearLowestSetBit(B);
  }
}

/** Function that adds Queen moves to the move list
 *
 * @params Piece bitboards (Queen, King)
 * @param PIECES: bitboard representing occupied spaces by the input player
 * @param OCCUPIED: bitboard representing all occupied spaces on the board
 * @param PINNED: bitboard of all pinned pieces for a color
 * @param checker_zone: bitboard of check areas for the current king (enemy
 * attacker piece(s) included).
 * @param wb_moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void get_queen_moves(uint64_t Q, uint64_t K, uint64_t PIECES, uint64_t OCCUPIED,
                     uint64_t PINNED, uint64_t checker_zone, Move *wb_moves,
                     uint8_t &n_wb_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (Q) {
    uint64_t bb = findLowestSetBitValue(Q);
    uint8_t bit = findSetBit(bb);
        uint64_t mask = bb & PINNED ? get_mask(bb, K) : FILLED;
    uint64_t moves = (h_v_moves(bb, bit, OCCUPIED, false, 0u) |
                      diag_moves(bb, bit, OCCUPIED, false, 0u)) &
                     ~PIECES & mask & checker_zone;

    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[bit];
    while (moves) {
      uint64_t bb_final = findLowestSetBitValue(moves);
      std::pair<uint8_t, uint8_t> final =
          bitToCoordinates[findSetBit(bb_final)];
      wb_moves[n_wb_moves++] = coordinatesToMove(initial, final);

      clearLowestSetBit(moves);
    }
    clearLowestSetBit(Q);
  }
}

/** Function that adds Knight moves to the move list
 *
 * @params Piece bitboards (Knight, King)
 * @param PIECES: bitboard representing occupied spaces by the input player
 * @param PINNED: bitboard of all pinned pieces for a color
 * @param checker_zone: bitboard of check areas for the current king (enemy
 * attacker piece(s) included).
 * @param wb_moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void get_knight_moves(uint64_t N, uint64_t K, uint64_t PIECES, uint64_t PINNED,
                      uint64_t checker_zone, Move *wb_moves,
                      uint8_t &n_wb_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  // todo: is it really efficient to redefine these everytime? maybe can
  // optimize where this is defined assuming knight is at bit 21 or F3 or (x3,
  // y5) Regarding the pos_moves.

  while (N) {
    uint64_t bb = findLowestSetBitValue(N);
    uint8_t kn_bit = findSetBit(bb);
    uint64_t pos_moves = 0;

    if (!(bb & PINNED)) { // only check for moves if it's not pinned.
                          // pinned knights cannot move.
      if (kn_bit > 21) {
        pos_moves = KNIGHT_MOVES << (kn_bit - 21);
      } else {
        pos_moves = KNIGHT_MOVES >> (21 - kn_bit);
      }
      if (kn_bit % 8 > 3) {
        pos_moves &= ~file_ab;
      } else {
        pos_moves &= ~file_gh;
      }
      pos_moves &= ~PIECES & checker_zone;

      std::pair<uint8_t, uint8_t> initial = bitToCoordinates[kn_bit];
      while (pos_moves) {
        uint64_t bb_final = findLowestSetBitValue(pos_moves);
        std::pair<uint8_t, uint8_t> final =
            bitToCoordinates[findSetBit(bb_final)];
        wb_moves[n_wb_moves++] = coordinatesToMove(initial, final);

        clearLowestSetBit(pos_moves);
      }
    }
    clearLowestSetBit(N);
  }
}

/** Function that adds King moves to the move list
 *
 * @params Piece bitboards (King)
 * @param PIECES: bitboard representing occupied spaces by the input player
 * @param DZ: bitboard representing the current 'Danger Zone' for the King,
 * which would put him in check if he moved there (illegal move)
 * @param wb_moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void get_king_moves(uint64_t K, uint64_t PIECES, uint64_t DZ, Move *wb_moves,
                    uint8_t &n_wb_moves) {

  // get moves
  uint8_t k_bit = findSetBit(K);

  uint64_t pos_moves;
  if (k_bit > 14) {
    pos_moves = KING_MOVES << (k_bit - 14);
  } else {
    pos_moves = KING_MOVES >> (14 - k_bit);
  }
  // todo: potential to make the FILEs more efficient
  if (k_bit % 8 > 3) {
    pos_moves &= ~file_a;
  } else {
    pos_moves &= ~file_h;
  }
  pos_moves &= ~PIECES & ~DZ;

  std::pair<uint8_t, uint8_t> initial = bitToCoordinates[k_bit];
  while (pos_moves) {
    uint64_t bb_final = findLowestSetBitValue(pos_moves);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb_final)];
    wb_moves[n_wb_moves++] = coordinatesToMove(initial, final);

    clearLowestSetBit(pos_moves);
  }
}

void get_X_pawn_moves(bool white_to_move, uint64_t MASK, uint64_t P, uint64_t K,
                      uint64_t E_P, uint64_t EMPTY, uint64_t OPP_PIECES,
                      uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  uint64_t P_FORWARD_1, P_FORWARD_2, P_ATTACK_L, P_ATTACK_R, P_PROMO_1,
      P_PROMO_L, P_PROMO_R;

  if (!white_to_move) {
    P_FORWARD_1 = (P >> 8) & EMPTY & ~rank_1 & MASK & checker_zone;
    P_FORWARD_2 =
        (P >> 16) & EMPTY & (EMPTY >> 8) & rank_5 & MASK & checker_zone;
    P_ATTACK_L =
        (P >> 9) & OPP_PIECES & ~rank_1 & ~file_h & MASK & checker_zone;
    P_ATTACK_R =
        (P >> 7) & OPP_PIECES & ~rank_1 & ~file_a & MASK & checker_zone;
    P_PROMO_1 = (P >> 8) & EMPTY & rank_1 & MASK & checker_zone;
    P_PROMO_L = (P >> 9) & OPP_PIECES & rank_1 & ~file_h & MASK & checker_zone;
    P_PROMO_R = (P >> 7) & OPP_PIECES & rank_1 & ~file_a & MASK & checker_zone;

    // TODO: replace all magic numbers
    // CHECK TO SEE IF WE CAN MOVE 1 SPACE FORWARD
    while (P_FORWARD_1) {
      uint64_t bb = findLowestSetBitValue(P_FORWARD_1);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 1;
      moves[n_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(P_FORWARD_1);
    }

    // check to see if you can move 2
    while (P_FORWARD_2) {
      uint64_t bb = findLowestSetBitValue(P_FORWARD_2);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 2;
      Move move = coordinatesToMove(initial, final);
      updateSpecialMove(move, PAWN_PUSH_2);
      moves[n_moves++] = move;

      clearLowestSetBit(P_FORWARD_2);
    }

    // check for attacks left
    while (P_ATTACK_L) {
      uint64_t bb = findLowestSetBitValue(P_ATTACK_L);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 1;
      initial.second += 1;
      moves[n_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(P_ATTACK_L);
    }

    // check for attacks right
    while (P_ATTACK_R) {
      uint64_t bb = findLowestSetBitValue(P_ATTACK_R);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 1;
      initial.second -= 1;
      moves[n_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(P_ATTACK_R);
    }

    // check for promotion straight
    while (P_PROMO_1) {
      uint64_t bb = findLowestSetBitValue(P_PROMO_1);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 1;
      Move move = coordinatesToMove(initial, final);

      updateSpecialMove(move, PROMOTION_QUEEN);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_ROOK);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_BISHOP);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_KNIGHT);
      moves[n_moves++] = move;

      clearLowestSetBit(P_PROMO_1);
    }

    // check for promotion left
    while (P_PROMO_L) {
      uint64_t bb = findLowestSetBitValue(P_PROMO_L);

      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 1;
      initial.second += 1;
      Move move = coordinatesToMove(initial, final);

      updateSpecialMove(move, PROMOTION_QUEEN);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_ROOK);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_BISHOP);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_KNIGHT);
      moves[n_moves++] = move;

      clearLowestSetBit(P_PROMO_L);
    }

    // check for promotion attack right
    while (P_PROMO_R) {
      uint64_t bb = findLowestSetBitValue(P_PROMO_R);

      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first += 1;
      initial.second -= 1;
      Move move = coordinatesToMove(initial, final);

      updateSpecialMove(move, PROMOTION_QUEEN);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_ROOK);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_BISHOP);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_KNIGHT);
      moves[n_moves++] = move;
      clearLowestSetBit(P_PROMO_R);
    }

    if (E_P) {
      // todo: specialize this for white
      if (checker_zone != FILLED && ((E_P << 8) & checker_zone)) {
        checker_zone |=
            (directional_mask[findSetBit(checker_zone)][FILES] & rank_3);
      }
      uint64_t P_EP_L = (P >> 9) & E_P & ~file_h & MASK & checker_zone;
      uint64_t P_EP_R = (P >> 7) & E_P & ~file_a & MASK & checker_zone;

        // check for en passant left
      while (P_EP_L) {
        uint64_t bb = findLowestSetBitValue(P_EP_L);
        std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
        std::pair<uint8_t, uint8_t> initial = final;
        initial.first += 1;
        initial.second += 1;
        Move move = coordinatesToMove(initial, final);
        updateSpecialMove(move, EN_PASSANT);
        moves[n_moves++] = move;

        clearLowestSetBit(P_EP_L);
      }

        // check for en passant right
      while (P_EP_R) {
        uint64_t bb = findLowestSetBitValue(P_EP_R);
        std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
        std::pair<uint8_t, uint8_t> initial = final;
        initial.first += 1;
        initial.second -= 1;
        Move move = coordinatesToMove(initial, final);
        updateSpecialMove(move, EN_PASSANT);
        moves[n_moves++] = move;

        clearLowestSetBit(P_EP_R);
      }
    }
  } else { // case for W

    P_FORWARD_1 = (P << 8) & EMPTY & ~rank_8 & MASK & checker_zone;
    P_FORWARD_2 =
        (P << 16) & EMPTY & (EMPTY << 8) & rank_4 & MASK & checker_zone;
    P_ATTACK_L =
        (P << 7) & OPP_PIECES & ~rank_8 & ~file_h & MASK & checker_zone;
    P_ATTACK_R =
        (P << 9) & OPP_PIECES & ~rank_8 & ~file_a & MASK & checker_zone;
    P_PROMO_1 = (P << 8) & EMPTY & rank_8 & MASK & checker_zone;
    P_PROMO_L = (P << 7) & OPP_PIECES & rank_8 & ~file_h & MASK & checker_zone;
    P_PROMO_R = (P << 9) & OPP_PIECES & rank_8 & ~file_a & MASK & checker_zone;

    // check to see if you can move 1
    while (P_FORWARD_1) {
      uint64_t bb = findLowestSetBitValue(P_FORWARD_1);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 1;
      moves[n_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(P_FORWARD_1);
    }

    // check to see if you can move 2
    while (P_FORWARD_2) {
      uint64_t bb = findLowestSetBitValue(P_FORWARD_2);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 2;
      Move move = coordinatesToMove(initial, final);
      updateSpecialMove(move, PAWN_PUSH_2);
      moves[n_moves++] = move;
      clearLowestSetBit(P_FORWARD_2);
    }

    // check for attacks left
    while (P_ATTACK_L) {
      uint64_t bb = findLowestSetBitValue(P_ATTACK_L);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 1;
      initial.second += 1;
      moves[n_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(P_ATTACK_L);
    }

    // check for attacks right
    while (P_ATTACK_R) {
      uint64_t bb = findLowestSetBitValue(P_ATTACK_R);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 1;
      initial.second -= 1;
      moves[n_moves++] = coordinatesToMove(initial, final);
      clearLowestSetBit(P_ATTACK_R);
    }

    // check for promotion straight
    while (P_PROMO_1) {
      uint64_t bb = findLowestSetBitValue(P_PROMO_1);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 1;
      Move move = coordinatesToMove(initial, final);

      updateSpecialMove(move, PROMOTION_QUEEN);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_ROOK);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_BISHOP);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_KNIGHT);
      moves[n_moves++] = move;
      clearLowestSetBit(P_PROMO_1);
    }

    // check for promotion left
    while (P_PROMO_L) {
      uint64_t bb = findLowestSetBitValue(P_PROMO_L);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 1;
      initial.second += 1;
      Move move = coordinatesToMove(initial, final);

      updateSpecialMove(move, PROMOTION_QUEEN);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_ROOK);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_BISHOP);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_KNIGHT);
      moves[n_moves++] = move;

      clearLowestSetBit(P_PROMO_L);
    }

    // check for promotion attack right
    while (P_PROMO_R) {
      uint64_t bb = findLowestSetBitValue(P_PROMO_R);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.first -= 1;
      initial.second -= 1;
      Move move = coordinatesToMove(initial, final);

      updateSpecialMove(move, PROMOTION_QUEEN);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_ROOK);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_BISHOP);
      moves[n_moves++] = move;

      updateSpecialMove(move, PROMOTION_KNIGHT);
      moves[n_moves++] = move;

      clearLowestSetBit(P_PROMO_R);
    }

    if (E_P) {
      if (checker_zone != FILLED && ((E_P >> 8) & checker_zone)) {
        checker_zone |=
            (directional_mask[findSetBit(checker_zone)][FILES] & rank_6);
      }

      uint64_t P_EP_L = (P << 7) & E_P & ~file_h & MASK & checker_zone;
      uint64_t P_EP_R = (P << 9) & E_P & ~file_a & MASK & checker_zone;

        // check for en passant left
      while (P_EP_L) {
        uint64_t bb = findLowestSetBitValue(P_EP_L);
        std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
        std::pair<uint8_t, uint8_t> initial = final;
        initial.first -= 1;
        initial.second += 1;
        Move move = coordinatesToMove(initial, final);
        updateSpecialMove(move, EN_PASSANT);
        moves[n_moves++] = move;
        clearLowestSetBit(P_EP_L);
      }

        // check for en passant right
      while (P_EP_R) {
        uint64_t bb = findLowestSetBitValue(P_EP_R);
        std::pair<uint8_t, uint8_t> final = bitToCoordinates[findSetBit(bb)];
        std::pair<uint8_t, uint8_t> initial = final;
        initial.first -= 1;
        initial.second -= 1;
        Move move = coordinatesToMove(initial, final);
        updateSpecialMove(move, EN_PASSANT);
        moves[n_moves++] = move;
        clearLowestSetBit(P_EP_R);
      }
    }
  }
}

void get_B_pawn_moves(bool white_to_move, uint64_t BP, uint64_t BK,
                      uint64_t E_P, uint64_t EMPTY, uint64_t WHITE_PIECES,
                      uint64_t PINNED, uint64_t checker_zone, Move *b_moves,
                      uint8_t &n_b_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  uint64_t pinned_pawns = BP & PINNED;
  while (pinned_pawns) {
    uint64_t bb = findLowestSetBitValue(pinned_pawns);
    uint64_t mask = get_mask(bb, BK);
    get_X_pawn_moves(white_to_move, mask, bb, BK, E_P, EMPTY, WHITE_PIECES,
                     checker_zone, b_moves, n_b_moves);
    clearLowestSetBit(pinned_pawns);
  }
  // Clear pinned pawns from pawn bitboard.
  BP &= ~PINNED;

  if (BP) { // we have at least 1 non-pinned pawn
    get_X_pawn_moves(white_to_move, FILLED, BP, BK, E_P, EMPTY, WHITE_PIECES,
                     checker_zone, b_moves, n_b_moves);
  }
}

void get_W_pawn_moves(bool white_to_move, uint64_t WP, uint64_t WK,
                      uint64_t E_P, uint64_t EMPTY, uint64_t BLACK_PIECES,
                      uint64_t PINNED, uint64_t checker_zone, Move *w_moves,
                      uint8_t &n_w_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  uint64_t pinned_pawns = WP & PINNED;
  while (pinned_pawns) {
    uint64_t bb = findLowestSetBitValue(pinned_pawns);
    uint64_t mask = get_mask(bb, WK);
    get_X_pawn_moves(white_to_move, mask, bb, WK, E_P, EMPTY, BLACK_PIECES,
                     checker_zone, w_moves, n_w_moves);
    clearLowestSetBit(pinned_pawns);
  }
  // Clear pinned pawns from the pawn bitboard.
  WP &= ~PINNED;

  if (WP) { // we have at least 1 non-pinned pawn
    get_X_pawn_moves(white_to_move, FILLED, WP, WK, E_P, EMPTY, BLACK_PIECES,
                     checker_zone, w_moves, n_w_moves);
  }
}

void get_K_castle(bool CK, uint64_t K, uint64_t EMPTY, uint64_t DZ,
                  Move *wb_moves, uint8_t &n_wb_moves) {
  if (!CK) {
    return;
  }
    // todo: implement lookup table
  if ((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)) {
    uint8_t k_bit =
        findSetBit((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1));

    std::pair<uint8_t, uint8_t> final = bitToCoordinates[k_bit];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.second -= 2;
    Move move = coordinatesToMove(initial, final);
    updateSpecialMove(move, CASTLE_KINGSIDE);
    wb_moves[n_wb_moves++] = move;
  }
}

void get_Q_castle(bool QK, uint64_t K, uint64_t EMPTY, uint64_t DZ,
                  Move *wb_moves, uint8_t &n_wb_moves) {
  if (!QK) {
    return;
  }

    if ((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)) !=
        0u) {
      uint8_t k_bit = findSetBit(((K >> 2) & EMPTY) & (EMPTY >> 1) &
                                 (EMPTY << 1) & ~DZ & ~(DZ >> 1));
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[k_bit];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.second += 2;
      Move move = coordinatesToMove(initial, final);
      updateSpecialMove(move, CASTLE_QUEENSIDE);
      wb_moves[n_wb_moves++] = move;
  }
}

uint64_t unsafe_for_XK(bool white_to_move, uint64_t P, uint64_t R, uint64_t N,
                       uint64_t B, uint64_t Q, uint64_t K, uint64_t EK,
                       uint64_t OCCUPIED) {

  uint64_t unsafe = 0;
  uint64_t D = B | Q;
  uint64_t HV = R | Q;

  // pawn
  if (P) {
    if (white_to_move) {
      unsafe = (P >> 7) & ~file_a;  // capture right
      unsafe |= (P >> 9) & ~file_h; // capture left
    } else {
      unsafe = (P << 9) & ~file_a;  // capture right
      unsafe |= (P << 7) & ~file_h; // capture left
    }
  }

  // Knight
  uint64_t pos_moves = 0;

  while (N) {
    uint8_t kn_bit = findSetBit(findLowestSetBitValue(N));

    if (kn_bit > 21) {
      pos_moves = KNIGHT_MOVES << (kn_bit - 21);
    } else {
      pos_moves = KNIGHT_MOVES >> (21 - kn_bit);
    }
    if (kn_bit % 8 > 3) {
      pos_moves &= ~file_ab;
    } else {
      pos_moves &= ~file_gh;
    }
    unsafe |= pos_moves;
    clearLowestSetBit(N);
  }

  // Diag pieces (Bishop, Queen).
  while (D) {
    uint64_t bb = findLowestSetBitValue(D);
    uint8_t bit = findSetBit(bb);
    unsafe |= diag_moves(bb, bit, OCCUPIED, true, EK);
    clearLowestSetBit(D);
  }

  // HV pieces (Rook, Queen).
  while (HV) {
    uint64_t bb = findLowestSetBitValue(HV);
    unsafe |= h_v_moves(bb, findSetBit(bb), OCCUPIED, true, EK);
    clearLowestSetBit(HV);
  }

  // King.
  uint8_t k_bit = findSetBit(K);

  if (k_bit > 14) {
    pos_moves = KING_MOVES << (k_bit - 14);
  } else {
    pos_moves = KING_MOVES >> (14 - k_bit);
  }
  if (k_bit % 8 > 3) {
    pos_moves &= ~file_a;
  } else {
    pos_moves &= ~file_h;
  }
  unsafe |= pos_moves;

  return unsafe;
}

void get_B_moves(GameState &gamestate, bool &CM, bool &SM, Move *b_moves,
                 uint8_t &n_b_moves) {
  n_b_moves = 0;

  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  uint64_t DZ = unsafe_for_XK(
      gamestate.whites_turn, gamestate.white.pawn, gamestate.white.rook,
      gamestate.white.knight, gamestate.white.bishop, gamestate.white.queen,
      gamestate.white.king, gamestate.black.king, OCCUPIED);
  // b_moves.clear();

  // DZ is the danger zone. If the king is inside of it, it is in check.
  uint8_t num_checkers = 0;
  uint64_t PINNED = get_pinned_pieces(
      gamestate.black.king, gamestate.black.pawn, gamestate.white.queen,
      gamestate.white.bishop, gamestate.white.rook, OCCUPIED,
      gamestate.en_passant,
      gamestate.whites_turn); // todo: need to put this to work. dont generate
                              // pinned moves if in check, skip that piece
  bool check = DZ & gamestate.black.king;

  uint64_t checkers = 0;
  uint64_t new_checker = 0;

  // checker zone is the area that the piece is attacking
                  // through (applies only to sliders). We have the potential to
                  // block the check by moving  apiece in the line of fire
                  // (pinning your own piece)
 uint64_t checker_zone = 0;


  if (check) { 

    uint64_t HV = gamestate.white.rook | gamestate.white.queen;

    uint64_t k_bit = findSetBit(gamestate.black.king);
    uint64_t K_moves;

    // Check horizontal pieces.
    K_moves = h_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker) {
      checkers |= new_checker;
      checker_zone |=
          h_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // Check vertical pieces.
    K_moves = v_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          v_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    uint64_t D = gamestate.white.bishop | gamestate.white.queen;

    // Check down and to the right pieces.
    K_moves = ddr_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          ddr_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // Check up and to the right pieces.
    K_moves = dur_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          dur_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // Check for knight attacks.
    if (k_bit > 21) {
      K_moves = KNIGHT_MOVES << (k_bit - 21);
    } else {
      K_moves = KNIGHT_MOVES >> (21 - k_bit);
    }
    if (k_bit % 8 > 3) {
      K_moves &= ~file_ab;
    } else {
      K_moves &= ~file_gh;
    }
    new_checker = K_moves & gamestate.white.knight;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // Check for pawn right attack (from pawns perspective).
    K_moves = (gamestate.black.king >> 9) & ~file_h;
    new_checker = K_moves & gamestate.white.pawn;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn left attack (from pawns perspective)
    K_moves = (gamestate.black.king >> 7) & ~file_a;
    new_checker = K_moves & gamestate.white.pawn;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

  } else { // only search for castles if you aren't in check
    get_K_castle(gamestate.black.can_king_side_castle, gamestate.black.king,
                 ~OCCUPIED, DZ, b_moves, n_b_moves);
    get_Q_castle(gamestate.black.can_queen_side_castle, gamestate.black.king,
                 ~OCCUPIED, DZ, b_moves, n_b_moves);
  }

  checker_zone |= checkers;

  // todo: pass check zones into the files

  if (num_checkers < 2) {

    get_B_pawn_moves(gamestate.whites_turn, gamestate.black.pawn,
                     gamestate.black.king, gamestate.en_passant, ~OCCUPIED,
                     WHITE_PIECES, PINNED, checker_zone, b_moves, n_b_moves);
    get_rook_moves(gamestate.black.rook, gamestate.black.king, BLACK_PIECES,
                   OCCUPIED, PINNED, checker_zone, b_moves, n_b_moves);
    get_bishop_moves(gamestate.black.bishop, gamestate.black.king, BLACK_PIECES,
                     OCCUPIED, PINNED, checker_zone, b_moves, n_b_moves);
    get_queen_moves(gamestate.black.queen, gamestate.black.king, BLACK_PIECES,
                    OCCUPIED, PINNED, checker_zone, b_moves, n_b_moves);
    get_knight_moves(gamestate.black.knight, gamestate.black.king, BLACK_PIECES,
                     PINNED, checker_zone, b_moves, n_b_moves);
  }
  get_king_moves(gamestate.black.king, BLACK_PIECES, DZ, b_moves, n_b_moves);

  if (!n_b_moves && check) {
    CM = true;
  } else if (!n_b_moves && !check) {
    SM = true;
  } else if ((gamestate.black.king | gamestate.white.king) == OCCUPIED) {
    SM = true;
  }
}

uint8_t getMoves(const GameState &gamestate, Move *moves) {
  uint8_t n_moves = 0;
  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  return n_moves;
}

void get_W_moves(GameState &gamestate, bool &CM, bool &SM, Move *w_moves,
                 uint8_t &n_w_moves) {

  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  // w_moves.clear();
  n_w_moves = 0;

  uint64_t DZ = unsafe_for_XK(
      gamestate.whites_turn, gamestate.black.pawn, gamestate.black.rook,
      gamestate.black.knight, gamestate.black.bishop, gamestate.black.queen,
      gamestate.black.king, gamestate.white.king, OCCUPIED);

  // DZ is the danger zone. If the king is inside of it, its in check.
  uint8_t num_checkers = 0;
  uint64_t PINNED = get_pinned_pieces(
      gamestate.white.king, gamestate.white.pawn, gamestate.black.queen,
      gamestate.black.bishop, gamestate.black.rook, OCCUPIED,
      gamestate.en_passant,
      gamestate.whites_turn); // todo: need to put this to work. dont generate
                              // pinned moves if in check, skip that piece
  bool check = DZ & gamestate.white.king;

  uint64_t checkers = 0;
  uint64_t new_checker = 0;

  // checker zone is the area that the piece is attacking
                  // through (applies only to sliders). We have the potential to
                  // block the check by moving  apiece in the line of fire
                  // (pinning your own piece)
 uint64_t checker_zone = 0;

  // ------------------
  if (check) { // currently in check
    // todo: generate checkers_bb, update_num_checkers. create method.
    uint64_t HV = gamestate.black.rook | gamestate.black.queen;
    uint8_t k_bit = findSetBit(gamestate.white.king);
    uint64_t K_moves;

    // check horizontal pieces
    K_moves = h_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker) {
      checkers |= new_checker;
      checker_zone |=
          h_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check vertical pieces
    K_moves = v_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          v_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    uint64_t D = gamestate.black.bishop | gamestate.black.queen;
    // check down and to the right pieces
    K_moves = ddr_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          ddr_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check up and to the right pieces
    K_moves = dur_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          dur_moves(new_checker, findSetBit(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // Check for knight attacks.
    if (k_bit > 21) {
      K_moves = KNIGHT_MOVES << (k_bit - 21);
    } else {
      K_moves = KNIGHT_MOVES >> (21 - k_bit);
    }

    if (k_bit % 8 > 3) {
      K_moves &= ~file_ab;
    } else {
      K_moves &= ~file_gh;
    }

    new_checker = K_moves & gamestate.black.knight;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn right attack (from pawns perspective)
    K_moves = (gamestate.white.king << 7) & ~file_h; // todo: verify
    new_checker = K_moves & gamestate.black.pawn;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn left attack (from pawns perspective)
    K_moves = (gamestate.white.king << 9) & ~file_a; // todo: verify
    new_checker = K_moves & gamestate.black.pawn;
    if (new_checker && num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

  } else { // only search for castles if you aren't in check
    get_K_castle(gamestate.white.can_king_side_castle, gamestate.white.king,
                 ~OCCUPIED, DZ, w_moves, n_w_moves);
    get_Q_castle(gamestate.white.can_queen_side_castle, gamestate.white.king,
                 ~OCCUPIED, DZ, w_moves, n_w_moves);
  }

  checker_zone |= checkers;
  // todo: pass check zones into the files

  if (num_checkers < 2) {
    get_W_pawn_moves(gamestate.whites_turn, gamestate.white.pawn,
                     gamestate.white.king, gamestate.en_passant, ~OCCUPIED,
                     BLACK_PIECES, PINNED, checker_zone, w_moves, n_w_moves);
    get_rook_moves(gamestate.white.rook, gamestate.white.king, WHITE_PIECES,
                   OCCUPIED, PINNED, checker_zone, w_moves, n_w_moves);
    get_bishop_moves(gamestate.white.bishop, gamestate.white.king, WHITE_PIECES,
                     OCCUPIED, PINNED, checker_zone, w_moves, n_w_moves);
    get_queen_moves(gamestate.white.queen, gamestate.white.king, WHITE_PIECES,
                    OCCUPIED, PINNED, checker_zone, w_moves, n_w_moves);
    get_knight_moves(gamestate.white.knight, gamestate.white.king, WHITE_PIECES,
                     PINNED, checker_zone, w_moves, n_w_moves);
  }
  get_king_moves(gamestate.white.king, WHITE_PIECES, DZ, w_moves, n_w_moves);

  if (!n_w_moves && check) {
    CM = true;
  } else if (!n_w_moves && !check) {
    SM = true;
  } else if ((gamestate.black.king | gamestate.white.king) == OCCUPIED) {
    SM = true;
  }
}

void apply_move(Move move, GameState &gamestate) {

  uint64_t P =
      gamestate.whites_turn ? gamestate.white.pawn : gamestate.black.pawn;
  uint64_t R =
      gamestate.whites_turn ? gamestate.white.rook : gamestate.black.rook;
  uint64_t N =
      gamestate.whites_turn ? gamestate.white.knight : gamestate.black.knight;
  uint64_t B =
      gamestate.whites_turn ? gamestate.white.bishop : gamestate.black.bishop;
  uint64_t Q =
      gamestate.whites_turn ? gamestate.white.queen : gamestate.black.queen;
  uint64_t K =
      gamestate.whites_turn ? gamestate.white.king : gamestate.black.king;

  uint64_t OP =
      !gamestate.whites_turn ? gamestate.white.pawn : gamestate.black.pawn;
  uint64_t OR =
      !gamestate.whites_turn ? gamestate.white.rook : gamestate.black.rook;
  uint64_t ON =
      !gamestate.whites_turn ? gamestate.white.knight : gamestate.black.knight;
  uint64_t OB =
      !gamestate.whites_turn ? gamestate.white.bishop : gamestate.black.bishop;
  uint64_t OQ =
      !gamestate.whites_turn ? gamestate.white.queen : gamestate.black.queen;
  uint64_t OK =
      !gamestate.whites_turn ? gamestate.white.king : gamestate.black.king;

  bool WCK = gamestate.white.can_king_side_castle;
  bool WCQ = gamestate.white.can_queen_side_castle;
  bool BCK = gamestate.black.can_king_side_castle;
  bool BCQ = gamestate.black.can_queen_side_castle;

  bool white_move = gamestate.whites_turn;

  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);

  uint8_t x1 = moveToX1(move);
  uint8_t y1 = moveToY1(move);
  uint8_t x2 = moveToX2(move);
  uint8_t y2 = moveToY2(move);
  SpecialMove special = moveToSpecial(move);
  
  uint64_t initial = (uint64_t)1 << ((x1 * 8) + (y1 % 8));
  uint64_t final = (uint64_t)1 << ((x2 * 8) + (y2 % 8));

  uint64_t capture;

  if (white_move) {
    capture = (BLACK_PIECES & ~OK) & final;
  } else {
    capture = WHITE_PIECES & ~OK & final;
  }

  if (capture) {
    if (OP & final) {
      OP &= ~final;
    } else if (OR & final) {

      if (white_move) {

        if (OR & 9223372036854775808u & final) {
          BCK = false;
        } else if (OR & 72057594037927936u & final) {
          BCQ = false;
        }
      } else {
        if (OR & 128u & final) {
          WCK = false;
        } else if (OR & 1u & final) {
          WCQ = false;
        }
      }

      OR &= ~final;
    } else if (ON & final) {
      ON &= ~final;
    } else if (OB & final) {
      OB &= ~final;
    } else if (OQ & final) {
      OQ &= ~final;
    }

  } else if (gamestate.en_passant == final &&
             (initial & P)) { // this means there was an en passant capture
    if (white_move) {
      OP &= ~(final >> 8);
    } else {
      OP &= ~(final << 8);
    }
  }
  // -----this concludes removing enemy pieces from board-----

  // need to move piece to the final position and also remove the initial
  // position

  // No Special move
  // TODO: make switch statement here.
  if (special == NONE) {
    // todo: search for which piece is moving
    if (Q & initial) {
      Q |= final;
      Q &= ~initial;
    } else if (B & initial) {
      B |= final;
      B &= ~initial;
    } else if (R & initial) {

      if (WCK && white_move && (R & 128u & initial)) {
        WCK = false;
      } else if (WCQ && white_move && (R & 1u & initial)) {
        WCQ = false;
      } else if (BCK && !white_move && (R & 9223372036854775808u & initial)) {
        BCK = false;
      } else if (BCQ && !white_move && (R & 72057594037927936u & initial)) {
        BCQ = false;
      }
      R |= final;
      R &= ~initial;
    } else if (N & initial) {
      N |= final;
      N &= ~initial;
    } else if (P & initial) {
      P |= final;
      P &= ~initial;
    } else if (K & initial) {
      if ((WCK || WCQ) && white_move) {
        WCK = false;
        WCQ = false;
      }
      if ((BCK || BCQ) && !white_move) {
        BCK = false;
        BCQ = false;
      }
      K = final;
    }

    gamestate.en_passant = 0u;
  } else {
    if (special == CASTLE_KINGSIDE) {
      if (white_move) {
        // TODO: resolve all these magic numbers.
        K = 64u;
        R -= 128u;
        R += 32u;
        WCK = false;
        WCQ = false;
      } else {
        K = 4611686018427387904u;
        R -= 9223372036854775808u;
        R += 2305843009213693952u;
        BCK = false;
        BCQ = false;
      }
      gamestate.en_passant = 0u;
    } else if (special == CASTLE_QUEENSIDE) {
      if (white_move) {
        K = 4u;
        R -= 1u;
        R += 8u;
        WCQ = false;
        WCK = false;
      } else {
        K = 288230376151711744u;
        R -= 72057594037927936u;
        R += 576460752303423488u;
        BCQ = false;
        BCK = false;
      }
      gamestate.en_passant = 0u;
    } else if (special == PROMOTION_QUEEN) { // promotion
      P &= ~initial;
      Q |= final;
      gamestate.en_passant = 0u;
    } else if (special == PROMOTION_BISHOP) {
      P &= ~initial;
      B |= final;
      gamestate.en_passant = 0u;
    } else if (special == PROMOTION_KNIGHT) {
      P &= ~initial;
      N |= final;
      gamestate.en_passant = 0u;
    } else if (special == PROMOTION_ROOK) {
      P &= ~initial;
      R |= final;
      gamestate.en_passant = 0u;
    } else if (special == EN_PASSANT) { // en passant capture

      P |= final;
      P &= ~initial;
      gamestate.en_passant = 0u;
    } else { // pawn push 2

      P |= final;
      P &= ~initial;
      if (white_move) {
        gamestate.en_passant = (final >> 8);
      } else {
        gamestate.en_passant = (final << 8);
      }
    }
  }

  gamestate.white.pawn = white_move ? P : OP;
  gamestate.white.rook = white_move ? R : OR;
  gamestate.white.knight = white_move ? N : ON;
  gamestate.white.bishop = white_move ? B : OB;
  gamestate.white.queen = white_move ? Q : OQ;
  gamestate.white.king = white_move ? K : OK;
  gamestate.white.can_king_side_castle = WCK;
  gamestate.white.can_queen_side_castle = WCQ;

  gamestate.black.pawn = !white_move ? P : OP;
  gamestate.black.rook = !white_move ? R : OR;
  gamestate.black.knight = !white_move ? N : ON;
  gamestate.black.bishop = !white_move ? B : OB;
  gamestate.black.queen = !white_move ? Q : OQ;
  gamestate.black.king = !white_move ? K : OK;
  gamestate.black.can_king_side_castle = BCK;
  gamestate.black.can_queen_side_castle = BCQ;

  white_move = !white_move; // alternate the turn
  gamestate.whites_turn = white_move;
}

void print_moves(bool white_to_move, Move *moves, uint8_t n_moves) {
  std::cout << (white_to_move ? "WHITE" : "BLACK") << "'S MOVE: " << std::endl;
  for (uint8_t i = 0; i < n_moves; i++) {
    std::cout << i + 1 << ": " + moveToString(moves[i]) << std::endl;
  }
}

void perft(uint32_t &nodes, GameState &gamestate, Move *moves, uint8_t &n_moves,
           uint8_t depth, uint8_t orig_depth, bool total) {

  bool a = false;
  if (gamestate.whites_turn) {
    get_W_moves(gamestate, a, a, moves, n_moves);
  } else {
    get_B_moves(gamestate, a, a, moves, n_moves);
  }

  if (depth == 1) {
    nodes += n_moves;
  }

  if (depth > 1) {

    for (uint8_t i = 0; i < n_moves; i++) {
      uint8_t cap_count_temp = 0;

      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
      apply_move(moves[i], gamestate_temp);

      //  else if (CMt or)
      Move moves_temp[MAX_POSSIBLE_MOVES_PER_POSITION];
      uint8_t n_moves_temp = 0;

      perft(nodes, gamestate_temp, moves_temp, n_moves_temp, uint8_t(depth - 1),
            orig_depth, total);

      if (depth == orig_depth && false) {
        if (total) {
          std::cout << round(((i * 100 / n_moves)))
                    << "% complete... -> d1:" << moveToString(moves[i])
                    << "--------------------------------------------------"
                    << std::endl;

        } else { // node based
          std::cout << i << ":" << moveToString(moves[i]) << " " << nodes
                    << std::endl;
          nodes = 0;
        }
      }
    }
  }
}

uint32_t nodes2 = 0;

double eval(const GameState gamestate) {

  // material
  double counter = 0;
  counter += (countSetBits(gamestate.white.pawn) -
              countSetBits(gamestate.black.pawn)) *
             100;
  counter += (countSetBits(gamestate.white.bishop) -
              countSetBits(gamestate.black.bishop)) *
             300; // todo: add special case regarding number of bishops
  counter += (countSetBits(gamestate.white.knight) -
              countSetBits(gamestate.black.knight)) *
             300;
  counter += (countSetBits(gamestate.white.rook) -
              countSetBits(gamestate.black.rook)) *
             500;
  counter += (countSetBits(gamestate.white.queen) -
              countSetBits(gamestate.black.queen)) *
             900;

  //    if (counter > 1000000){
  //       std::cout << counter << std::endl;
  //       print_board(gamestate);
  //       exit(1);
  //    }

  return counter;
}

AI_return minimax(GameState gamestate, bool CM, bool SM, uint8_t depth,
                  bool my_turn, double alpha = -100000000,
                  double beta = 100000000) {

  // std::cout<<"alpha: "<<alpha<<". beta: "<<beta<<"."<<std::endl;

  if (depth == 0) { // todo: add a conditon for game over
    // todo add evaluation function
    nodes2++;
    // simply a placeholder to avoid error, TODO make more elegant.
    Move leaf_move;
    leaf_move.data = 0;
    AI_return leaf = {leaf_move, eval(gamestate)};
    //    std::cout<<leaf.value<<std::endl;
    return leaf;
  }

  if (my_turn) {
    Move w_moves[MAX_POSSIBLE_MOVES_PER_POSITION];
    uint8_t n_w_moves = 0;

    Move max_move;
    double max_val = -10000000;
    AI_return a;

    get_W_moves(gamestate, CM, SM, w_moves, n_w_moves);
    if (CM) { // std::cout << "CHECKMATE. BLACK WINS" << std::endl;
      Move null_move;
      null_move.data = 0;
      AI_return leaf = {null_move, -10000};
      return leaf;
    }
    if (SM) { // std::cout << "STALEMATE." << std::endl;
      Move null_move;
      null_move.data = 0;
      AI_return leaf = {null_move, 0};
      return leaf;
    }

    for (uint8_t i = 0; i < n_w_moves; i++) {

      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));

      //   bool CMt = false, SMt = false;

      apply_move(w_moves[i], gamestate_temp);

      a = minimax(gamestate_temp, CM, SM, depth - 1, !my_turn, alpha, beta);

      if (a.value > max_val) {
        max_val = a.value;
        max_move = w_moves[i];
      }

      alpha = std::max(alpha, a.value);
      if (beta <= alpha) {
        break;
      }
    }

    AI_return leaf_node;
    leaf_node.value = max_val;
    leaf_node.move = max_move;
    return leaf_node;

  } else {
    Move b_moves[MAX_POSSIBLE_MOVES_PER_POSITION];
    uint8_t n_b_moves = 0;

    Move min_move;
    double min_val = 10000000;
    AI_return a;

    get_B_moves(gamestate, CM, SM, b_moves, n_b_moves);

    if (CM) { // std::cout << "CHECKMATE. WHITE WINS" << std::endl;
      Move null_move;
      null_move.data = 0;
      AI_return leaf = {null_move, 10000};
      return leaf;
    }
    if (SM) { // std::cout << "STALEMATE." << std::endl;
      Move null_move;
      null_move.data = 0;
      AI_return leaf = {null_move, 0};
      return leaf;
    }

    for (uint8_t j = 0; j < n_b_moves; j++) {

      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
      bool CMt = false, SMt = false;

      apply_move(b_moves[j], gamestate_temp);

      a = minimax(gamestate_temp, CMt, SMt, depth - 1, !my_turn, alpha, beta);

      if (a.value < min_val) {
        min_val = a.value;
        min_move = b_moves[j];
      }

      beta = std::min(beta, a.value);
      if (beta <= alpha) {
        break;
      }
    }

    AI_return leaf_node;
    leaf_node.value = min_val;
    leaf_node.move = min_move;

    return leaf_node;
  }

  // todo: implement the eval function
}

void fenToGameState(const std::string fen, GameState &gamestate) {

  std::string regex_string =
      "^ *([rnbqkpRNBQKP1-8]+/){7}([rnbqkpRNBQKP1-8]+) ([wb]) "
      "(([kqKQ]{1,4})|(-)) (([a-h][36])|(-)) *(.*)$";

  if (!regex_match(fen, std::regex(regex_string))) {
    logErrorAndExit("ERROR: Not a valid FEN.");
    return;
  }

  // Counters to validate FEN is a legal and possible position.
  // TODO: validate the castling flags and rook/king location.
  // TODO: make sure kings are separated by at least 1 square.
  // TODO: the sum of the pieces and empty squares sum to 8 per rank.
  // TODO: there should be no consecutive numbers.
  // TODO: non active color is not in check.
  // For more info:
  // https://chess.stackexchange.com/questions/1482/how-do-you-know-when-a-fen-position-is-legal

  uint8_t n_p = 0;
  uint8_t n_r = 0;
  uint8_t n_n = 0;
  uint8_t n_b = 0;
  uint8_t n_q = 0;
  uint8_t n_k = 0;
  uint8_t n_P = 0;
  uint8_t n_R = 0;
  uint8_t n_N = 0;
  uint8_t n_B = 0;
  uint8_t n_Q = 0;
  uint8_t n_K = 0;

  uint8_t byte = 7;
  uint8_t bit = 0;

  // Populate piece positions.
  for (uint8_t i = 0; i < fen.find(' '); i++) {
    // Check for empty positions.
    if (isdigit(fen[i])) {
      bit += fen[i] - '0';
      continue;
    }
    switch (fen[i]) {
    case 'p':
      if (byte == 0 || byte == 7) {
        logErrorAndExit("ERROR: Pawns cannot be on rank 1 or rank 8.");
        memset(&gamestate, 0, sizeof(GameState));
        return;
      }
      gamestate.black.pawn += 1ull << (byte * 8 + bit);
      n_p++;
      break;
    case 'r':
      gamestate.black.rook += (1ull << (byte * 8 + bit));
      n_r++;
      break;
    case 'n':
      gamestate.black.knight += 1ull << (byte * 8 + bit);
      n_n++;
      break;
    case 'b':
      gamestate.black.bishop += 1ull << (byte * 8 + bit);
      n_b++;
      break;
    case 'q':
      gamestate.black.queen += 1ull << (byte * 8 + bit);
      n_q++;
      break;
    case 'k':
      gamestate.black.king += 1ull << (byte * 8 + bit);
      n_k++;
      break;
    case 'P':
      if (byte == 0 || byte == 7) {
        logErrorAndExit("ERROR: Pawns cannot be on rank 1 or rank 8.");
        memset(&gamestate, 0, sizeof(GameState));
        return;
      }
      gamestate.white.pawn += 1ull << (byte * 8 + bit);
      n_P++;
      break;
    case 'R':
      gamestate.white.rook += 1ull << (byte * 8 + bit);
      n_R++;
      break;
    case 'N':
      gamestate.white.knight += 1ull << (byte * 8 + bit);
      n_N++;
      break;
    case 'B':
      gamestate.white.bishop += 1ull << (byte * 8 + bit);
      n_B++;
      break;
    case 'Q':
      gamestate.white.queen += 1ull << (byte * 8 + bit);
      n_Q++;
      break;
    case 'K':
      gamestate.white.king += 1ull << (byte * 8 + bit);
      n_K++;
      break;
    case '/':
      byte -= 1;
      bit = 0;
      continue;
    default:
      logErrorAndExit("ERROR: Unknown character in FEN string!");
      break;
    }
    bit++;
  }

  // Piece count sanity checks.
  int8_t n_p_missing = 8 - n_p;
  int8_t n_P_missing = 8 - n_P;
  uint8_t n_promoted = (n_r > 2 ? n_r - 2 : 0) + (n_n > 2 ? n_n - 2 : 0) +
                       (n_b > 2 ? n_b - 2 : 0) + (n_q > 1 ? n_q - 1 : 0);
  uint8_t n_Promoted = (n_R > 2 ? n_R - 2 : 0) + (n_N > 2 ? n_N - 2 : 0) +
                       (n_B > 2 ? n_B - 2 : 0) + (n_Q > 1 ? n_Q - 1 : 0);
  if (n_P > 8 || n_p > 8 || n_K != 1 || n_k != 1 || n_p_missing < n_promoted ||
      n_P_missing < n_Promoted) {
    logErrorAndExit("ERROR: FEN position not legal/possible.");
    memset(&gamestate, 0, sizeof(GameState));
    return;
  }

  // Populate extra game state data.
  // TODO: Add half move and full move functionality. The 4th and 5th fields.
  uint8_t field = 1; // 1: turn, 2: castling flags, 3: en passant.
  for (uint8_t i = fen.find(' ') + 1; i < fen.length(); i++) {
    // Nit: could be a switch statement.
    if (field == 1) {
      if (fen[i] == 'w') {
        gamestate.whites_turn = true;
        field++;
        continue;
      }
      if (fen[i] == 'b') {
        gamestate.whites_turn = false;
        field++;
        continue;
      }
    } else if (field == 2) {
      if (((gamestate.white.can_king_side_castle ||
            gamestate.white.can_queen_side_castle ||
            gamestate.black.can_king_side_castle ||
            gamestate.black.can_queen_side_castle) &&
           fen[i] == ' ') ||
          fen[i] == '-') {
        field++;
        continue;
      }
      if (fen[i] == 'K') {
        gamestate.white.can_king_side_castle = true;
        continue;
      }
      if (fen[i] == 'Q') {
        gamestate.white.can_queen_side_castle = true;
        continue;
      }
      if (fen[i] == 'k') {
        gamestate.black.can_king_side_castle = true;
        continue;
      }
      if (fen[i] == 'q') {
        gamestate.black.can_queen_side_castle = true;
        continue;
      }
    } else {
      if (fen[i] >= 'a' && fen[i] <= 'h') {
        uint8_t col = fen[i] - 'a';
        uint8_t row = fen[i + 1] - '0' - 1;
        gamestate.en_passant = 1ull << (row * 8 + col);
        // std::cout<< (row * 8 + col) << std::endl;
      }
    }
  }
}

void generate_board(std::string name, uint8_t diff) {
  std::cout << "GAME START" << std::endl;

  std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  // FEN = "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1";
  // FEN= "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2";
  bool CM = false, SM = false;

  GameState gamestate;
  fenToGameState(FEN, gamestate);

  AI_return AI_choice;

  uint8_t depth;
  if (diff == 1) {
    depth = 4;
  } else if (diff == 2) {
    depth = 5;
  } else if (diff == 3) {
    depth = 6;
  }

  // for now, the AI is only white
  // todo: implement AI for both colors
  srand(time(nullptr));

  Player p = Player(true);
  Player p2 = Player(true, true);

  while (!CM && !SM) {

    uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
    uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
    uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    if (gamestate.whites_turn) {

      std::cout << "WHITE'S MOVE: " << std::endl;
      std::cout << "AI Agent thinking... wait a few seconds." << std::endl;
      auto start = std::chrono::high_resolution_clock::now();

      AI_choice = minimax(gamestate, CM, SM, depth, true);
      auto end = std::chrono::high_resolution_clock::now();

      std::cout << "Move chosen: " << moveToString(AI_choice.move) << std::endl;
      std::cout << AI_choice.value << std::endl;

      apply_move(AI_choice.move, gamestate);

      std::cout << "depth: " << depth + 0<< ". time elapsed: "
                << (double)(end - start).count() / 1000000000
                << " s. nodes searched: " << nodes2 << "." << std::endl;
      std::cout << "NPS: "
                << nodes2 / ((double)(end - start).count() / 1000000000)
                << std::endl;
      std::cout << " " << std::endl;
    } else {
      print_board(gamestate);
      std::cout << "BLACK'S MOVE: " << std::endl;

      // todo: create a player class for their choosing mechanism
      Move b_moves[MAX_POSSIBLE_MOVES_PER_POSITION];
      uint8_t n_b_moves = 0;

      // TODO: uncomment this and fix
      get_B_moves(gamestate, CM, SM, b_moves, n_b_moves);

      std::cout << "Please select your move: " << std::endl;
      print_moves(gamestate.whites_turn, b_moves, n_b_moves);

      int user_choice;
      std::cin >> user_choice;

      apply_move(b_moves[user_choice - 1], gamestate);

      std::cout << "Move chosen: " << moveToString(b_moves[user_choice - 1])
                << std::endl;
      std::cout << " " << std::endl;
    }
  }
}
