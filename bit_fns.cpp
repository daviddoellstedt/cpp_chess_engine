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
#include <stdint.h>
#include <string>
#include <vector>

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
    // TODO add error handling here.
    return "";
    break;
  }
}

std::string moveToString(Move move) {
  std::string result = "";
  result += std::to_string(moveToX1(move)) + std::to_string(moveToY1(move)) +
            ">" + std::to_string(moveToX2(move)) +
            std::to_string(moveToY2(move)) + " " +
            specialMoveToString(moveToSpecial(move));
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

/** Printing the board to the command line.
 *
 * arguments: the 12 bitboards for the all the pieces
 */
void print_board(const GameState gamestate) {
  char grid[8][8] = {// 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 7
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 6
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 5
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 4
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 3
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 2
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 1
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}}; // 0

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
      }
    }
  }

  std::string dividing_line =
      "|---|-----|-----|-----|-----|-----|-----|-----|-----|";

  std::string line;
  for (int i = 0; i <= 7; i++) {
    std::cout << dividing_line << std::endl;
    line = "| " + std::to_string(8 - i) + " |  ";
    for (int j = 0; j < 8; j++) {
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
uint64_t h_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED) {
  uint64_t horiz_moves =
      (((OCCUPIED)-2 * piece) ^ rev((rev(OCCUPIED) - 2 * rev(piece)))) &
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
uint64_t v_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED) {
  uint64_t vert_moves =
      (((OCCUPIED & directional_mask[sl_bit][FILES]) - 2 * piece) ^
       rev((rev(OCCUPIED & directional_mask[sl_bit][FILES]) -
            2 * rev(piece)))) &
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
uint64_t h_v_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED,
                   bool unsafe_calc, uint64_t K) {

  // this line is used in the case where we need to generate zones for the king
  // that are unsafe. If the king is in the attack zone of a horizontal/vertical
  // slider, we want to remove the king from the calculation. Because a move of
  // the king, that still lies in the path of attack (further away from the
  // slider) is still an "unsafe" move.
  if (unsafe_calc) {
    OCCUPIED &= ~K;
  }

  // todo: room for optimization? Less new variables?
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
uint64_t ddr_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED) {
  uint64_t ddr_moves =
      (((OCCUPIED & directional_mask[sl_bit][DIAGONOLS_DOWN_RIGHT]) -
        2 * piece) ^
       rev((rev(OCCUPIED & directional_mask[sl_bit][DIAGONOLS_DOWN_RIGHT]) -
            2 * rev(piece)))) &
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
uint64_t dur_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED) {
  uint64_t dur_moves =
      (((OCCUPIED & directional_mask[sl_bit][DIAGONOLS_UP_RIGHT]) - 2 * piece) ^
       rev((rev(OCCUPIED & directional_mask[sl_bit][DIAGONOLS_UP_RIGHT]) -
            2 * rev(piece)))) &
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
uint64_t diag_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED,
                    bool unsafe_calc, uint64_t K) {

  // this line is used in the case where we need to generate zones for the king
  // that are unsafe. If the king is in the attack zone of a diagonal slider, we
  // want to remove the king from the calculation. Because a move of the king,
  // that still lies in the path of attack (further away from the slider) is
  // still an "unsafe" move.
  if (unsafe_calc) {
    OCCUPIED &= ~K;
  }
  // todo: room for optimization? Less new variables?
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
  int k_bit = (int)log2(p2), p_bit = (int)log2(p1);

  int k_x = ((k_bit - k_bit % 8) / 8), p_x = ((p_bit - p_bit % 8) / 8);
  int k_y = (k_bit % 8), p_y = (p_bit % 8);

  if (k_x - p_x == 0) { // return horizontal mask
    return directional_mask[k_bit][RANKS];
  } else if (k_y - p_y == 0) { // return vertical mask
    return directional_mask[k_bit][FILES];
  } else if (((p_x - k_x) > 0 and (p_y - k_y) < 0) or
             ((p_x - k_x) < 0 and (p_y - k_y) > 0)) { // return ddr_mask
    return directional_mask[k_bit][DIAGONOLS_DOWN_RIGHT];
  } else if (((p_x - k_x) < 0 and (p_y - k_y) < 0) or
             ((p_x - k_x) > 0 and (p_y - k_y) > 0)) { // return dur_mask
    return directional_mask[k_bit][DIAGONOLS_UP_RIGHT];
  } else {
    std::cout << "ERROR in get_pinned_mask" << std::endl;
    return 0u;
  }
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
                           uint64_t ER, uint64_t OCCUPIED, uint64_t E_P,
                           uint64_t &E_P_special) {

  uint64_t PINNED = 0u, NEW_PIN, K_slider, H_moves;
  std::vector<uint64_t> h_v_slider_bbs,
      diag_slider_bbs; // individual bitboards for each piece

  // for the 4 directions (4 iterations)
  //  1. generate sliding moves from the kings position (include "capture" of
  //  the kings pieces
  //  2. generate sliding moves from opponents pieces
  //  3. take the AND of 1 and 2. This is a location of a pinned piece, if any.
  //  OR this result with pinned piece bb
  int k_bit = (int)log2(K);

  for (int i = 0; i < 4; i++) {
    if (i == 0) { // horizontal check
      K_slider = h_moves(K, k_bit, OCCUPIED);

      uint64_t EHV = EQ | ER;
      for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
        if (EHV & 1) {
          uint64_t h = (uint64_t)1 << bit;
          H_moves = h_moves(h, bit, OCCUPIED);
          NEW_PIN = K_slider & H_moves;

          // todo add if logic for player turn
          if ((((E_P << 8) & K_slider) != 0 and
               ((E_P << 7) & ~file_h & P & H_moves)) or
              (((E_P << 8) & H_moves) != 0 and
               ((E_P << 7) & ~file_h & P & K_slider))) {
            NEW_PIN |= (E_P << 7);
            E_P_special =
                (directional_mask[(int)log2((E_P << 7))][FILES] |
                 directional_mask[(int)log2((E_P << 7))][DIAGONOLS_DOWN_RIGHT] |
                 directional_mask[(int)log2((E_P << 7))][DIAGONOLS_UP_RIGHT]) &
                ~get_mask((E_P << 7), E_P);

            //  viz_bb(E_P_special);
          }

          if ((((E_P << 8) & K_slider) != 0 and
               ((E_P << 9) & ~file_a & P & H_moves)) or
              (((E_P << 8) & H_moves) != 0 and
               ((E_P << 9) & ~file_a & P & K_slider))) {
            NEW_PIN |= (E_P << 9);
            E_P_special =
                (directional_mask[(int)log2((E_P << 9))][FILES] |
                 directional_mask[(int)log2((E_P << 9))][DIAGONOLS_DOWN_RIGHT] |
                 directional_mask[(int)log2((E_P << 9))][DIAGONOLS_UP_RIGHT]) &
                ~get_mask((E_P << 9), E_P);
          }

          // for white
          if ((((E_P >> 8) & K_slider) != 0 and
               ((E_P >> 9) & ~file_h & P & H_moves)) or
              (((E_P >> 8) & H_moves) != 0 and
               ((E_P >> 9) & ~file_h & P & K_slider))) {
            NEW_PIN |= (E_P >> 9);
            E_P_special =
                (directional_mask[(int)log2((E_P >> 9))][FILES] |
                 directional_mask[(int)log2((E_P >> 9))][DIAGONOLS_DOWN_RIGHT] |
                 directional_mask[(int)log2((E_P >> 9))][DIAGONOLS_UP_RIGHT]) &
                ~get_mask((E_P >> 9), E_P);
          }

          if ((((E_P >> 8) & K_slider) != 0 and
               ((E_P >> 7) & ~file_a & P & H_moves)) or
              (((E_P >> 8) & H_moves) != 0 and
               ((E_P >> 7) & ~file_a & P & K_slider))) {
            NEW_PIN |= (E_P >> 7);
            E_P_special =
                (directional_mask[(int)log2((E_P >> 7))][FILES] |
                 directional_mask[(int)log2((E_P >> 7))][DIAGONOLS_DOWN_RIGHT] |
                 directional_mask[(int)log2((E_P >> 7))][DIAGONOLS_UP_RIGHT]) &
                ~get_mask((E_P >> 7), E_P);
          }

          PINNED |= NEW_PIN;
        }
        EHV >>= 1;
        if (!EHV) {
          break;
        }
      }

    } else if (i == 1) { // vertical check
      uint64_t EHV = EQ | ER;
      K_slider = v_moves(K, k_bit, OCCUPIED);

      for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
        if (EHV & 1) {
          uint64_t v = (uint64_t)1 << bit;
          NEW_PIN = K_slider & v_moves(v, bit, OCCUPIED);
          PINNED |= NEW_PIN;
        }
        EHV >>= 1;
        if (!EHV) {
          break;
        }
      }
    } else if (i == 2) { // ddr check
      uint64_t ED = EQ | EB;
      K_slider = ddr_moves(K, k_bit, OCCUPIED);
      for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
        if (ED & 1) {
          uint64_t ddr = (uint64_t)1 << bit;
          NEW_PIN = K_slider & ddr_moves(ddr, bit, OCCUPIED);
          PINNED |= NEW_PIN;
        }
        ED >>= 1;
        if (!ED) {
          break;
        }
      }
    } else { // dur check
      uint64_t ED = EQ | EB;
      K_slider = dur_moves(K, k_bit, OCCUPIED);
      for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
        if (ED & 1) {
          uint64_t dur = (uint64_t)1 << bit;
          NEW_PIN = K_slider & dur_moves(dur, bit, OCCUPIED);
          PINNED |= NEW_PIN;
        }
        ED >>= 1;
        if (!ED) {
          break;
        }
      }
    }
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
 * will be appended to this vector.
 */
void get_rook_moves(uint64_t R, uint64_t K, uint64_t PIECES, uint64_t OCCUPIED,
                    uint64_t PINNED, uint64_t checker_zone,
                    std::vector<Move> &wb_moves) {
  if (checker_zone == 0) {
    checker_zone = FILLED;
  }

  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    if (!R) {
      return;
    }

    if (R & 1) {
      uint64_t bb = (uint64_t)1 << bit;
      // get moves
      uint8_t sl_bit = bit;

      uint64_t mask = FILLED;
      if ((bb & PINNED) > 0u) {
        mask = get_mask(bb, K);
      }
      // todo: make this a lookup table for improved performance
      std::bitset<64> moves(h_v_moves(bb, sl_bit, OCCUPIED, false, 0u) &
                            ~PIECES & mask & checker_zone);
      // loop through moves and append to list, if there are any
      if (moves != 0) {
        std::pair<uint8_t, uint8_t> initial = bitToCoordinates[sl_bit];

        for (int i = 0; i < 64; i++) {
          if (moves[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            wb_moves.emplace_back(coordinatesToMove(initial, final));
          }
        }
      }
    }
    R >>= 1;
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
                      std::vector<Move> &wb_moves) {

  if (checker_zone == 0) {
    checker_zone = FILLED;
  }

  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    if (!B) {
      return;
    }

    if (B & 1) {
      uint64_t bb = (uint64_t)1 << bit;
      // get moves
      uint8_t sl_bit = bit;

      uint64_t mask = FILLED;
      if ((bb & PINNED) > 0u) {
        mask = get_mask(bb, K);
      }
      // todo: make this a lookup table for improved performance
      std::bitset<64> moves(diag_moves(bb, sl_bit, OCCUPIED, false, 0u) &
                            ~PIECES & mask & checker_zone);
      // loop through moves and append to list, if there are any
      if (moves != 0) {
        std::pair<uint8_t, uint8_t> initial = bitToCoordinates[sl_bit];
        // todo: can maybe optimize by not searching the entire range
        for (int i = 0; i < 64; i++) {
          if (moves[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            wb_moves.emplace_back(coordinatesToMove(initial, final));
          }
        }
      }
    }
    B >>= 1;
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
                     uint64_t PINNED, uint64_t checker_zone,
                     std::vector<Move> &wb_moves) {
  if (checker_zone == 0) {
    checker_zone = FILLED;
  }

  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    if (!Q) {
      return;
    }

    if (Q & 1) {
      uint8_t sl_bit = bit;
      uint64_t bb = (uint64_t)1 << bit;

      uint64_t mask = FILLED;
      if ((bb & PINNED) > 0u) {
        mask = get_mask(bb, K);
      }
      // todo: make this a lookup table for improved performance
      std::bitset<64> moves((h_v_moves(bb, sl_bit, OCCUPIED, false, 0u) |
                             diag_moves(bb, sl_bit, OCCUPIED, false, 0u)) &
                            ~PIECES & mask & checker_zone);
      // loop through moves and append to list, if there are any
      if (moves != 0) {
        std::pair<uint8_t, uint8_t> initial = bitToCoordinates[sl_bit];
        for (int i = 0; i < 64; i++) {
          if (moves[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            wb_moves.emplace_back(coordinatesToMove(initial, final));
          }
        }
      }
    }
    Q >>= 1;
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
                      uint64_t checker_zone, std::vector<Move> &wb_moves) {
  if (checker_zone == 0) {
    checker_zone = FILLED;
  }

  // todo: is it really efficient to redefine these everytime? maybe can
  // optimize where this is defined assuming knight is at bit 21 or F3 or (x3,
  // y5) Regarding the pos_moves.

  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    // get moves
    // todo: make this a lookup table for improved performance
    if (!N) {
      return;
    }

    if (N & 1) {
      uint8_t kn_bit = bit;
      uint64_t bb = (uint64_t)1 << bit;
      uint64_t pos_moves = 0;

      if ((bb & PINNED) == 0u) { // only check for moves if it's not pinned.
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

        // todo: find a more efficient way to loop through the board (by being
        // smart about it)
        std::bitset<64> moves(pos_moves);
        // loop through moves and append to list, if there are any
        if (moves != 0) {
          std::pair<uint8_t, uint8_t> initial = bitToCoordinates[kn_bit];
          for (int i = 0; i < 64; i++) {
            if (moves[i] == 1) {
              std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
              wb_moves.emplace_back(coordinatesToMove(initial, final));
            }
          }
        }
      }
    }
    N >>= 1;
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
void get_king_moves(uint64_t K, uint64_t PIECES, uint64_t DZ,
                    std::vector<Move> &wb_moves) {
  // todo: is it really efficient to redefine these everytime? maybe can
  // optimize where this is defined assuming knight is at bit 21 or F3 or (x3,
  // y5)
  uint64_t pos_moves;

  // get moves
  // todo: make this a lookup table for improved performance
  uint8_t k_bit = 0;

  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    if (K & 1) {
      k_bit = bit;
      break;
    }
    K >>= 1;
  }

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

  // todo: find a more efficient way to loop through the board (by being smart
  // about it)
  std::bitset<64> moves(pos_moves);
  // loop through moves and append to list, if there are any
  if (moves != 0) {
    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[k_bit];
    for (int i = 0; i < 64; i++) {
      if (moves[i] == 1) {
        std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
        wb_moves.emplace_back(coordinatesToMove(initial, final));
      }
    }
  }
}

void get_X_pawn_moves(std::string X, uint64_t MASK, uint64_t P, uint64_t K,
                      uint64_t E_P, uint64_t EMPTY, uint64_t OPP_PIECES,
                      uint64_t checker_zone, std::vector<Move> &moves) {
  uint64_t P_FORWARD_1, P_FORWARD_2, P_ATTACK_L, P_ATTACK_R, P_PROMO_1,
      P_PROMO_L, P_PROMO_R;
  std::bitset<64> bits;

  if (X == "B") {
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

    //  viz_bb(P_PROMO_L);

    // CHECK TO SEE IF WE CAN MOVE 1 SPACE FORWARD
    if (P_FORWARD_1 > 0u) {
      bits = P_FORWARD_1; // check to see if you can move 1
      for (int i = 8; i < 48; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 1;
          moves.emplace_back(coordinatesToMove(initial, final));
        }
      }
    }

    if (P_FORWARD_2 > 0u) {
      bits = P_FORWARD_2; // check to see if you can move 2
      for (int i = 32; i < 40; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 2;
          Move move = coordinatesToMove(initial, final);
          updateSpecialMove(move, PAWN_PUSH_2);
          moves.emplace_back(move);
        }
      }
    }

    if (P_ATTACK_L > 0u) {
      bits = P_ATTACK_L; // check for attacks left
      for (int i = 8; i < 48; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 1;
          initial.second += 1;
          moves.emplace_back(coordinatesToMove(initial, final));
        }
      }
    }

    if (P_ATTACK_R > 0u) {
      bits = P_ATTACK_R; // check for attacks right
      for (int i = 8; i < 48; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 1;
          initial.second -= 1;
          moves.emplace_back(coordinatesToMove(initial, final));
        }
      }
    }

    // check for promotion straight
    if (P_PROMO_1 > 0u) {
      bits = P_PROMO_1;
      for (int i = 0; i < 8; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 1;
          Move move = coordinatesToMove(initial, final);

          updateSpecialMove(move, PROMOTION_QUEEN);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_ROOK);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_BISHOP);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_KNIGHT);
          moves.emplace_back(move);
        }
      }
    }

    if (P_PROMO_L > 0u) {
      bits = P_PROMO_L; // check for promotion left
      for (int i = 0; i < 8; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 1;
          initial.second += 1;
          Move move = coordinatesToMove(initial, final);

          updateSpecialMove(move, PROMOTION_QUEEN);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_ROOK);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_BISHOP);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_KNIGHT);
          moves.emplace_back(move);
        }
      }
    }

    if (P_PROMO_R > 0u) {
      bits = P_PROMO_R; // check for promotion attack right
      for (int i = 0; i < 8; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first += 1;
          initial.second -= 1;
          Move move = coordinatesToMove(initial, final);

          updateSpecialMove(move, PROMOTION_QUEEN);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_ROOK);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_BISHOP);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_KNIGHT);
          moves.emplace_back(move);
        }
      }
    }

    if (E_P != 0) {
      // todo: specialize this for white
      if (checker_zone != FILLED and ((E_P << 8) & checker_zone) != 0) {
        checker_zone |=
            (directional_mask[(int)log2(checker_zone)][FILES] & rank_3);
      }
      uint64_t P_EP_L = (P >> 9) & E_P & ~file_h & MASK & checker_zone;
      uint64_t P_EP_R = (P >> 7) & E_P & ~file_a & MASK & checker_zone;

      if (P_EP_L > 0u) {
        // check for en passant left
        bits = P_EP_L;
        for (int i = 16; i < 24; i++) {
          if (bits[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            std::pair<uint8_t, uint8_t> initial = final;
            initial.first += 1;
            initial.second += 1;
            Move move = coordinatesToMove(initial, final);
            updateSpecialMove(move, EN_PASSANT);
            moves.emplace_back(move);
          }
        }
      }

      if (P_EP_R > 0u) {
        // check for en passant right
        bits = P_EP_R;
        for (int i = 16; i < 24; i++) {
          if (bits[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            std::pair<uint8_t, uint8_t> initial = final;
            initial.first += 1;
            initial.second -= 1;
            Move move = coordinatesToMove(initial, final);
            updateSpecialMove(move, EN_PASSANT);
            moves.emplace_back(move);
          }
        }
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

    if (P_FORWARD_1 > 0u) {

      bits = P_FORWARD_1; // check to see if you can move 1
      for (int i = 16; i < 56; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 1;
          moves.emplace_back(coordinatesToMove(initial, final));
        }
      }
    }

    if (P_FORWARD_2 > 0u) {
      bits = P_FORWARD_2; // check to see if you can move 2
      for (int i = 24; i < 32; i++) {

        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 2;
          Move move = coordinatesToMove(initial, final);
          updateSpecialMove(move, PAWN_PUSH_2);
          moves.emplace_back(move);
        }
      }
    }

    if (P_ATTACK_L > 0u) {
      bits = P_ATTACK_L; // check for attacks left
      for (int i = 16; i < 56; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 1;
          initial.second += 1;
          moves.emplace_back(coordinatesToMove(initial, final));
        }
      }
    }

    if (P_ATTACK_R > 0u) {
      bits = P_ATTACK_R; // check for attacks right
      for (int i = 16; i < 56; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 1;
          initial.second -= 1;
          moves.emplace_back(coordinatesToMove(initial, final));
        }
      }
    }

    if (P_PROMO_1 > 0u) {
      bits = P_PROMO_1; // check for promotion straight
      for (int i = 56; i < 64; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 1;
          Move move = coordinatesToMove(initial, final);

          updateSpecialMove(move, PROMOTION_QUEEN);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_ROOK);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_BISHOP);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_KNIGHT);
          moves.emplace_back(move);
        }
      }
    }

    if (P_PROMO_L > 0u) {
      bits = P_PROMO_L; // check for promotion left
      for (int i = 56; i < 64; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 1;
          initial.second += 1;
          Move move = coordinatesToMove(initial, final);

          updateSpecialMove(move, PROMOTION_QUEEN);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_ROOK);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_BISHOP);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_KNIGHT);
          moves.emplace_back(move);
        }
      }
    }

    if (P_PROMO_R > 0u) {
      bits = P_PROMO_R; // check for promotion attack right
      for (int i = 56; i < 64; i++) {
        if (bits[i] == 1) {
          std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
          std::pair<uint8_t, uint8_t> initial = final;
          initial.first -= 1;
          initial.second -= 1;
          Move move = coordinatesToMove(initial, final);

          updateSpecialMove(move, PROMOTION_QUEEN);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_ROOK);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_BISHOP);
          moves.emplace_back(move);

          updateSpecialMove(move, PROMOTION_KNIGHT);
          moves.emplace_back(move);
        }
      }
    }

    if (E_P != 0) {
      if (checker_zone != FILLED and ((E_P >> 8) & checker_zone) != 0) {
        checker_zone |=
            (directional_mask[(int)log2(checker_zone)][FILES] & rank_6);
      }

      uint64_t P_EP_L = (P << 7) & E_P & ~file_h & MASK & checker_zone;
      uint64_t P_EP_R = (P << 9) & E_P & ~file_a & MASK & checker_zone;

      if (P_EP_L != 0u) {
        // check for en passant left
        bits = P_EP_L;
        for (int i = 40; i < 48; i++) {
          if (bits[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            std::pair<uint8_t, uint8_t> initial = final;
            initial.first -= 1;
            initial.second += 1;
            Move move = coordinatesToMove(initial, final);
            updateSpecialMove(move, EN_PASSANT);
            moves.emplace_back(move);
          }
        }
      }

      if (P_EP_R != 0u) {
        // check for en passant right
        bits = P_EP_R;
        for (int i = 40; i < 48; i++) {
          if (bits[i] == 1) {
            std::pair<uint8_t, uint8_t> final = bitToCoordinates[i];
            std::pair<uint8_t, uint8_t> initial = final;
            initial.first -= 1;
            initial.second -= 1;
            Move move = coordinatesToMove(initial, final);
            updateSpecialMove(move, EN_PASSANT);
            moves.emplace_back(move);
          }
        }
      }
    }
  }
}
void get_B_pawn_moves(uint64_t BP, uint64_t BK, uint64_t E_P, uint64_t EMPTY,
                      uint64_t WHITE_PIECES, uint64_t PINNED,
                      uint64_t checker_zone, uint64_t E_P_SPECIAL,
                      std::vector<Move> &b_moves) {

  uint64_t mask;
  uint64_t pinned_pawns = (BP & PINNED);
  if (checker_zone == 0) {
    checker_zone = FILLED;
  }
  if (pinned_pawns) { // we have at least 1 pawn pinned
    BP &= ~PINNED;
    for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
      if (pinned_pawns & 1) {
        uint64_t bb = (uint64_t)1 << bit;
        mask = (get_mask(bb, BK) | E_P_SPECIAL);
        get_X_pawn_moves("B", mask, bb, BK, E_P, EMPTY, WHITE_PIECES,
                         checker_zone, b_moves);
      }
      pinned_pawns >>= 1;
      if (!pinned_pawns) {
        break;
      }
    }
  }

  if (BP) { // we have at least 1 non-pinned pawn
    mask = FILLED;
    get_X_pawn_moves("B", mask, BP, BK, E_P, EMPTY, WHITE_PIECES, checker_zone,
                     b_moves);
  }
}
void get_W_pawn_moves(uint64_t WP, uint64_t WK, uint64_t E_P, uint64_t EMPTY,
                      uint64_t BLACK_PIECES, uint64_t PINNED,
                      uint64_t checker_zone, uint64_t E_P_SPECIAL,
                      std::vector<Move> &w_moves) {

  uint64_t mask;
  uint64_t pinned_pawns = (WP & PINNED);
  if (checker_zone == 0) {
    checker_zone = FILLED;
  }

  if (pinned_pawns) { // we have at least 1 pawn pinned
    WP &= ~PINNED;

    for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
      if (pinned_pawns & 1) {
        uint64_t bb = (uint64_t)1 << bit;
        mask = get_mask(bb, WK) | E_P_SPECIAL;
        get_X_pawn_moves("W", mask, bb, WK, E_P, EMPTY, BLACK_PIECES,
                         checker_zone, w_moves);
      }
      pinned_pawns >>= 1;
      if (!pinned_pawns) {
        break;
      }
    }
  }

  if (WP) { // we have at least 1 non-pinned pawn
    mask = FILLED;
    get_X_pawn_moves("W", mask, WP, WK, E_P, EMPTY, BLACK_PIECES, checker_zone,
                     w_moves);
  }
}

void get_K_castle(bool CK, uint64_t K, uint64_t EMPTY, uint64_t DZ,
                  std::vector<Move> &wb_moves) {
  if (CK) {
    // todo: implement lookup table
    if (((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)) != 0u) {
      uint8_t k_bit =
          (int)log2(((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)));

      std::pair<uint8_t, uint8_t> final = bitToCoordinates[k_bit];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.second -= 2;
      Move move = coordinatesToMove(initial, final);
      updateSpecialMove(move, CASTLE_KINGSIDE);
      wb_moves.emplace_back(move);
    }
  }
}
void get_Q_castle(bool QK, uint64_t K, uint64_t EMPTY, uint64_t DZ,
                  std::vector<Move> &wb_moves) {

  if (QK) {
    // todo: implement lookup table
    if ((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)) !=
        0u) {
      uint8_t k_bit = (int)log2((((K >> 2) & EMPTY) & (EMPTY >> 1) &
                                 (EMPTY << 1) & ~DZ & ~(DZ >> 1)));
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[k_bit];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.second += 2;
      Move move = coordinatesToMove(initial, final);
      updateSpecialMove(move, CASTLE_QUEENSIDE);
      wb_moves.emplace_back(move);
    }
  }
}

uint64_t unsafe_for_XK(std::string X, uint64_t P, uint64_t R, uint64_t N,
                       uint64_t B, uint64_t Q, uint64_t K, uint64_t EK,
                       uint64_t OCCUPIED) {

  uint64_t unsafe = 0u, D = B | Q, HV = R | Q;

  // pawn
  if (P != 0u) {
    if (X == "B") {
      unsafe = (P << 9) & ~file_a;  // capture right
      unsafe |= (P << 7) & ~file_h; // capture left
    } else {
      unsafe = (P >> 7) & ~file_a;  // capture right
      unsafe |= (P >> 9) & ~file_h; // capture left
    }
  }

  // knight
  uint64_t pos_moves;
  if (N) {
    // todo: is it really efficient to redefine these everytime? maybe can
    // optimize where this is defined assuming knight is at bit 21 or F3 or (x3,
    // y5)
    for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
      if (N & 1) {

        uint64_t bb = (uint64_t)1 << bit;
        // todo: make this a lookup table for improved performance
        uint8_t kn_bit = bit;
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
        // viz_bb(pos_n_moves);
        unsafe |= pos_moves;
      }
      N >>= 1;
      if (!N) {
        break;
      }
    }
  }

  // diag pieces (Bishop, Queen)
  if (D) {
    for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
      if (D & 1) {
        uint64_t bb = (uint64_t)1 << bit;
        uint8_t sl_bit = bit;
        unsafe |= diag_moves(bb, sl_bit, OCCUPIED, true, EK);
      }
      D >>= 1;
      if (!D) {
        break;
      }
    }
  }

  // hv pieces (Rook, Queen)
  if (HV) {
    for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
      if (HV & 1) {
        uint8_t sl_bit = bit;
        uint64_t bb = (uint64_t)1 << bit;
        unsafe |= h_v_moves(bb, sl_bit, OCCUPIED, true, EK);
      }
      HV >>= 1;
      if (!HV) {
        break;
      }
    }
  }

  // king
  uint8_t k_bit = 0;
  for (uint8_t bit = 0; bit < N_SQUARES; bit++) {
    if (K & 1) {
      k_bit = bit;
      break;
    }
    K >>= 1;
  }

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
  unsafe |= pos_moves;

  return unsafe;
}

void get_B_moves(GameState &gamestate, uint64_t E_P, bool &CM, bool &SM,
                 std::vector<Move> &b_moves) {

  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  uint64_t DZ = unsafe_for_XK("B", gamestate.white.pawn, gamestate.white.rook,
                              gamestate.white.knight, gamestate.white.bishop,
                              gamestate.white.queen, gamestate.white.king,
                              gamestate.black.king, OCCUPIED);
  b_moves.clear();
  uint64_t E_P_SPECIAL = 0u;

  // DZ is the danger zone. If the king is inside of it, its in check.
  int num_checkers = 0;
  uint64_t PINNED = get_pinned_pieces(
      gamestate.black.king, gamestate.black.pawn, gamestate.white.queen,
      gamestate.white.bishop, gamestate.white.rook, OCCUPIED, E_P,
      E_P_SPECIAL); // todo: need to put this to work. dont generate pinned
                    // moves if in check, skip that piece
  bool check = (DZ & gamestate.black.king) != 0u;
  // ------------------
  uint64_t checkers = 0, new_checker,
           checker_zone =
               0; // checker zone is the area that the piece is attacking
                  // through (applies only to sliders). We have the potential to
                  // block the check by moving  apiece in the line of fire
                  // (pinning your own piece)

  // ------------------
  if (check) { // currently in check
    // todo: generate checkers_bb, update_num_checkers. create method.
    uint64_t HV = gamestate.white.rook | gamestate.white.queen;

    int k_bit = (int)log2(gamestate.black.king);
    uint64_t K_moves;

    // check horizontal pieces
    K_moves = h_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker != 0u) {
      checkers |= new_checker;
      checker_zone |=
          h_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check vertical pieces
    K_moves = v_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          v_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    uint64_t D = gamestate.white.bishop | gamestate.white.queen;
    // check down and to the right pieces
    K_moves = ddr_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          ddr_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check up and to the right pieces
    K_moves = dur_moves(gamestate.black.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          dur_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check for knight attacks
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
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn right attack (from pawns perspective)
    K_moves = (gamestate.black.king >> 9) & ~file_h;
    new_checker = K_moves & gamestate.white.pawn;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn left attack (from pawns perspective)
    K_moves = (gamestate.black.king >> 7) & ~file_a;
    new_checker = K_moves & gamestate.white.pawn;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

  } else { // only search for castles if you aren't in check
    get_K_castle(gamestate.black.can_king_side_castle, gamestate.black.king,
                 ~OCCUPIED, DZ, b_moves);
    get_Q_castle(gamestate.black.can_queen_side_castle, gamestate.black.king,
                 ~OCCUPIED, DZ, b_moves);
  }

  checker_zone |= checkers;

  // todo: pass check zones into the files

  if (num_checkers < 2) {

    get_B_pawn_moves(gamestate.black.pawn, gamestate.black.king, E_P, ~OCCUPIED,
                     WHITE_PIECES, PINNED, checker_zone, E_P_SPECIAL, b_moves);
    get_rook_moves(gamestate.black.rook, gamestate.black.king, BLACK_PIECES,
                   OCCUPIED, PINNED, checker_zone, b_moves);
    get_bishop_moves(gamestate.black.bishop, gamestate.black.king, BLACK_PIECES,
                     OCCUPIED, PINNED, checker_zone, b_moves);
    get_queen_moves(gamestate.black.queen, gamestate.black.king, BLACK_PIECES,
                    OCCUPIED, PINNED, checker_zone, b_moves);
    get_knight_moves(gamestate.black.knight, gamestate.black.king, BLACK_PIECES,
                     PINNED, checker_zone, b_moves);
  }
  get_king_moves(gamestate.black.king, BLACK_PIECES, DZ, b_moves);

  if (b_moves.empty() and check) {
    CM = true;
  } else if (b_moves.empty() and !check) {
    SM = true;
  } else if ((gamestate.black.king | gamestate.white.king) == OCCUPIED) {
    SM = true;
  }

  // return check;
}

void get_W_moves(const GameState &gamestate, uint64_t E_P, bool &CM, bool &SM,
                 std::vector<Move> &w_moves) {

  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  w_moves.clear();

  uint64_t E_P_SPECIAL = 0u;

  uint64_t DZ = unsafe_for_XK("W", gamestate.black.pawn, gamestate.black.rook,
                              gamestate.black.knight, gamestate.black.bishop,
                              gamestate.black.queen, gamestate.black.king,
                              gamestate.white.king, OCCUPIED);

  // DZ is the danger zone. If the king is inside of it, its in check.
  int num_checkers = 0;
  uint64_t PINNED = get_pinned_pieces(
      gamestate.white.king, gamestate.white.pawn, gamestate.black.queen,
      gamestate.black.bishop, gamestate.black.rook, OCCUPIED, E_P,
      E_P_SPECIAL); // todo: need to put this to work. dont generate pinned
                    // moves if in check, skip that piece
  bool check = (DZ & gamestate.white.king) != 0u;
  // ------------------
  uint64_t checkers = 0, new_checker,
           checker_zone =
               0; // checker zone is the area that the piece is attacking
                  // through (applies only to sliders). We have the potential to
                  // block the check by moving  apiece in the line of fire
                  // (pinning your own piece)

  // ------------------
  if (check) { // currently in check
    // todo: generate checkers_bb, update_num_checkers. create method.
    uint64_t HV = gamestate.black.rook | gamestate.black.queen;
    int k_bit = (int)log2(gamestate.white.king);
    uint64_t K_moves;

    // check horizontal pieces
    K_moves = h_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker != 0u) {
      checkers |= new_checker;
      checker_zone |=
          h_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check vertical pieces
    K_moves = v_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & HV;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          v_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    uint64_t D = gamestate.black.bishop | gamestate.black.queen;
    // check down and to the right pieces
    K_moves = ddr_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          ddr_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check up and to the right pieces
    K_moves = dur_moves(gamestate.white.king, k_bit, OCCUPIED);
    new_checker = K_moves & D;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      checker_zone |=
          dur_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
      num_checkers++;
    }

    // check for knight attacks
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
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn right attack (from pawns perspective)
    K_moves = (gamestate.white.king << 7) & ~file_h; // todo: verify
    new_checker = K_moves & gamestate.black.pawn;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

    // check for pawn left attack (from pawns perspective)
    K_moves = (gamestate.white.king << 9) & ~file_a; // todo: verify
    new_checker = K_moves & gamestate.black.pawn;
    if (new_checker != 0u and num_checkers != 2) {
      checkers |= new_checker;
      num_checkers++;
    }

  } else { // only search for castles if you aren't in check
    get_K_castle(gamestate.white.can_king_side_castle, gamestate.white.king,
                 ~OCCUPIED, DZ, w_moves);
    get_Q_castle(gamestate.white.can_queen_side_castle, gamestate.white.king,
                 ~OCCUPIED, DZ, w_moves);
  }

  checker_zone |= checkers;
  // todo: pass check zones into the files

  if (num_checkers < 2) {
    get_W_pawn_moves(gamestate.white.pawn, gamestate.white.king, E_P, ~OCCUPIED,
                     BLACK_PIECES, PINNED, checker_zone, E_P_SPECIAL, w_moves);
    get_rook_moves(gamestate.white.rook, gamestate.white.king, WHITE_PIECES,
                   OCCUPIED, PINNED, checker_zone, w_moves);
    get_bishop_moves(gamestate.white.bishop, gamestate.white.king, WHITE_PIECES,
                     OCCUPIED, PINNED, checker_zone, w_moves);
    get_queen_moves(gamestate.white.queen, gamestate.white.king, WHITE_PIECES,
                    OCCUPIED, PINNED, checker_zone, w_moves);
    get_knight_moves(gamestate.white.knight, gamestate.white.king, WHITE_PIECES,
                     PINNED, checker_zone, w_moves);
  }
  get_king_moves(gamestate.white.king, WHITE_PIECES, DZ, w_moves);

  if (w_moves.empty() and check) {
    CM = true;
  } else if (w_moves.empty() and !check) {
    SM = true;
  } else if ((gamestate.black.king | gamestate.white.king) == OCCUPIED) {
    SM = true;
  }
}

void apply_move(Move move, GameState &gamestate, uint64_t &E_P) {

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

  // TODO: switch to bit shifting.
  uint64_t initial = pow(2, ((x1 * 8) + (y1 % 8)));
  uint64_t final = pow(2, ((x2 * 8) + (y2 % 8)));

  //-----discover what piece is being moved-----
  //    uint64_t moving_piece;
  //    if ((Q & initial) != 0){moving_piece = Q;}
  //    else if ((B & initial) != 0){moving_piece = B;}
  //    else if ((R & initial) != 0){moving_piece = R;}
  //    else if ((N & initial) != 0){moving_piece = N;}
  //    else if ((P & initial) != 0){moving_piece = P;}
  //    else if ((K & initial) != 0){moving_piece = K;}

  //-----remove enemy pieces in the case of a capture-----
  uint64_t capture;

  if (white_move) {
    capture = (BLACK_PIECES & ~OK) & final;
  } else {
    capture = WHITE_PIECES & ~OK & final;
  }

  if (capture != 0) {
    if ((OP & final) != 0) {
      OP &= ~final;
      // cap_counter++;
    } else if ((OR & final) != 0) {

      if (white_move) {

        if ((OR & 9223372036854775808u & final) == 9223372036854775808u) {
          BCK = false;
        } else if ((OR & 72057594037927936u & final) == 72057594037927936u) {
          BCQ = false;
        }
      } else {
        if ((OR & 128u & final) == 128u) {
          WCK = false;
        } else if ((OR & 1u & final) == 1u) {
          WCQ = false;
        }
      }

      OR &= ~final;
      // cap_counter++;
    } else if ((ON & final) != 0) {
      ON &= ~final;
      //  cap_counter++;
    } else if ((OB & final) != 0) {
      OB &= ~final;
      //   cap_counter++;
    } else if ((OQ & final) != 0) {
      OQ &= ~final;
      //   cap_counter++;
      // ;
    }

  } else if (E_P == final and
             ((initial & P) !=
              0)) { // this means there was an en passant capture

    if (white_move) {
      OP &= ~(final >> 8);
    } else {
      OP &= ~(final << 8);
    }
    // cap_counter++;
  }
  // -----this concludes removing enemy pieces from board-----

  // need to move piece to the final position and also remove the initial
  // position

  // No Special move
  // TODO: make switch statement here.
  if (special == NONE) {
    // todo: search for which piece is moving
    if ((Q & initial) != 0) {
      Q |= final;
      Q &= ~initial;
    } else if ((B & initial) != 0) {
      B |= final;
      B &= ~initial;
    } else if ((R & initial) != 0) {

      if (WCK and white_move and (R & 128u & initial) == 128u) {
        WCK = false;
      } else if (WCQ and white_move and (R & 1u & initial) == 1u) {
        WCQ = false;
      } else if (BCK and !white_move and
                 (R & 9223372036854775808u & initial) == 9223372036854775808u) {
        BCK = false;
      } else if (BCQ and !white_move and
                 (R & 72057594037927936u & initial) == 72057594037927936u) {
        BCQ = false;
      }
      R |= final;
      R &= ~initial;
    } else if ((N & initial) != 0) {
      N |= final;
      N &= ~initial;
    } else if ((P & initial) != 0) {
      P |= final;
      P &= ~initial;
    } else if ((K & initial) != 0) {
      if ((WCK or WCQ) and white_move) {
        WCK = false;
        WCQ = false;
      }
      if ((BCK or BCQ) and !white_move) {
        BCK = false;
        BCQ = false;
      }
      K = final;
    }

    E_P = 0u;
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
      E_P = 0u;
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
      E_P = 0u;
    } else if (special == PROMOTION_QUEEN) { // promotion
      P &= ~initial;
      Q |= final;
      E_P = 0u;
    } else if (special == PROMOTION_BISHOP) {
      P &= ~initial;
      B |= final;
      E_P = 0u;
    } else if (special == PROMOTION_KNIGHT) {
      P &= ~initial;
      N |= final;
      E_P = 0u;
    } else if (special == PROMOTION_ROOK) {
      P &= ~initial;
      R |= final;
      E_P = 0u;
    } else if (special == EN_PASSANT) { // en passant capture

      P |= final;
      P &= ~initial;
      E_P = 0u;
    } else { // pawn push 2

      P |= final;
      P &= ~initial;
      if (white_move) {
        E_P = (final >> 8);
      } else {
        E_P = (final << 8);
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

void print_moves(bool white_move, std::vector<Move> b_moves,
                 std::vector<Move> w_moves) {

  if (white_move) {
    std::cout << "WHITE'S MOVE: " << std::endl;
    for (int i = 0; i < w_moves.size(); i++) {
      std::cout << i + 1 << ": " + moveToString(w_moves[i]) << std::endl;
    }
    //  std::cout << "total moves: " << w_moves.size() << std::endl;
  } else {
    std::cout << "BLACK'S MOVE: " << std::endl;
    for (int i = 0; i < b_moves.size(); i++) {
      std::cout << i + 1 << ": " + moveToString(b_moves[i]) << std::endl;
    }
  }
}

bool aa = false;

void perft(uint32_t &nodes, uint32_t &cap_counter, GameState &gamestate,
           std::vector<Move> moves, uint64_t &E_P, bool CM, bool SM, int depth,
           int orig_depth, std::string n) {

  bool check = false;

  if (gamestate.whites_turn) {
    get_W_moves(gamestate, E_P, CM, SM, moves);
  } else {
    get_B_moves(gamestate, E_P, CM, SM, moves);
  }

  if (depth != 0) {

    for (int i = 0; i < moves.size(); i++) {
      int cap_count_temp = 0;

      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
      uint64_t E_Pt = E_P;
      bool CMt = false, SMt = false;
      apply_move(moves[i], gamestate_temp, E_Pt);

      //  std::cout<<"depth: "<< depth<<std::endl;
      //  std::cout<<"nodes: "<< nodes<<std::endl;

      if (depth == 1) {
        nodes++;
        //  cap_counter += cap_count_temp;
      }
      //  else if (CMt or)
      perft(nodes, cap_counter, gamestate_temp, moves, E_Pt, CMt, SMt,
            depth - 1, orig_depth, n);

      if (depth == orig_depth) {
        //          viz_bb( WPt);
        if (n == "total") {
          std::cout << round(((i * 100 / moves.size())))
                    << "% complete... -> d1:" << moveToString(moves[i])
                    << "--------------------------------------------------"
                    << std::endl;

        } else if (n == "node") {
          std::cout << i << ":" << moveToString(moves[i]) << " " << nodes
                    << std::endl;
          nodes = 0;
        }

      } else if (depth == orig_depth - 1 and false) {
        if (n == "total") {
          std::cout << round(((i * 100 / moves.size())))
                    << "% complete... -> d1:" << moveToString(moves[i])
                    << "--------------------------------------------------"
                    << std::endl;
        } else if (n == "node") {
          std::cout << "     " << i << ":" << moveToString(moves[i]) << " "
                    << nodes << std::endl;
          //   nodes = 0;
        }
      }
    }
  }
}

int nodes2 = 0;

double eval(const GameState gamestate) {

  // material
  double counter = 0;
  counter += ((double)std::bitset<64>(gamestate.white.pawn).count() -
              (double)std::bitset<64>(gamestate.black.pawn).count()) *
             100;
  counter += ((double)std::bitset<64>(gamestate.white.bishop).count() -
              (double)std::bitset<64>(gamestate.black.bishop).count()) *
             300; // todo: add special case regarding number of bishops
  counter += ((double)std::bitset<64>(gamestate.white.knight).count() -
              (double)std::bitset<64>(gamestate.black.knight).count()) *
             300;
  counter += ((double)std::bitset<64>(gamestate.white.rook).count() -
              (double)std::bitset<64>(gamestate.black.rook).count()) *
             500;
  counter += ((double)std::bitset<64>(gamestate.white.queen).count() -
              (double)std::bitset<64>(gamestate.black.queen).count()) *
             900;

  return counter;
}

AI_return minimax(GameState &gamestate, uint64_t E_P, bool CM, bool SM,
                  int depth, bool my_turn, double alpha = -100000000,
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
    std::vector<Move> w_moves;

    Move max_move;
    double max_val = -10000000;
    AI_return a;

    get_W_moves(gamestate, E_P, CM, SM, w_moves);
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

    for (int i = 0; i < w_moves.size(); i++) {

      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));

      uint64_t E_Pt = E_P;
      bool CMt = false, SMt = false;

      apply_move(w_moves[i], gamestate_temp, E_Pt);

      a = minimax(gamestate_temp, E_Pt, CMt, SMt, depth - 1, !my_turn, alpha,
                  beta);

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
    std::vector<Move> b_moves;

    Move min_move;
    double min_val = 10000000;
    AI_return a;

    get_B_moves(gamestate, E_P, CM, SM, b_moves);

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

    for (int j = 0; j < b_moves.size(); j++) {

      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
      uint64_t E_Pt = E_P;
      bool CMt = false, SMt = false;

      apply_move(b_moves[j], gamestate_temp, E_Pt);

      a = minimax(gamestate_temp, E_Pt, CMt, SMt, depth - 1, !my_turn, alpha,
                  beta);

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
  uint8_t byte = 7;
  uint8_t bit = 0;

  // Populate piece positions.
  for (int i = 0; i < fen.find(' '); i++) {
    // Check for empty positions.
    if (isdigit(fen[i])) {
      bit += fen[i] - '0';
    }
    switch (fen[i]) {
    case 'p':
      gamestate.black.pawn += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'r':
      gamestate.black.rook += (1ull << (byte * 8 + bit));
      bit++;
      break;
    case 'n':
      gamestate.black.knight += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'b':
      gamestate.black.bishop += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'q':
      gamestate.black.queen += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'k':
      gamestate.black.king += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'P':
      gamestate.white.pawn += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'R':
      gamestate.white.rook += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'N':
      gamestate.white.knight += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'B':
      gamestate.white.bishop += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'Q':
      gamestate.white.queen += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case 'K':
      gamestate.white.king += 1ull << (byte * 8 + bit);
      bit++;
      break;
    case '/':
      byte -= 1;
      bit = 0;
      break;
    default:
      // todo: add error catch here.
      break;
    }
  }

  // Populate extra game state data.
  for (int i = fen.find(' ') + 1; i < fen.length(); i++) {
    switch (fen[i]) {
    case 'w':
      gamestate.whites_turn = true;
      break;
    case 'b':
      gamestate.whites_turn = false;
      break;
    case 'K':
      gamestate.white.can_king_side_castle = true;
      break;
    case 'Q':
      gamestate.white.can_queen_side_castle = true;
      break;
    case 'k':
      gamestate.black.can_king_side_castle = true;
      break;
    case 'q':
      gamestate.black.can_queen_side_castle = true;
      break;
    default:
      // todo: add error checking here.
      break;
    }
  }
}

void read_FEN(char g[8][8], std::string FEN, bool &white_move, bool &WCK,
              bool &WCQ, bool &BCK, bool &BCQ) {
  int row = 0;
  int col = 0;

  for (int i = 0; i < FEN.length(); i++) {

    if (row > 7) {

      // board has been read in
      if (FEN[i] == 'w') {
        white_move = true;
      }
      if (FEN[i] == 'b') {
        white_move = false;
      }
      if (FEN[i] == 'k') {
        BCK = true;
      }
      if (FEN[i] == 'q') {
        BCQ = true;
      }
      if (FEN[i] == 'K') {
        WCK = true;
      }
      if (FEN[i] == 'Q') {
        WCQ = true;
      }

      // break;

    } else {

      if (isdigit(FEN[i])) {
        for (int j = 0; j < (int)(FEN[i]) - 48; j++) {
          g[row][col] = ' ';
          col = (col + 1) % 8;
          if (col == 0) {
            row += 1;
          }
        }

      } else if (FEN[i] == '/') {
      } else {
        g[row][col] = FEN[i];
        col = (col + 1) % 8;
        if (col == 0) {
          row += 1;
        }
      }
    }
  }
} // end of readFEN

void generate_board(std::string name, int diff) {
  //

  std::cout << "GAME START" << std::endl;

  char grid[8][8] = {//  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 7
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 6
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 5
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 4
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 3
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 2
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},  // 1
                     {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}}; // 0

  std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  // FEN = "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1";
  // FEN= "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2";
  bool BCK = false, BCQ = false, WCK = false, WCQ = false, CM = false,
       SM = false, white_to_move;
  // todo: add E_P functionality to read FEN
  uint64_t E_P = 0u;

  GameState gamestate;
  fenToGameState(FEN, gamestate);

  AI_return AI_choice;

  int depth;
  if (diff == 1) {
    depth = 4;
  } else if (diff == 2) {
    depth = 5;
  } else if (diff == 3) {
    depth = 6;
  }

  // This is the GAME
  // for now, the AI is only white
  // todo: implement AI for both colors
  srand(time(nullptr));

  Player p = Player(true);
  Player p2 = Player(true, true);
  std::cout << p.color << std::endl;

  while (!CM and !SM and true) {

  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    if (gamestate.whites_turn) {
      // print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

      std::cout << "WHITE'S MOVE: " << std::endl;
      std::cout << "AI Agent thinking... wait a few seconds." << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      // std::cout<<"WHITES MOVE (SHOULD BE 1): "<<white_to_move<<std::endl;

      AI_choice = minimax(gamestate, E_P, CM, SM, depth, true);
      auto end = std::chrono::high_resolution_clock::now();

      std::cout << "Move chosen: " << moveToString(AI_choice.move) << std::endl;
      std::cout << AI_choice.value << std::endl;
      std::cout << "WHITES MOVE (SHOULD BE 1): " << white_to_move << std::endl;

      apply_move(AI_choice.move, gamestate, E_P);

      std::cout << "depth: " << depth << ". time elapsed: "
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
      std::vector<Move> b_moves;

      // TODO: uncomment this and fix
      get_B_moves(gamestate, E_P, CM, SM, b_moves);

      std::cout << "Please select your move: " << std::endl;
      print_moves(white_to_move, b_moves, b_moves);

      int user_choice;
      std::cin >> user_choice;

      apply_move(b_moves[user_choice - 1], gamestate, E_P);

      std::cout << "Move chosen: " << moveToString(b_moves[user_choice - 1])
                << std::endl;
      std::cout << " " << std::endl;
    }
  }
}
