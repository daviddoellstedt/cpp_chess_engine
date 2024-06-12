#pragma once

#include "constants.h"
#include "helper_functions.h"
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
  uint64_t getInitialBitboard(void) { return 1ULL << (getX1() * 8 + getY1()); }
  uint64_t getFinalBitboard(void) { return 1ULL << (getX2() * 8 + getY2()); }
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
  Move(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SpecialMove special) {
    setX1(x1);
    setY1(y1);
    setX2(x2);
    setY2(y2);
    setSpecial(special);
  }
  Move(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    setX1(x1);
    setY1(y1);
    setX2(x2);
    setY2(y2);
  }
};
