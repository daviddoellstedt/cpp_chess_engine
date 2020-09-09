//
// Created by David Doellstedt on 5/31/20.
//

#ifndef DDS_CHESS_ENGINE_HELPER_FUNCTIONS_H
#define DDS_CHESS_ENGINE_HELPER_FUNCTIONS_H

#include <vector>

using namespace std;

void ind_bbs(unsigned long long bb, vector<unsigned long long>& bbs);
void viz_bb(unsigned long long bb);
void generate_bit();
unsigned long long rev(unsigned long long n);
string ind_x(int bit, int adder);
string ind_y(int bit, int adder);
string b_2_ind(int bit, int x_adder, int y_adder);


#endif //DDS_CHESS_ENGINE_HELPER_FUNCTIONS_H
