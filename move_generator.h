#pragma once

#include "board.h"
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
  std::string specialToString(void);

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
  std::string toString(void);
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

/** Initializes the magic bitboard tables.
 */
void initializeMagicBitboardTables(void);

// TODO add documentation.
uint8_t generateMoves(GameState &game_state, Move *moves, bool &check);
