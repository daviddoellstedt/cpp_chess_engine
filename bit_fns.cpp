#include "bit_fns.h"
#include "Players.h"
#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include "move_generator.h"
#include <bitset>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <regex>
#include <stdint.h>
#include <string>

// TODO: could cleanup and move definition outside of class for readability.
class Move {
private:
  /**Moves are stored as a 16-bit integer. Kept as lightweight as possible to
   * allow deeper search. Bits: 0 - 2:   Initial x position 3 - 5:   Initial y
   * position 6 - 8:   Final x position 9 - 11:  Final y position 12 - 15:
   * Special move flags (see SpecialMove enum)
   */
  uint16_t data = 0;
  void setX1(uint8_t x1) { data |= x1; }
  void setY1(uint8_t y1) { data |= y1 << 3; }
  void setX2(uint8_t x2) { data |= x2 << 6; }
  void setY2(uint8_t y2) { data |= y2 << 9; }
  std::string specialToString(void) {
    switch (getSpecial()) {
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

public:
  uint8_t getX1(void) { return data & X_INITIAL; }
  uint8_t getY1(void) { return (data & Y_INITIAL) >> 3; }
  uint8_t getX2(void) { return (data & X_FINAL) >> 6; }
  uint8_t getY2(void) { return (data & Y_FINAL) >> 9; }
  SpecialMove getSpecial(void) { return (SpecialMove)((data & SPECIAL) >> 12); }
  void setSpecial(SpecialMove special_move) {
    data &= ~SPECIAL;
    data |= special_move << 12;
  }
  std::string toString(void) {
    return (char)('a' + getY1()) + std::to_string(getX1() + 1) +
           (char)('a' + getY2()) + std::to_string(getX2() + 1) + " " +
           specialToString();
  }
  Move() {};
  Move(std::pair<uint8_t, uint8_t> initial, std::pair<uint8_t, uint8_t> final,
       SpecialMove special) {
    setX1(initial.first);
    setY1(initial.second);
    setX2(final.first);
    setY2(final.second);
    setSpecial(special);
  }
  Move(std::pair<uint8_t, uint8_t> initial, std::pair<uint8_t, uint8_t> final) {
    setX1(initial.first);
    setY1(initial.second);
    setX2(final.first);
    setY2(final.second);
  }
};

struct AI_return {
  Move move;
  int16_t value = 0;
  uint32_t nodes_searched = 0;
};

void printBitboard(uint64_t bb) {
  for (int i = 56; i >= 0; i -= 8) {
    std::bitset<8> bb_bitset((bb >> i) & 0xFF);
    std::cout << bb_bitset << std::endl;
  }
}

/** Function that returns a bitboard mask of the straight line between two
 * pieces. Inputs need to be colinear at a diagonal or orthogonal
 * perspective. More or less a lookup table.
 *
 * @param p1: first piece
 * @param p2: second piece
 * @return bitboard mask of rank/file/diagonal connection between the two pieces
 */
uint64_t getMask(uint64_t p1, uint64_t p2) {
  uint8_t k_bit = getSetBit(p2);
  uint8_t p_bit = getSetBit(p1);

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
uint64_t getPinnedPieces(uint64_t K, uint64_t P, uint64_t EQ, uint64_t EB,
                         uint64_t ER, uint64_t OCCUPIED, uint64_t &E_P,
                         bool white_to_move) {
  uint64_t PINNED = 0;
  uint8_t k_bit = getSetBit(K);

  // Horizontal check.
  uint64_t K_h_v_slider = h_v_moves(K, OCCUPIED);
  uint64_t K_slider = K_h_v_slider & directional_mask[k_bit][RANKS];

  uint64_t EHV = EQ | ER;
  while (EHV) {
    uint64_t bb = getLowestSetBitValue(EHV);
    uint64_t H_moves =
        h_v_moves(bb, OCCUPIED) & directional_mask[getSetBit(bb)][RANKS];

    // Check for special en passant pins.
    uint64_t ep_pawn = white_to_move ? E_P >> 8 : E_P << 8;
    uint64_t p_can_capture_ep_r =
        P & (white_to_move ? (E_P >> 9) & ~file_h : (E_P << 7) & ~file_a);
    uint64_t p_can_capture_ep_l =
        P & (white_to_move ? (E_P >> 7) & ~file_a : (E_P << 9) & ~file_h);
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
  K_slider = K_h_v_slider & directional_mask[k_bit][FILES];

  while (EHV) {
    uint64_t bb = getLowestSetBitValue(EHV);
    PINNED |= K_slider & h_v_moves(bb, OCCUPIED) &
              directional_mask[getSetBit(bb)][FILES];
    clearLowestSetBit(EHV);
  }

  // Down right diagonal check.
  uint64_t K_slider_diag = diag_moves(K, OCCUPIED);
  uint64_t ED = EQ | EB;
  K_slider = K_slider_diag & directional_mask[k_bit][DIAGONALS_DOWN_RIGHT];
  while (ED) {
    uint64_t bb = getLowestSetBitValue(ED);
    PINNED |= K_slider & diag_moves(bb, OCCUPIED) &
              directional_mask[getSetBit(bb)][DIAGONALS_DOWN_RIGHT];

    clearLowestSetBit(ED);
  }

  // Upper right diagonal check.
  ED = EQ | EB;
  K_slider = K_slider_diag & directional_mask[k_bit][DIAGONALS_UP_RIGHT];
  while (ED) {
    uint64_t bb = getLowestSetBitValue(ED);
    PINNED |= K_slider & diag_moves(bb, OCCUPIED) &
              directional_mask[getSetBit(bb)][DIAGONALS_UP_RIGHT];
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
 * @param moves: list of all possible moves for the input player. Output
 * will be appended to this list.
 */
void generateRookMoves(uint64_t R, uint64_t K, uint64_t PIECES,
                       uint64_t OCCUPIED, uint64_t PINNED,
                       uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (R) {

    uint64_t bb = getLowestSetBitValue(R);
    uint8_t bit = getSetBit(bb);

    uint64_t mask = bb & PINNED ? getMask(bb, K) : FILLED;

    uint64_t possible_moves =
        h_v_moves(bb, OCCUPIED, false, 0u) & ~PIECES & mask & checker_zone;

    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[bit];
    while (possible_moves) {
      uint64_t final_bb = getLowestSetBitValue(possible_moves);
      uint8_t final_bit = getSetBit(final_bb);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[final_bit];
      // moves[n_moves++] = coordinatesToMove(initial, final);
      moves[n_moves++] = Move(initial, final);
      clearLowestSetBit(possible_moves);
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
 * @param moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void generateBishopMoves(uint64_t B, uint64_t K, uint64_t PIECES,
                         uint64_t OCCUPIED, uint64_t PINNED,
                         uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (B) {
    uint64_t bb = getLowestSetBitValue(B);
    uint8_t bit = getSetBit(bb);
    uint64_t mask = bb & PINNED ? getMask(bb, K) : FILLED;
    uint64_t possible_moves =
        diag_moves(bb, OCCUPIED) & ~PIECES & mask & checker_zone;

    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[bit];
    while (possible_moves) {
      uint64_t bb_final = getLowestSetBitValue(possible_moves);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb_final)];
      moves[n_moves++] = Move(initial, final);

      clearLowestSetBit(possible_moves);
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
 * @param moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void generateQueenMoves(uint64_t Q, uint64_t K, uint64_t PIECES,
                        uint64_t OCCUPIED, uint64_t PINNED,
                        uint64_t checker_zone, Move *moves, uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (Q) {
    uint64_t bb = getLowestSetBitValue(Q);
    uint8_t bit = getSetBit(bb);
    uint64_t mask = bb & PINNED ? getMask(bb, K) : FILLED;
    uint64_t possible_moves =
        (h_v_moves(bb, OCCUPIED) | diag_moves(bb, OCCUPIED)) & ~PIECES & mask &
        checker_zone;

    std::pair<uint8_t, uint8_t> initial = bitToCoordinates[bit];
    while (possible_moves) {
      uint64_t bb_final = getLowestSetBitValue(possible_moves);
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb_final)];
      moves[n_moves++] = Move(initial, final);

      clearLowestSetBit(possible_moves);
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
 * @param moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void generateKnightMoves(uint64_t N, uint64_t K, uint64_t PIECES,
                         uint64_t PINNED, uint64_t checker_zone, Move *moves,
                         uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  while (N) {
    uint64_t bb = getLowestSetBitValue(N);
    uint8_t kn_bit = getSetBit(bb);

    if (!(bb & PINNED)) { // only check for moves if it's not pinned.
                          // pinned knights cannot move.
      uint64_t pos_moves =
          kn_bit > KNIGHT_MASK_BIT_POSITION
              ? KNIGHT_MOVES << (kn_bit - KNIGHT_MASK_BIT_POSITION)
              : KNIGHT_MOVES >> (KNIGHT_MASK_BIT_POSITION - kn_bit);
      pos_moves &=
          ~PIECES & checker_zone & (kn_bit % 8 > 3 ? ~file_ab : ~file_gh);

      std::pair<uint8_t, uint8_t> initial = bitToCoordinates[kn_bit];
      while (pos_moves) {
        uint64_t bb_final = getLowestSetBitValue(pos_moves);
        std::pair<uint8_t, uint8_t> final =
            bitToCoordinates[getSetBit(bb_final)];
        moves[n_moves++] = Move(initial, final);

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
 * @param moves: list of all possible moves for the inpout player. output
 * will be appended to this variable.
 */
void generateKingMoves(uint64_t K, uint64_t PIECES, uint64_t DZ, Move *moves,
                       uint8_t &n_moves) {
  uint8_t k_bit = getSetBit(K);
  uint64_t pos_moves = k_bit > KING_MASK_BIT_POSITION
                           ? KING_MOVES << (k_bit - KING_MASK_BIT_POSITION)
                           : KING_MOVES >> (KING_MASK_BIT_POSITION - k_bit);
  pos_moves &= ~PIECES & ~DZ & (k_bit % 8 > 3 ? ~file_a : ~file_h);

  std::pair<uint8_t, uint8_t> initial = bitToCoordinates[k_bit];
  while (pos_moves) {
    uint64_t bb_final = getLowestSetBitValue(pos_moves);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb_final)];
    moves[n_moves++] = Move(initial, final);

    clearLowestSetBit(pos_moves);
  }
}

void generatePawnMoves(bool white_to_move, uint64_t MASK, uint64_t P,
                       uint64_t K, uint64_t E_P, uint64_t EMPTY,
                       uint64_t ENEMY_PIECES, uint64_t checker_zone,
                       Move *moves, uint8_t &n_moves) {
  uint64_t P_FORWARD_1 =
      EMPTY & MASK & checker_zone &
      (white_to_move ? (P << 8) & ~rank_8 : (P >> 8) & ~rank_1);
  uint64_t P_FORWARD_2 = EMPTY & MASK & checker_zone &
                         (white_to_move ? (P << 16) & (EMPTY << 8) & rank_4
                                        : (P >> 16) & (EMPTY >> 8) & rank_5);
  uint64_t P_ATTACK_L =
      ENEMY_PIECES & ~file_h & MASK & checker_zone &
      (white_to_move ? (P << 7) & ~rank_8 : (P >> 9) & ~rank_1);
  uint64_t P_ATTACK_R =
      ENEMY_PIECES & ~file_a & MASK & checker_zone &
      (white_to_move ? (P << 9) & ~rank_8 : (P >> 7) & ~rank_1);
  uint64_t P_PROMO_1 = EMPTY & MASK & checker_zone &
                       (white_to_move ? (P << 8) & rank_8 : (P >> 8) & rank_1);
  uint64_t P_PROMO_L = ENEMY_PIECES & ~file_h & MASK & checker_zone &
                       (white_to_move ? (P << 7) & rank_8 : (P >> 9) & rank_1);
  uint64_t P_PROMO_R = ENEMY_PIECES & ~file_a & MASK & checker_zone &
                       (white_to_move ? (P << 9) & rank_8 : (P >> 7) & rank_1);

  checker_zone |= white_to_move && ((E_P >> 8) & checker_zone) ? E_P : 0;
  checker_zone |= !white_to_move && ((E_P << 8) & checker_zone) ? E_P : 0;
  uint64_t P_EP_L =
      E_P & ~file_h & MASK & checker_zone & (white_to_move ? P << 7 : P >> 9);
  uint64_t P_EP_R =
      E_P & ~file_a & MASK & checker_zone & (white_to_move ? P << 9 : P >> 7);

  // CHECK TO SEE IF WE CAN MOVE 1 SPACE FORWARD
  while (P_FORWARD_1) {
    uint64_t bb = getLowestSetBitValue(P_FORWARD_1);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    moves[n_moves++] = Move(initial, final);
    clearLowestSetBit(P_FORWARD_1);
  }

  // check to see if you can move 2
  while (P_FORWARD_2) {
    uint64_t bb = getLowestSetBitValue(P_FORWARD_2);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -2 : 2;
    Move move = Move(initial, final, PAWN_PUSH_2);
    moves[n_moves++] = move;
    clearLowestSetBit(P_FORWARD_2);
  }

  // check for attacks left
  while (P_ATTACK_L) {
    uint64_t bb = getLowestSetBitValue(P_ATTACK_L);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    initial.second += white_to_move ? 1 : 1;
    moves[n_moves++] = Move(initial, final);
    clearLowestSetBit(P_ATTACK_L);
  }

  // check for attacks right
  while (P_ATTACK_R) {
    uint64_t bb = getLowestSetBitValue(P_ATTACK_R);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    initial.second += white_to_move ? -1 : -1;
    moves[n_moves++] = Move(initial, final);
    clearLowestSetBit(P_ATTACK_R);
  }

  // check for promotion straight
  while (P_PROMO_1) {
    uint64_t bb = getLowestSetBitValue(P_PROMO_1);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    Move move = Move(initial, final, PROMOTION_QUEEN);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_ROOK);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_BISHOP);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_KNIGHT);
    ;
    moves[n_moves++] = move;

    clearLowestSetBit(P_PROMO_1);
  }

  // check for promotion left
  while (P_PROMO_L) {
    uint64_t bb = getLowestSetBitValue(P_PROMO_L);

    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    initial.second += white_to_move ? 1 : 1;
    Move move = Move(initial, final, PROMOTION_QUEEN);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_ROOK);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_BISHOP);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_KNIGHT);
    ;
    moves[n_moves++] = move;

    clearLowestSetBit(P_PROMO_L);
  }

  // check for promotion attack right
  while (P_PROMO_R) {
    uint64_t bb = getLowestSetBitValue(P_PROMO_R);

    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    initial.second += white_to_move ? -1 : -1;
    Move move = Move(initial, final, PROMOTION_QUEEN);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_ROOK);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_BISHOP);
    moves[n_moves++] = move;
    move.setSpecial(PROMOTION_KNIGHT);
    ;
    moves[n_moves++] = move;

    clearLowestSetBit(P_PROMO_R);
  }

  // check for en passant left
  while (P_EP_L) {
    uint64_t bb = getLowestSetBitValue(P_EP_L);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    initial.second += white_to_move ? 1 : 1;
    Move move = Move(initial, final, EN_PASSANT);
    moves[n_moves++] = move;
    clearLowestSetBit(P_EP_L);
  }

  // check for en passant right
  while (P_EP_R) {
    uint64_t bb = getLowestSetBitValue(P_EP_R);
    std::pair<uint8_t, uint8_t> final = bitToCoordinates[getSetBit(bb)];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.first += white_to_move ? -1 : 1;
    initial.second += white_to_move ? -1 : -1;
    Move move = Move(initial, final, EN_PASSANT);
    moves[n_moves++] = move;
    clearLowestSetBit(P_EP_R);
  }
}

void generatePinnedPawnMoves(bool white_to_move, uint64_t &P, uint64_t K,
                             uint64_t E_P, uint64_t EMPTY,
                             uint64_t ENEMY_PIECES, uint64_t PINNED,
                             uint64_t checker_zone, Move *moves,
                             uint8_t &n_moves) {
  uint64_t pinned_pawns = P & PINNED;
  while (pinned_pawns) {
    uint64_t bb = getLowestSetBitValue(pinned_pawns);
    uint64_t mask = getMask(bb, K);
    generatePawnMoves(white_to_move, mask, bb, K, E_P, EMPTY, ENEMY_PIECES,
                      checker_zone, moves, n_moves);
    clearLowestSetBit(pinned_pawns);
  }
  // Clear pinned pawns from pawn bitboard.
  P &= ~PINNED;
}

void generateBlackPawnMoves(bool white_to_move, uint64_t BP, uint64_t BK,
                            uint64_t E_P, uint64_t EMPTY, uint64_t WHITE_PIECES,
                            uint64_t PINNED, uint64_t checker_zone, Move *moves,
                            uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  generatePinnedPawnMoves(white_to_move, BP, BK, E_P, EMPTY, WHITE_PIECES,
                          PINNED, checker_zone, moves, n_moves);

  if (BP) { // we have at least 1 non-pinned pawn.
    generatePawnMoves(white_to_move, FILLED, BP, BK, E_P, EMPTY, WHITE_PIECES,
                      checker_zone, moves, n_moves);
  }
}

void generateWhitePawnMoves(bool white_to_move, uint64_t WP, uint64_t WK,
                            uint64_t E_P, uint64_t EMPTY, uint64_t BLACK_PIECES,
                            uint64_t PINNED, uint64_t checker_zone, Move *moves,
                            uint8_t &n_moves) {
  if (!checker_zone) {
    checker_zone = FILLED;
  }

  generatePinnedPawnMoves(white_to_move, WP, WK, E_P, EMPTY, BLACK_PIECES,
                          PINNED, checker_zone, moves, n_moves);

  if (WP) { // we have at least 1 non-pinned pawn
    generatePawnMoves(white_to_move, FILLED, WP, WK, E_P, EMPTY, BLACK_PIECES,
                      checker_zone, moves, n_moves);
  }
}

void generateKingsideCastleMove(bool CK, uint64_t K, uint64_t EMPTY,
                                uint64_t DZ, Move *moves, uint8_t &n_moves) {
  if (!CK) {
    return;
  }
    // todo: implement lookup table
  if ((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)) {
    uint8_t k_bit =
        getSetBit((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1));

    std::pair<uint8_t, uint8_t> final = bitToCoordinates[k_bit];
    std::pair<uint8_t, uint8_t> initial = final;
    initial.second -= 2;
    Move move = Move(initial, final, CASTLE_KINGSIDE);
    moves[n_moves++] = move;
  }
}

void generateQueensideCastleMove(bool QK, uint64_t K, uint64_t EMPTY,
                                 uint64_t DZ, Move *moves, uint8_t &n_moves) {
  if (!QK) {
    return;
  }

    if ((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)) !=
        0u) {
      uint8_t k_bit = getSetBit(((K >> 2) & EMPTY) & (EMPTY >> 1) &
                                (EMPTY << 1) & ~DZ & ~(DZ >> 1));
      std::pair<uint8_t, uint8_t> final = bitToCoordinates[k_bit];
      std::pair<uint8_t, uint8_t> initial = final;
      initial.second += 2;
      Move move = Move(initial, final, CASTLE_QUEENSIDE);
      moves[n_moves++] = move;
    }
}

uint64_t getPawnAttackZone(bool white_to_move, uint64_t P) {
  return white_to_move ? ((P >> 7) & ~file_a) | ((P >> 9) & ~file_h)
                       : ((P << 9) & ~file_a) | ((P << 7) & ~file_h);
}

uint64_t getRookQueenAttackZone(uint64_t K, uint64_t ER, uint64_t EQ,
                                uint64_t OCCUPIED) {
  uint64_t EHV = ER | EQ;
  uint64_t DZ = 0;
  while (EHV) {
    uint64_t hv_piece = getLowestSetBitValue(EHV);
    DZ |= h_v_moves(hv_piece, OCCUPIED, true, K);
    clearLowestSetBit(EHV);
  }
  return DZ;
}

uint64_t getBishopQueenAttackZone(uint64_t K, uint64_t EB, uint64_t EQ,
                                  uint64_t OCCUPIED) {
  uint64_t ED = EB | EQ;
  uint64_t DZ = 0;
  while (ED) {
    uint64_t diag_piece = getLowestSetBitValue(ED);
    DZ |= diag_moves(diag_piece, OCCUPIED, true, K);

    clearLowestSetBit(ED);
  }
  return DZ;
}

uint64_t getKnightAttackZone(uint64_t N) {
  uint64_t DZ = 0;
  while (N) {
    uint8_t kn_bit = getSetBit(getLowestSetBitValue(N));
    uint64_t pos_moves =
        kn_bit > KNIGHT_MASK_BIT_POSITION
            ? KNIGHT_MOVES << (kn_bit - KNIGHT_MASK_BIT_POSITION)
            : KNIGHT_MOVES >> (KNIGHT_MASK_BIT_POSITION - kn_bit);
    pos_moves &= kn_bit % 8 > 3 ? ~file_ab : ~file_gh;
    DZ |= pos_moves;
    clearLowestSetBit(N);
  }
  return DZ;
}

uint64_t getKingAttackZone(uint64_t K) {
  uint8_t k_bit = getSetBit(K);
  uint64_t pos_moves = k_bit > KING_MASK_BIT_POSITION
                           ? KING_MOVES << (k_bit - KING_MASK_BIT_POSITION)
                           : KING_MOVES >> (KING_MASK_BIT_POSITION - k_bit);
  pos_moves &= k_bit % 8 > 3 ? ~file_a : ~file_h;
  return pos_moves;
}

// Check horizontal/vertical pieces. Note: only one horizontal/vertical slider
// can be checking a king at a time.
uint8_t getHorizontalAndVerticalChecker(uint64_t K, uint64_t ER, uint64_t EQ,
                                        uint64_t OCCUPIED,
                                        uint64_t &checker_zone) {
  uint64_t EHV = ER | EQ;
  uint64_t K_exposure = h_v_moves(K, OCCUPIED);
  uint64_t new_checker = K_exposure & EHV;
  if (new_checker) {
    checker_zone |= new_checker;
    checker_zone |= h_v_moves(new_checker, OCCUPIED) & K_exposure;
    return 1;
  }
  return 0;
}

// Check horizontal/vertical pieces. Note: only one diagonal slider
// can be checking a king at a time.
uint8_t getDiagonalChecker(uint64_t K, uint64_t EB, uint64_t EQ,
                           uint64_t OCCUPIED, uint64_t &checker_zone) {
  uint64_t ED = EB | EQ;
  uint64_t K_exposure = diag_moves(K, OCCUPIED);
  uint64_t new_checker = K_exposure & ED;
  if (new_checker) {
    checker_zone |= new_checker;
    checker_zone |= diag_moves(new_checker, OCCUPIED) & K_exposure;
    return 1;
  }
  return 0;
}

// Check knight pieces.
uint8_t getKnightChecker(uint64_t K, uint64_t EN, uint64_t OCCUPIED,
                         uint64_t &checker_zone) {
  // Check for knight attacks.
  uint64_t k_bit = getSetBit(K);
  uint64_t K_exposure =
      k_bit > KNIGHT_MASK_BIT_POSITION
          ? KNIGHT_MOVES << (k_bit - KNIGHT_MASK_BIT_POSITION)
          : KNIGHT_MOVES >> (KNIGHT_MASK_BIT_POSITION - k_bit);
  K_exposure &= k_bit % 8 > 3 ? ~file_ab : ~file_gh;

  uint64_t new_checker = K_exposure & EN;
  if (new_checker) {
    checker_zone |= new_checker;
    return 1;
  }
  return 0;
}

uint8_t getPawnChecker(bool white_to_move, uint64_t K, uint64_t EP,
                       uint64_t &checker_zone) {

  if (white_to_move) {
    // Check for pawn right attack (from pawns perspective).
    uint64_t K_exposure = (K << 7) & ~file_h;
    uint64_t new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }

    // check for pawn left attack (from pawns perspective)
    K_exposure = (K << 9) & ~file_a;
    new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }

  } else {
    // Check for pawn right attack (from pawns perspective).
    uint64_t K_exposure = (K >> 9) & ~file_h;
    uint64_t new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }

    // check for pawn left attack (from pawns perspective)
    K_exposure = (K >> 7) & ~file_a;
    new_checker = K_exposure & EP;
    if (new_checker) {
      checker_zone |= new_checker;
      return 1;
    }
  }

  return 0;
}

bool isInCheck(bool white_to_move, uint64_t K, PlayerState enemy_player_state,
               uint64_t OCCUPIED, uint64_t &DZ, uint64_t &checker_zone,
               uint8_t &n_checkers) {
  DZ |= getPawnAttackZone(white_to_move, enemy_player_state.pawn);
  DZ |= getRookQueenAttackZone(K, enemy_player_state.rook,
                               enemy_player_state.queen, OCCUPIED);
  DZ |= getBishopQueenAttackZone(K, enemy_player_state.bishop,
                                 enemy_player_state.queen, OCCUPIED);
  DZ |= getKnightAttackZone(enemy_player_state.knight);
  DZ |= getKingAttackZone(enemy_player_state.king);

  bool check = K & DZ;
  if (check) {
    n_checkers +=
        getPawnChecker(white_to_move, K, enemy_player_state.pawn, checker_zone);
    n_checkers += getHorizontalAndVerticalChecker(K, enemy_player_state.rook,
                                                  enemy_player_state.queen,
                                                  OCCUPIED, checker_zone);
    n_checkers +=
        getDiagonalChecker(K, enemy_player_state.bishop,
                           enemy_player_state.queen, OCCUPIED, checker_zone);
    n_checkers +=
        getKnightChecker(K, enemy_player_state.knight, OCCUPIED, checker_zone);
  }
  return check;
}

uint8_t generateBlackMoves(GameState &gamestate, Move *moves, bool &check) {
  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  uint64_t DZ = 0;
  uint8_t n_checkers = 0;
  uint64_t checker_zone = 0;
  check = isInCheck(gamestate.whites_turn, gamestate.black.king,
                    gamestate.white, OCCUPIED, DZ, checker_zone, n_checkers);

  uint64_t PINNED = getPinnedPieces(
      gamestate.black.king, gamestate.black.pawn, gamestate.white.queen,
      gamestate.white.bishop, gamestate.white.rook, OCCUPIED,
      gamestate.en_passant, gamestate.whites_turn);

  uint8_t n_moves = 0;
  if (!check) {
    generateKingsideCastleMove(gamestate.black.can_king_side_castle,
                               gamestate.black.king, ~OCCUPIED, DZ, moves,
                               n_moves);
    generateQueensideCastleMove(gamestate.black.can_queen_side_castle,
                                gamestate.black.king, ~OCCUPIED, DZ, moves,
                                n_moves);
  }

  if (n_checkers < 2) {
    generateBlackPawnMoves(gamestate.whites_turn, gamestate.black.pawn,
                           gamestate.black.king, gamestate.en_passant,
                           ~OCCUPIED, WHITE_PIECES, PINNED, checker_zone, moves,
                           n_moves);
    generateRookMoves(gamestate.black.rook, gamestate.black.king, BLACK_PIECES,
                      OCCUPIED, PINNED, checker_zone, moves, n_moves);
    generateBishopMoves(gamestate.black.bishop, gamestate.black.king,
                        BLACK_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                        n_moves);
    generateQueenMoves(gamestate.black.queen, gamestate.black.king,
                       BLACK_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                       n_moves);
    generateKnightMoves(gamestate.black.knight, gamestate.black.king,
                        BLACK_PIECES, PINNED, checker_zone, moves, n_moves);
  }
  generateKingMoves(gamestate.black.king, BLACK_PIECES, DZ, moves, n_moves);

  return n_moves;
}

uint8_t generateWhiteMoves(GameState &gamestate, Move *moves, bool &check) {
  uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
  uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
  uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

  uint64_t DZ = 0;
  uint8_t n_checkers = 0;
  uint64_t checker_zone = 0;
  check = isInCheck(gamestate.whites_turn, gamestate.white.king,
                    gamestate.black, OCCUPIED, DZ, checker_zone, n_checkers);

  uint64_t PINNED = getPinnedPieces(
      gamestate.white.king, gamestate.white.pawn, gamestate.black.queen,
      gamestate.black.bishop, gamestate.black.rook, OCCUPIED,
      gamestate.en_passant, gamestate.whites_turn);

  uint8_t n_moves = 0;
  if (!check) {
    generateKingsideCastleMove(gamestate.white.can_king_side_castle,
                               gamestate.white.king, ~OCCUPIED, DZ, moves,
                               n_moves);
    generateQueensideCastleMove(gamestate.white.can_queen_side_castle,
                                gamestate.white.king, ~OCCUPIED, DZ, moves,
                                n_moves);
  }

  if (n_checkers < 2) {
    generateWhitePawnMoves(gamestate.whites_turn, gamestate.white.pawn,
                           gamestate.white.king, gamestate.en_passant,
                           ~OCCUPIED, BLACK_PIECES, PINNED, checker_zone, moves,
                           n_moves);
    generateRookMoves(gamestate.white.rook, gamestate.white.king, WHITE_PIECES,
                      OCCUPIED, PINNED, checker_zone, moves, n_moves);
    generateBishopMoves(gamestate.white.bishop, gamestate.white.king,
                        WHITE_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                        n_moves);
    generateQueenMoves(gamestate.white.queen, gamestate.white.king,
                       WHITE_PIECES, OCCUPIED, PINNED, checker_zone, moves,
                       n_moves);
    generateKnightMoves(gamestate.white.knight, gamestate.white.king,
                        WHITE_PIECES, PINNED, checker_zone, moves, n_moves);
  }
  generateKingMoves(gamestate.white.king, WHITE_PIECES, DZ, moves, n_moves);

  return n_moves;
}

uint8_t generateMoves(GameState &gamestate, Move *moves, bool &check) {
  return gamestate.whites_turn ? generateWhiteMoves(gamestate, moves, check)
                               : generateBlackMoves(gamestate, moves, check);
}

uint64_t moveGetInitialPositionBitboard(Move move) {
  return (uint64_t)1 << ((move.getX1() * 8) + (move.getY1() % 8));
}

uint64_t moveGetFinalPositionBitboard(Move move) {
  return (uint64_t)1 << ((move.getX2() * 8) + (move.getY2() % 8));
}

void handleCapturedPiece(bool white_to_move, uint64_t P,
                         PlayerState &enemy_player, uint64_t E_P,
                         uint64_t initial, uint64_t final) {
  uint64_t ENEMY_PIECES = white_to_move
                              ? generatePlayerOccupiedBitboard(enemy_player)
                              : generatePlayerOccupiedBitboard(enemy_player);

  if (ENEMY_PIECES & final) {
    if (enemy_player.pawn & final) {
      enemy_player.pawn &= ~final;
      return;
    }
    if (enemy_player.knight & final) {
      enemy_player.knight &= ~final;
      return;
    }
    if (enemy_player.bishop & final) {
      enemy_player.bishop &= ~final;
      return;
    }
    if (enemy_player.queen & final) {
      enemy_player.queen &= ~final;
      return;
    }
    if (enemy_player.rook & final) {
      enemy_player.rook &= ~final;
      if (final & (white_to_move ? BLACK_ROOK_STARTING_POSITION_KINGSIDE
                                 : WHITE_ROOK_STARTING_POSITION_KINGSIDE)) {
        enemy_player.can_king_side_castle = false;
      } else if (final &
                 (white_to_move ? BLACK_ROOK_STARTING_POSITION_QUEENSIDE
                                : WHITE_ROOK_STARTING_POSITION_QUEENSIDE)) {
        enemy_player.can_queen_side_castle = false;
      }
      return;
    }
  }
  if ((E_P & final) && (P & initial)) {
    enemy_player.pawn &= (white_to_move ? ~(final >> 8) : ~(final << 8));
    return;
  }
}

void realizeMovedPiece(bool white_to_move, PlayerState &active_player,
                       uint64_t &E_P, uint64_t initial, uint64_t final,
                       SpecialMove special) {
  switch (special) {
  case NONE:
    if (active_player.queen & initial) {
      active_player.queen |= final;
      active_player.queen &= ~initial;
    } else if (active_player.bishop & initial) {
      active_player.bishop |= final;
      active_player.bishop &= ~initial;
    } else if (active_player.knight & initial) {
      active_player.knight |= final;
      active_player.knight &= ~initial;
    } else if (active_player.pawn & initial) {
      active_player.pawn |= final;
      active_player.pawn &= ~initial;
    } else if (active_player.king & initial) {
      active_player.can_king_side_castle = false;
      active_player.can_queen_side_castle = false;
      active_player.king = final;
    } else if (active_player.rook & initial) {
      if (initial & (white_to_move ? WHITE_ROOK_STARTING_POSITION_KINGSIDE
                                   : BLACK_ROOK_STARTING_POSITION_KINGSIDE)) {
        active_player.can_king_side_castle = false;
      } else if (initial &
                 (white_to_move ? WHITE_ROOK_STARTING_POSITION_QUEENSIDE
                                : BLACK_ROOK_STARTING_POSITION_QUEENSIDE)) {
        active_player.can_queen_side_castle = false;
      }
      active_player.rook |= final;
      active_player.rook &= ~initial;
    }
    E_P = 0;
    return;
  case CASTLE_KINGSIDE:
    active_player.king <<= 2;
    active_player.rook |= white_to_move
                              ? WHITE_ROOK_POST_KINGSIDE_CASTLE_POSITION
                              : BLACK_ROOK_POST_KINGSIDE_CASTLE_POSITION;
    active_player.rook &= white_to_move
                              ? ~WHITE_ROOK_STARTING_POSITION_KINGSIDE
                              : ~BLACK_ROOK_STARTING_POSITION_KINGSIDE;
    active_player.can_king_side_castle = false;
    active_player.can_queen_side_castle = false;
    E_P = 0;
    return;
  case CASTLE_QUEENSIDE:
    active_player.king >>= 2;
    active_player.rook |= white_to_move
                              ? WHITE_ROOK_POST_QUEENSIDE_CASTLE_POSITION
                              : BLACK_ROOK_POST_QUEENSIDE_CASTLE_POSITION;
    active_player.rook &= white_to_move
                              ? ~WHITE_ROOK_STARTING_POSITION_QUEENSIDE
                              : ~BLACK_ROOK_STARTING_POSITION_QUEENSIDE;
    active_player.can_king_side_castle = false;
    active_player.can_queen_side_castle = false;
    E_P = 0;
    return;
  case PROMOTION_QUEEN:
    active_player.pawn &= ~initial;
    active_player.queen |= final;
    E_P = 0;
    return;
  case PROMOTION_ROOK:
    active_player.pawn &= ~initial;
    active_player.rook |= final;
    E_P = 0;
    return;
  case PROMOTION_BISHOP:
    active_player.pawn &= ~initial;
    active_player.bishop |= final;
    E_P = 0;
    return;
  case PROMOTION_KNIGHT:
    active_player.pawn &= ~initial;
    active_player.knight |= final;
    E_P = 0;
    return;
  case EN_PASSANT:
    active_player.pawn |= final;
    active_player.pawn &= ~initial;
    E_P = 0;
    return;
  case PAWN_PUSH_2:
    active_player.pawn |= final;
    active_player.pawn &= ~initial;
    E_P = white_to_move ? final >> 8 : final << 8;
    return;
  default:
    logErrorAndExit("ERROR: Unexpected SpecialMove value!");
    return;
  }
}

void applyWhiteMove(GameState &gamestate, const Move &move, uint64_t initial,
                    uint64_t final, SpecialMove special) {
  handleCapturedPiece(gamestate.whites_turn, gamestate.white.pawn,
                      gamestate.black, gamestate.en_passant, initial, final);
  realizeMovedPiece(gamestate.whites_turn, gamestate.white,
                    gamestate.en_passant, initial, final, special);
}

void applyBlackMove(GameState &gamestate, const Move &move, uint64_t initial,
                    uint64_t final, SpecialMove special) {
  handleCapturedPiece(gamestate.whites_turn, gamestate.black.pawn,
                      gamestate.white, gamestate.en_passant, initial, final);
  realizeMovedPiece(gamestate.whites_turn, gamestate.black,
                    gamestate.en_passant, initial, final, special);
}

void applyMove(Move move, GameState &gamestate) {
  const uint64_t initial = moveGetInitialPositionBitboard(move);
  const uint64_t final = moveGetFinalPositionBitboard(move);
  const SpecialMove special = move.getSpecial();
  if (gamestate.whites_turn) {
    applyWhiteMove(gamestate, move, initial, final, special);
  } else {
    applyBlackMove(gamestate, move, initial, final, special);
  }
  gamestate.whites_turn = !gamestate.whites_turn;
}

void print_moves(bool white_to_move, Move *moves, uint8_t n_moves) {
  std::cout << (white_to_move ? "WHITE" : "BLACK") << "'S MOVE: " << std::endl;
  for (uint8_t i = 0; i < n_moves; i++) {
    std::cout << i + 1 << ": " + moves[i].toString() << std::endl;
  }
}

void perft(uint32_t &nodes, GameState &gamestate, uint8_t depth,
           uint8_t orig_depth, bool total) {
  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(gamestate, moves, check);

  if (depth == 1) {
    nodes += n_moves;
  }

  if (depth > 1) {

    for (uint8_t i = 0; i < n_moves; i++) {
      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
      applyMove(moves[i], gamestate_temp);

      perft(nodes, gamestate_temp, uint8_t(depth - 1), orig_depth, total);

      // TODO: make part of a 'verbose' flag.
      if (depth == orig_depth && false) {
        if (total) {
          std::cout << round(((i * 100 / n_moves)))
                    << "% complete... -> d1:" << moves[i].toString()
                    << "--------------------------------------------------"
                    << std::endl;

        } else { // node based
          std::cout << i << ":" << moves[i].toString() << " " << nodes
                    << std::endl;
          nodes = 0;
        }
      }
    }
  }
}

int16_t eval(const GameState gamestate) {
  // material
  int16_t counter = 0;
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
  //       printBoard(gamestate);
  //       exit(1);
  //    }

  return counter;
}

AI_return negamax(GameState gamestate, uint8_t depth, int8_t color = 1,
                  int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX) {
  AI_return node_max;
  node_max.value = INT16_MIN;
  node_max.nodes_searched++;

  // Terminal Node.
  if (depth == 0) {
    Move leaf_move;
    AI_return leaf = {leaf_move, (int16_t)(eval(gamestate) * color), 1};
    return leaf;
  }

  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(gamestate, moves, check);

  // Terminal node, Checkmate/Stalemate.
  if (n_moves == 0) {
    AI_return leaf = {Move(), (int16_t)(check ? INT16_MAX * -color : 0), 1};
    return leaf;
  }

  for (uint8_t i = 0; i < n_moves; i++) {
    GameState gamestate_temp;
    memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
    applyMove(moves[i], gamestate_temp);
    AI_return node_temp =
        negamax(gamestate_temp, depth - 1, -color, -beta, -alpha);
    // AI_return node_temp =
    //     negamax(move_gamestate_scores[i].gamestate, depth - 1, -color, -beta,
    //     -alpha);
    node_temp.value *= -color;
    node_max.nodes_searched += node_temp.nodes_searched;

    if (node_temp.value > node_max.value) {
      node_max.value = node_temp.value;
      node_max.move = moves[i];
      // node_max.move = move_gamestate_scores[i].move;
    }

    alpha = std::max(alpha, node_max.value);
    if (false or alpha >= beta) {
      break;
    }
  }
  return node_max;
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

  uint8_t n_p = 0, n_r = 0, n_n = 0, n_b = 0, n_q = 0, n_k = 0, n_P = 0,
          n_R = 0, n_N = 0, n_B = 0, n_Q = 0, n_K = 0;

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
    switch (field) {
    case 1:
      if (fen[i] == 'w') {
        gamestate.whites_turn = true;
        field++;
      } else if (fen[i] == 'b') {
        gamestate.whites_turn = false;
        field++;
      }
      continue;
    case 2:
      if (((gamestate.white.can_king_side_castle ||
            gamestate.white.can_queen_side_castle ||
            gamestate.black.can_king_side_castle ||
            gamestate.black.can_queen_side_castle) &&
           fen[i] == ' ') ||
          fen[i] == '-') {
        field++;
      } else if (fen[i] == 'K') {
        gamestate.white.can_king_side_castle = true;
      } else if (fen[i] == 'Q') {
        gamestate.white.can_queen_side_castle = true;
      } else if (fen[i] == 'k') {
        gamestate.black.can_king_side_castle = true;
      } else if (fen[i] == 'q') {
        gamestate.black.can_queen_side_castle = true;
      }
      continue;
    case 3:
      if (fen[i] >= 'a' && fen[i] <= 'h') {
        uint8_t col = fen[i] - 'a';
        uint8_t row = fen[i + 1] - '0' - 1;
        gamestate.en_passant = 1ull << (row * 8 + col);
      }
      continue;
    default:
      logErrorAndExit("ERROR: Unexpected field value.");
      break;
    }
  }
}

void generate_board(std::string name, uint8_t diff) {
  std::cout << "GAME START" << std::endl;

  std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
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
    depth = 3;
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
      depth = 8;
      AI_choice = negamax(gamestate, depth);
      std::cout << AI_choice.nodes_searched + 0 << std::endl;

      auto end = std::chrono::high_resolution_clock::now();

      std::cout << "Move chosen: " << AI_choice.move.toString() << std::endl;
      std::cout << AI_choice.value << std::endl;

      applyMove(AI_choice.move, gamestate);

      std::cout << "depth: " << depth + 0 << ". time elapsed: "
                << (double)(end - start).count() / 1000000000
                << " s. nodes searched: " << AI_choice.nodes_searched << "."
                << std::endl;
      std::cout << "NPS: "
                << AI_choice.nodes_searched /
                       ((double)(end - start).count() / 1000000000)
                << std::endl;
      std::cout << " " << std::endl;
    } else {
      printBoard(gamestate);
      std::cout << "BLACK'S MOVE: " << std::endl;

      // todo: create a player class for their choosing mechanism
      Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];

      // TODO: uncomment this and fix
      bool check = false;
      uint8_t n_moves = generateMoves(gamestate, moves, check);

      std::cout << "Please select your move: " << std::endl;
      print_moves(gamestate.whites_turn, moves, n_moves);

      int user_choice;
      std::cin >> user_choice;

      applyMove(moves[user_choice - 1], gamestate);

      std::cout << "Move chosen: " << moves[user_choice - 1].toString()
                << std::endl;
      std::cout << " " << std::endl;
    }
  }
}
