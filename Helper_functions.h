//
// Created by David Doellstedt on 5/31/20.
//

#ifndef DDS_CHESS_ENGINE_HELPER_FUNCTIONS_H
#define DDS_CHESS_ENGINE_HELPER_FUNCTIONS_H

#include <vector>



void ind_bbs(unsigned long long bb, std::vector<unsigned long long>& bbs);
void viz_bb(unsigned long long bb);
void generate_bit();
unsigned long long rev(unsigned long long n);

#endif //DDS_CHESS_ENGINE_HELPER_FUNCTIONS_H
