#include "player.h"

bool Player::assign_color() {
  srand(time(nullptr));
  return rand() % 2;
}
