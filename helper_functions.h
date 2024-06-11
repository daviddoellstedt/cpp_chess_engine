#pragma once

#include <stdint.h>
#include <string>

/** Reverses the bits of a 64 bit integer.
 *
 * @param x: 64 bit integer.
 * @return Integer with reversed bits.
 */
uint64_t reverse(uint64_t x);

/** Returns the position of the set bit. Behavior is defined for numbers that
 * have exactly 1 bit set.
 *
 * @param x: Number.
 * @return The position of the set bit.
 */
uint8_t getSetBit(uint64_t x);

/** Prints an error message and exits the program.
 *
 * @param error_message: Error message.
 */
void logErrorAndExit(std::string error_message);

/** Returns the bitboard of just the lowest set bit. 1 << lowest_bit.
 *
 * @param x: Number.
 * @return The value of the lowest set bit.
 */
uint64_t getLowestSetBitValue(uint64_t x);

/** Clears the lowest set bit.
 *
 * @param x: Number.
 */
void clearLowestSetBit(uint64_t &x);

/** Counts the total set bits.
 *
 * @param x: Number.
 * @return The count of set bits.
 */
uint8_t countSetBits(uint64_t x);

/** Generates a random 64 bit integer..
 *
 * @return Random 64 bit integer.
 */
uint64_t generateRandom64(void);

/** Prints a bitboard as a 8x8 grid.
 *
 * @param bitboard: 64 bit bitboard.
 */
void printBitboard(uint64_t bitboard);
