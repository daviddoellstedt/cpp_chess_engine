#include "helper_functions.h"
#include <iostream>
#include <stdint.h>
#include <string>

uint64_t reverse(uint64_t x) {
  x = (x >> 32) | (x << 32);
  x = ((x & 0xFFFF0000FFFF0000) >> 16) | ((x & 0x0000FFFF0000FFFF) << 16);
  x = ((x & 0xFF00FF00FF00FF00) >> 8) | ((x & 0x00FF00FF00FF00FF) << 8);
  x = ((x & 0xF0F0F0F0F0F0F0F0) >> 4) | ((x & 0x0F0F0F0F0F0F0F0F) << 4);
  x = ((x & 0xCCCCCCCCCCCCCCCC) >> 2) | ((x & 0x3333333333333333) << 2);
  x = ((x & 0xAAAAAAAAAAAAAAAA) >> 1) | ((x & 0x5555555555555555) << 1);
  return x;
}

uint8_t getSetBit(uint64_t x) { return 63 - __builtin_clzll(x); }

void logErrorAndExit(std::string error_message) {
  std::cout << error_message << std::endl;
  exit(1);
}

uint64_t getLowestSetBitValue(uint64_t x) { return x & ~(x - 1); }

void clearLowestSetBit(uint64_t &x) { x &= (x - 1); }

uint8_t countSetBits(uint64_t x) {
  uint8_t count = 0;
  while (x) {
    clearLowestSetBit(x);
    count++;
  }
  return count;
}
