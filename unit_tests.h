#pragma once

#include <stdint.h>
#include <string>


uint32_t legalMoveGeneratorTest(std::string fen, uint8_t depth);

void testAllPerft(void);
