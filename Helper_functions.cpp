//
// Created by David Doellstedt on 5/31/20.
//

#include "Helper_functions.h"
#include <stdint.h>

uint64_t rev(uint64_t n) {
  n = (n >> 32) | (n << 32);
  n = ((n & 0xFFFF0000FFFF0000) >> 16) | ((n & 0x0000FFFF0000FFFF) << 16);
  n = ((n & 0xFF00FF00FF00FF00) >> 8) | ((n & 0x00FF00FF00FF00FF) << 8);
  n = ((n & 0xF0F0F0F0F0F0F0F0) >> 4) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
  n = ((n & 0xCCCCCCCCCCCCCCCC) >> 2) | ((n & 0x3333333333333333) << 2);
  n = ((n & 0xAAAAAAAAAAAAAAAA) >> 1) | ((n & 0x5555555555555555) << 1);
  return n;
}
