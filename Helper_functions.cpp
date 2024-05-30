//
// Created by David Doellstedt on 5/31/20.
//

#include "Helper_functions.h"
#include <iostream>
#include <stdint.h>

// todo: potential to make more efficient
// function that reverses the bits
uint64_t rev(uint64_t n) {
  uint64_t r = 0;
  for (int i = 0; i < 64; i++) {
    r = r << 1 | (n & 1);
    n >>= 1;
  }
  return r;
}
