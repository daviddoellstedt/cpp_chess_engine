#pragma once

#include "board.h"

/** Returns a score value of the board position. Always evaluated from white's
 *  perspective. White score = -Black score.
 *
 * @param game_state: Game state.
 * @return Score value.
 */
int16_t evaluatePosition(const GameState game_state);

/** Initializes the values of the black positional tables. Just a mirror of
 *  the white positional tables.
 */
void initializePositionTables(void);
