#ifndef DDS_CHESS_ENGINE_UNIT_TESTS_H
#define DDS_CHESS_ENGINE_UNIT_TESTS_H

#include <stdint.h>
#include <string>


uint32_t legalMoveGeneratorTest(std::string fen, uint8_t depth);

void testAllPerft(void);

#endif //DDS_CHESS_ENGINE_UNIT_TESTS_H