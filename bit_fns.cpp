//
// Created by David Doellstedt on 5/12/20.
//
#include <iostream>
#include <string>
#include <bitset>
#include <cmath>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <chrono>  // for high_resolution_clock
#include "Players.h"
#include "Helper_functions.h"

using namespace std;

struct AI_return {
    string move;
    double value;
};

unsigned long long bit_2_long[64] = {1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u, 256u, 512u, 1024u, 2048u, 4096u, 8192u, \
                                    16384u, 32768u, 65536u, 131072u, 262144u, 524288u, 1048576u, 2097152u, 4194304u, \
                                    8388608u, 16777216u, 33554432u, 67108864u, 134217728u, 268435456u, 536870912u, \
                                    1073741824u, 2147483648u, 4294967296u, 8589934592u, 17179869184u, 34359738368u, \
                                    68719476736u, 137438953472u, 274877906944u, 549755813888u, 1099511627776u, \
                                    2199023255552u, 4398046511104u, 8796093022208u, 17592186044416u, 35184372088832u, \
                                    70368744177664u, 140737488355328u, 281474976710656u, 562949953421312u, 1125899906842624u, \
                                    2251799813685248u, 4503599627370496u, 9007199254740992u, 18014398509481984u, \
                                    36028797018963968u, 72057594037927936u, 144115188075855872u, 288230376151711744u, \
                                    576460752303423488u, 1152921504606846976u, 2305843009213693952u, 4611686018427387904u, \
                                    9223372036854775808u};

int bit_2_x[64] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, \
                    4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7};
int bit_2_y[64] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, \
                    1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};

unsigned long long rank_masks [8] = {255u, 65280u, 16711680u, 4278190080u, 1095216660480u, 280375465082880u, \
                                    71776119061217280u, 18374686479671623680u};  //bottom to top (rank 1 to rank 8)
unsigned long long file_masks [8] = {72340172838076673u, 144680345676153346u, 289360691352306692u,  \
                                    578721382704613384u, 1157442765409226768u, 2314885530818453536u, \
                                    4629771061636907072u, 9259542123273814144u};  //left to right (file A-G)
unsigned long long diag_dn_r_masks [15] = {1u, 258u, 66052u, 16909320u, 4328785936u, 1108169199648u, 283691315109952u, \
                                    72624976668147840u, 145249953336295424u, 290499906672525312u, 580999813328273408u, \
                                    1161999622361579520u, 2323998145211531264u, 4647714815446351872u, \
                                    9223372036854775808u}; //diagnol down and to the right (start at A1 end at H8)

unsigned long long diag_up_r_masks [15] = {72057594037927936u, 144396663052566528u, 288794425616760832u, 577588855528488960u, \
                                    1155177711073755136u, 2310355422147575808u, 4620710844295151872u, \
                                    9241421688590303745u, 36099303471055874u, 141012904183812u, 550831656968u, \
                                    2151686160u, 8405024u, 32832u, 128u}; //diagnol up and to the right (start at A8 end at H1)

int loc_masks[64][4] = {{0, 0, 0, 7}, {0, 1, 1, 8}, {0, 2, 2, 9}, {0, 3, 3, 10}, {0, 4, 4, 11}, {0, 5, 5, 12}, {0, 6, 6, 13}, {0, 7, 7, 14},
                        {1, 0, 1, 6}, {1, 1, 2, 7}, {1, 2, 3, 8}, {1, 3, 4, 9},  {1, 4, 5, 10}, {1, 5, 6, 11}, {1, 6, 7, 12}, {1, 7, 8, 13},
                        {2, 0, 2, 5}, {2, 1, 3, 6}, {2, 2, 4, 7}, {2, 3, 5, 8},  {2, 4, 6, 9},  {2, 5, 7, 10}, {2, 6, 8, 11}, {2, 7, 9, 12},
                        {3, 0, 3, 4}, {3, 1, 4, 5}, {3, 2, 5, 6}, {3, 3, 6, 7},  {3, 4, 7, 8},  {3, 5, 8, 9},  {3, 6, 9, 10}, {3, 7, 10, 11},
                        {4, 0, 4, 3}, {4, 1, 5, 4}, {4, 2, 6, 5}, {4, 3, 7, 6},  {4, 4, 8, 7},  {4, 5, 9, 8},  {4, 6, 10, 9}, {4, 7, 11, 10},
                        {5, 0, 5, 2}, {5, 1, 6, 3}, {5, 2, 7, 4}, {5, 3, 8, 5},  {5, 4, 9, 6},  {5, 5, 10, 7}, {5, 6, 11, 8}, {5, 7, 12, 9},
                        {6, 0, 6, 1}, {6, 1, 7, 2}, {6, 2, 8, 3}, {6, 3, 9, 4},  {6, 4, 10, 5}, {6, 5, 11, 6}, {6, 6, 12, 7}, {6, 7, 13, 8},
                        {7, 0, 7, 0}, {7, 1, 8, 1}, {7, 2, 9, 2}, {7, 3, 10, 3}, {7, 4, 11, 4}, {7, 5, 12, 5}, {7, 6, 13, 6}, {7, 7, 14, 7}}; //mask index table, rank, file, diag down and right, diag up and right

unsigned long long FILE_A = 72340172838076673u, FILE_H = 9259542123273814144u, FILE_AB = 217020518514230019u, FILE_GH = 13889313184910721216u;
unsigned long long KNIGHT_MOVES = 345879119952u, KING_MOVES = 14721248u;
//unsigned int RANK_1 = 255u;
unsigned long long  RANK_3 = 16711680u, RANK_4 = 4278190080u, RANK_5 = 1095216660480u, RANK_6 = 280375465082880u, RANK_8 = 18374686479671623680u,  FILLED = 18446744073709551615u;
unsigned long long RANK_1 = 255u;
/** Printing the board to the command line.
*
* arguments: the 12 bitboards for the all the pieces
*/
void print_board(unsigned long long BR, unsigned long long BN, unsigned long long BB, \
                unsigned long long BQ, unsigned long long BK, unsigned long long BP, unsigned long long WR, \
                unsigned long long WN, unsigned long long WB, unsigned long long WQ, unsigned long long WK, \
                unsigned long long WP){
    unsigned long long ALL = (BR | BN | BB | BQ | BK | BP | WR | WN | WB | WQ | WK | WP);
    char grid[8][8] = {
            //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

    vector<unsigned long long> bbs; //individual bitboards for each piece
    ind_bbs(ALL, bbs);    //generate list of indivudal bitboards
    unsigned long long test_bit;

    for (auto bb: bbs) {

        int i = (int)log2(bb);
        test_bit = bb;
        if ((test_bit & WP) != 0){grid[(i - i % 8) / 8][i%8] = 'P';  }
        else if ((test_bit & WR) != 0){grid[(i - i % 8) / 8][i%8] = 'R'; }
        else if ((test_bit & WN) != 0){grid[(i - i % 8) / 8][i%8] = 'N';  }
        else if ((test_bit & WB) != 0){grid[(i - i % 8) / 8][i%8] = 'B';  }
        else if ((test_bit & WQ) != 0){grid[(i - i % 8) / 8][i%8] = 'Q';  }
        else if ((test_bit & WK) != 0){grid[(i - i % 8) / 8][i%8] = 'K'; }
        else if ((test_bit & BP) != 0){grid[(i - i % 8) / 8][i%8] = 'p';  }
        else if ((test_bit & BR) != 0){grid[(i - i % 8) / 8][i%8] = 'r';  }
        else if ((test_bit & BN) != 0){grid[(i - i % 8) / 8][i%8] = 'n';  }
        else if ((test_bit & BB) != 0){grid[(i - i % 8) / 8][i%8] = 'b';  }
        else if ((test_bit & BQ) != 0){grid[(i - i % 8) / 8][i%8] = 'q';  }
        else if ((test_bit & BK) != 0){grid[(i - i % 8) / 8][i%8] = 'k';  }
    }
    cout << "    |-----|-----|-----|-----|-----|-----|-----|-----|" << endl;
    cout<<"    |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |"<<endl;
    for(int i = 7; i >= 0; i--) {
        string str0(1, grid[i][0]);  string str1(1, grid[i][1]);  string str2(1, grid[i][2]);  string str3(1, grid[i][3]);  string str4(1, grid[i][4]);  string str5(1, grid[i][5]); string str6(1, grid[i][6]); string str7(1, grid[i][7]);

        cout << "|---|-----|-----|-----|-----|-----|-----|-----|-----|" << endl;
        cout << "| " + to_string(i) + " |  " + str0 + "  |  " + str1 + "  |  " + str2 + "  |  " + str3 + "  |  " + \
        str4 + "  |  " + str5 + "  |  " +
                str6 + "  |  " + str7 + "  |" << endl;

    }
    cout << "|---|-----|-----|-----|-----|-----|-----|-----|-----|" << endl;

}

/** Helper function used to convert a grid of pieces into the 12 unique bitboards. This should only be executed once,
* at the start of the game.
*
* @param g: the char grid of pieces
* @params 12 unique bitboards (by reference, so they can be modified)
*/
//todo: get rid of this eventually. Should be populating the bitboards directly from the FEN. Need to modify read FEN function also
void grid_to_bbs(char g[8][8], unsigned long long& BR, unsigned long long& BN, unsigned long long& BB, \
                unsigned long long& BQ, unsigned long long& BK, unsigned long long& BP, unsigned long long& WR, \
                unsigned long long& WN, unsigned long long& WB, unsigned long long& WQ, unsigned long long& WK, \
                unsigned long long& WP){
    int counter = -1;

    for(int i = 7; i >= 0; i--) {
        for(int j = 0; j < 8; j++) {
            counter++;

            if(g[i][j] == 'r') {BR += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'n') {BN += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'b') {BB += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'q') {BQ += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'k') {BK += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'p') {BP += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'R') {WR += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'N') {WN += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'B') {WB += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'Q') {WQ += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'K') {WK += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == 'P') {WP += (unsigned long long)pow(2, counter);}
            else if (g[i][j] == ' ') {}  //do nothing
        }
    }
}

/** Function that can generate the possible moves a slider piece can make in the horizontal direction
*
* @param piece: bitboard representing a horizontal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return horiz_moves: bitboard of horizontal moves
*/
unsigned long long h_moves(unsigned long long piece, int sl_bit, unsigned long long OCCUPIED){
    unsigned long long horiz_moves = (((OCCUPIED) - 2 * piece) ^ rev((rev(OCCUPIED) - 2 * rev(piece)))) & rank_masks[loc_masks[sl_bit][0]];
    return horiz_moves;
}

/** Function that can generate the possible moves a slider piece can make in the vertical direction
*
* @param piece: bitboard representing a vertical sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return vert_moves: bitboard of vertical moves
*/
unsigned long long v_moves(unsigned long long piece, int sl_bit, unsigned long long OCCUPIED){
    unsigned long long vert_moves = (((OCCUPIED & file_masks[loc_masks[sl_bit][1]]) - 2 * piece) ^ rev((rev(OCCUPIED & file_masks[loc_masks[sl_bit][1]]) - 2 * rev(piece)))) & file_masks[loc_masks[sl_bit][1]];
    return vert_moves;
}

/** Function that compiles the horizontal and vertical moves bitboards and handles a case where we check for unsafe moves for the king.
*
* @param piece: bitboard representing a horizontal/vertical sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param unsafe_calc: flag used to see if we need to remove the enemy king from the occupied spaces (see inline comment for more details)
* @param K: bitboard representing king location. (see inline comment in the function for more details)
* @return bitboard of horizontal and vertical moves
*/
unsigned long long h_v_moves(unsigned long long piece, int sl_bit, unsigned long long OCCUPIED, bool unsafe_calc, unsigned long long K){

    // this line is used in the case where we need to generate zones for the king that are unsafe. If the king is in the
    // attack zone of a horizontal/vertical slider, we want to remove the king from the calculation. Because a move of the king, that
    // still lies in the path of attack (further away from the slider) is still an "unsafe" move.
    if (unsafe_calc){OCCUPIED &= ~K;}

    //todo: room for optimization? Less new variables?
    return h_moves(piece, sl_bit, OCCUPIED) | v_moves(piece, sl_bit, OCCUPIED);
}

/** Function that can generate the possible moves a slider piece can make in the (down, right) and (up, left) diagonol
*
* @param piece: bitboard representing a diagonal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return ddr_moves: bitboard of (down, right) and (up, left) moves
*/
unsigned long long ddr_moves(unsigned long long piece, int sl_bit, unsigned long long OCCUPIED){
    unsigned long long ddr_moves = (((OCCUPIED & diag_dn_r_masks[loc_masks[sl_bit][2]]) - 2 * piece) ^ rev((rev(OCCUPIED & diag_dn_r_masks[loc_masks[sl_bit][2]]) - 2 * rev(piece)))) & diag_dn_r_masks[loc_masks[sl_bit][2]];
    return ddr_moves;
}

/** Function that can generate the possible moves a slider piece can make in the (up, right) and (down, left) diagonol
*
* @param piece: bitboard representing a diagonal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return dur_moves: bitboard of (up, right) and (down, left) moves
*/
unsigned long long dur_moves(unsigned long long piece, int sl_bit, unsigned long long OCCUPIED){
    unsigned long long dur_moves = (((OCCUPIED & diag_up_r_masks[loc_masks[sl_bit][3]]) - 2 * piece) ^ rev((rev(OCCUPIED & diag_up_r_masks[loc_masks[sl_bit][3]]) - 2 * rev(piece)))) & diag_up_r_masks[loc_masks[sl_bit][3]];
    return dur_moves;
}

/** Function that compiles the diagonal moves bitboards and handles a case where we check for unsafe moves for the king.
*
* @param piece: bitboard representing a diagonal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param unsafe_calc: flag used to see if we need to remove the enemy king from the occupied spaces (see inline comment for more details)
* @param K: bitboard representing king location. (see inline comment in the function for more details)
* @return bitboard of all diagonal moves
*/
unsigned long long diag_moves(unsigned long long piece, int sl_bit, unsigned long long OCCUPIED, bool unsafe_calc, unsigned long long K){

    // this line is used in the case where we need to generate zones for the king that are unsafe. If the king is in the
    // attack zone of a diagonal slider, we want to remove the king from the calculation. Because a move of the king, that
    // still lies in the path of attack (further away from the slider) is still an "unsafe" move.
    if (unsafe_calc){OCCUPIED &= ~K;}
    //todo: room for optimization? Less new variables?
    return ddr_moves(piece, sl_bit, OCCUPIED) | dur_moves(piece, sl_bit, OCCUPIED);
}


/** Function that returns a bitboard mask of the straight line between two pieces. Inputs are guaranteed to be colinear
* at a diagonal or orthogonal perspective. More or less a lookup table.
*
* @param p1: first piece
* @param p2: second piece
* @return bitboard mask of rank/file/diagonal connection between the two pieces
*/
unsigned long long get_mask(unsigned long long p1, unsigned long long p2){
    int k_bit = (int)log2(p2), p_bit = (int)log2(p1);

    int k_x = ((k_bit - k_bit % 8) / 8), p_x = ((p_bit - p_bit % 8) / 8);
    int k_y = (k_bit % 8), p_y = (p_bit % 8);

    if (k_x - p_x == 0){ //return horizontal mask
        return rank_masks[loc_masks[k_bit][0]];
    }
    else if (k_y - p_y == 0){  //return vertical mask
        return file_masks[loc_masks[k_bit][1]];
    }
    else if (((p_x - k_x) > 0 and (p_y - k_y) < 0) or ((p_x - k_x) < 0 and (p_y - k_y) > 0)){  //return ddr_mask
        return diag_dn_r_masks[loc_masks[k_bit][2]];
    }
    else if (((p_x - k_x) < 0 and (p_y - k_y) < 0) or ((p_x - k_x) > 0 and (p_y - k_y) > 0)) {  //return dur_mask
        return diag_up_r_masks[loc_masks[k_bit][3]];
    }
    else{
        cout<<"ERROR in get_pinned_mask"<<endl;
        return 0u;
    }
}

/** Function that returns a bitboard of pieces that are pinned. Pieces that if you were to move them, would place your
* own King in check (an illegal move).
*
* @params Piece bitboards (note that the E preceding a Piece letter denotes the enemies piece, ex: EB = enemy bishop)
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return PINNED: bitboard of all pinned pieces for a color
*/
unsigned long long get_pinned_pieces(unsigned long long K, unsigned long long P, unsigned long long EQ, \
    unsigned long long EB, unsigned long long ER, unsigned long long OCCUPIED, unsigned long long E_P, \
    unsigned long long& E_P_special){

    unsigned long long PINNED = 0u, NEW_PIN, K_slider, H_moves;
    vector<unsigned long long> h_v_slider_bbs, diag_slider_bbs; //individual bitboards for each piece
    ind_bbs(EQ | ER, h_v_slider_bbs);    //generate list of indivudal Queens and rooks
    ind_bbs(EQ | EB, diag_slider_bbs);    //generate list of indivudal Bishops and Queens

    //for the 4 directions (4 iterations)
    // 1. generate sliding moves from the kings position (include "capture" of the kings pieces
    // 2. generate sliding moves from opponents pieces
    // 3. take the AND of 1 and 2. This is a location of a pinned piece, if any. OR this result with pinned piece bb
    int k_bit = (int)log2(K);

    for (int i = 0; i <4; i++){
        if(i == 0){ // horizontal check
            K_slider = h_moves(K, k_bit, OCCUPIED);

            for (auto h: h_v_slider_bbs) {

                H_moves = h_moves(h, (int)log2(h), OCCUPIED);
                NEW_PIN = K_slider & H_moves;
                //todo: fix bug where the attacker moves are not detecting apinned pawn in the case of an en passant opportunity. Need to add a special check for E_P

                //todo add if logic for player turn

                if((((E_P << 8) & K_slider) != 0 and ((E_P << 7) & ~FILE_H & P & H_moves)) or (((E_P << 8) & H_moves) != 0 and ((E_P << 7) & ~FILE_H & P & K_slider))) {
                    NEW_PIN |= (E_P << 7);
                   // E_P_special = file_masks[loc_masks[(int)log2((E_P << 7))][1]];
                 //  E_P_special = get_mask((E_P << 7), E_P) | get_mask((E_P << 9), E_P);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P << 7))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P << 7))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P << 7))][3]]) & ~get_mask((E_P << 7), E_P);

                    //  viz_bb(E_P_special);

                }

                if((((E_P << 8) & K_slider) != 0 and ((E_P << 9) & ~FILE_A & P & H_moves)) or (((E_P << 8) & H_moves) != 0 and ((E_P << 9) & ~FILE_A & P & K_slider))){
                    NEW_PIN |= (E_P << 9);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P << 9))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P << 9))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P << 9))][3]]) & ~get_mask((E_P << 9), E_P);
                }


                //for white
                if((((E_P >> 8) & K_slider) != 0 and ((E_P >> 9) & ~FILE_H & P & H_moves)) or (((E_P >> 8) & H_moves) != 0 and ((E_P >> 9) & ~FILE_H & P & K_slider))) {
                    NEW_PIN |= (E_P >> 9);
                    //E_P_special = file_masks[loc_masks[(int)log2((E_P >> 9))][1]];
                   // E_P_special = get_mask((E_P << 7), E_P) | get_mask((E_P << 9), E_P);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P >> 9))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P >> 9))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P >> 9))][3]]) & ~get_mask((E_P >> 9), E_P);


                }

                if((((E_P >> 8) & K_slider) != 0 and ((E_P >> 7) & ~FILE_A & P & H_moves)) or (((E_P >> 8) & H_moves) != 0 and ((E_P >> 7) & ~FILE_A & P & K_slider))){
                    NEW_PIN |= (E_P >> 7);
                   // E_P_special = file_masks[loc_masks[(int)log2((E_P >> 7))][1]];
                  //  E_P_special = get_mask((E_P << 7), E_P) | get_mask((E_P << 9), E_P);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P >> 7))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P >> 7))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P >> 7))][3]]) & ~get_mask((E_P >> 7), E_P);


                }

                PINNED |= NEW_PIN;
            }
        }
        else if (i == 1){ //vertical check
            K_slider = v_moves(K, k_bit, OCCUPIED);
            for (auto v: h_v_slider_bbs) {
                NEW_PIN = K_slider & v_moves(v, (int)log2(v), OCCUPIED);
                PINNED |= NEW_PIN;
            }
        }
        else if (i == 2) { //ddr check
            K_slider = ddr_moves(K, k_bit, OCCUPIED);
            for (auto ddr: diag_slider_bbs) {
                NEW_PIN = K_slider & ddr_moves(ddr, (int)log2(ddr), OCCUPIED);
                PINNED |= NEW_PIN;
            }
        }
        else{   //dur check
            K_slider = dur_moves(K, k_bit, OCCUPIED);
            for (auto dur: diag_slider_bbs) {
                NEW_PIN = K_slider & dur_moves(dur, (int)log2(dur), OCCUPIED);
                PINNED |= NEW_PIN;
            }
        }
    }
    return PINNED;
}

/** Function that returns a bitboard of possible rook moves
*
* @params Piece bitboards
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param PINNED: bitboard of all pinned pieces for a color
* @param checker_zone: bitboard of
*/

//HERE 9/8/20

void get_rook_moves(unsigned long long R, unsigned long long K, unsigned long long PIECES, \
    unsigned long long OCCUPIED, unsigned long long PINNED, unsigned long long checker_zone, vector<string>& wb_moves){

    if(R != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        vector<unsigned long long> r_bbs; //individual bitboards for each piece
        ind_bbs(R, r_bbs);    //generate list of indivudal bitboards
        unsigned long long mask;

        for (auto bb: r_bbs) {

            // get moves
            int sl_bit = (int)log2(bb);

            mask = FILLED;
            if ((bb & PINNED) > 0u){mask = get_mask(bb, K);}
            //todo: make this a lookup table for improved performance
            bitset<64>moves(h_v_moves(bb, sl_bit, OCCUPIED, false, 0u) & ~PIECES & mask & checker_zone);
            // loop through moves and append to list, if there are any
            if (moves != 0) {
            //    viz_bb(h_v_moves(bb, sl_bit, OCCUPIED) & ~WHITE_PIECES);
             //   cout<<"----"<<endl;
                string ind_i = ind_x(sl_bit, 0) + ind_y(sl_bit, 0) + ">";
                for (int i = 0; i < 64; i++) {
                    if (moves[i] == 1) {
                        string ind_f = ind_x(i, 0) + ind_y(i, 0);
                        wb_moves.emplace_back(ind_i + ind_f);
                    }
                }
            }
        }
    }
}
void get_bishop_moves(unsigned long long B, unsigned long long K, unsigned long long PIECES, unsigned long long OCCUPIED, unsigned long long PINNED, unsigned long long checker_zone, vector<string>& wb_moves){
    if(B != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        vector<unsigned long long> bishop_bbs; //individual bitboards for each piece
        ind_bbs(B, bishop_bbs);    //generate list of indivudal bitboards
        unsigned long long mask;

        for (auto bb: bishop_bbs) {
            // get moves
            int sl_bit = (int)log2(bb);

            mask = FILLED;
            if ((bb & PINNED) > 0u){mask = get_mask(bb, K);}
            //todo: make this a lookup table for improved performance
            bitset<64>moves(diag_moves(bb, sl_bit, OCCUPIED,false, 0u) & ~PIECES & mask & checker_zone);
            // loop through moves and append to list, if there are any
            if (moves != 0) {
                string ind_i = ind_x(sl_bit, 0) + ind_y(sl_bit, 0) + ">";
                //todo: can maybe optimize by not searching the entire range
                for (int i = 0; i < 64; i++) {
                    if (moves[i] == 1) {
                        string ind_f = ind_x(i, 0) + ind_y(i, 0);
                        wb_moves.emplace_back(ind_i + ind_f);
                    }
                }
            }
        }
    }
}
void get_queen_moves(unsigned long long Q, unsigned long long K, unsigned long long PIECES, unsigned long long OCCUPIED, unsigned long long PINNED, unsigned long long checker_zone, vector<string>& wb_moves) {
    if (Q != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        vector<unsigned long long> q_bbs; //individual bitboards for each piece
        ind_bbs(Q, q_bbs);    //generate list of indivudal bitboards
        unsigned long long mask;

        for (auto bb: q_bbs) {
            // get moves
            int sl_bit = (int) log2(bb);

            mask = FILLED;
            if ((bb & PINNED) > 0u){mask = get_mask(bb, K);}
            //todo: make this a lookup table for improved performance
            bitset<64> moves((h_v_moves(bb, sl_bit, OCCUPIED, false, 0u) | diag_moves(bb, sl_bit, OCCUPIED, false, 0u)) & ~PIECES & mask & checker_zone);
            // loop through moves and append to list, if there are any
            if (moves != 0) {
                string ind_i = ind_x(sl_bit, 0) + ind_y(sl_bit, 0) + ">";
                for (int i = 0; i < 64; i++) {
                    if (moves[i] == 1) {
                        string ind_f = ind_x(i, 0) + ind_y(i, 0);
                        wb_moves.emplace_back(ind_i + ind_f);
                    }
                }
            }
        }
    }
}
void get_knight_moves(unsigned long long N, unsigned long long K, unsigned long long PIECES, unsigned long long PINNED, unsigned long long checker_zone, vector<string>& wb_moves) {
    if (N != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        vector<unsigned long long> r_bbs; //individual bitboards for each piece
        ind_bbs(N, r_bbs);    //generate list of indivudal bitboards
        //todo: is it really efficient to redefine these everytime? maybe can optimize where this is defined
        //assuming knight is at bit 21 or F3 or (x3, y5)
        unsigned long long pos_moves;

        for (auto bb: r_bbs) {
            // get moves\
            //todo: make this a lookup table for improved performance
            int kn_bit = (int) log2(bb);

            if ((bb & PINNED) == 0u) {   //only check for moves if it's not pinned. pinned knights cannot move.

                if (kn_bit > 21) {
                    pos_moves = KNIGHT_MOVES << (kn_bit - 21);
                } else {
                    pos_moves = KNIGHT_MOVES >> (21 - kn_bit);
                }
                if (kn_bit % 8 > 3) {
                    pos_moves &= ~FILE_AB;
                } else {
                    pos_moves &= ~FILE_GH;
                }
                pos_moves &= ~PIECES & checker_zone;

                //viz_bb(pos_moves);

                // todo: find a more efficient way to loop through the board (by being smart about it)
                bitset<64> moves(pos_moves);
                // loop through moves and append to list, if there are any
                if (moves != 0) {
                    string ind_i = ind_x(kn_bit, 0) + ind_y(kn_bit, 0) + ">";
                    for (int i = 0; i < 64; i++) {
                        if (moves[i] == 1) {
                            string ind_f = ind_x(i, 0) + ind_y(i, 0);
                            wb_moves.emplace_back(ind_i + ind_f);
                        }
                    }
                }
                pos_moves = 0u;
            }
        }
    }
}
void get_king_moves(unsigned long long K, unsigned long long PIECES, unsigned long long DZ, vector<string>& wb_moves) {
        //todo: is it really efficient to redefine these everytime? maybe can optimize where this is defined
         //assuming knight is at bit 21 or F3 or (x3, y5)
        unsigned long long pos_moves;

        // get moves
        //todo: make this a lookup table for improved performance
        int k_bit = (int) log2(K);

        if (k_bit > 14) {
            pos_moves = KING_MOVES << (k_bit - 14);
        }
        else{
            pos_moves = KING_MOVES >> (14 - k_bit);
        }
        //todo: potential to make the FILEs more efficient
        if (k_bit % 8 > 3){
            pos_moves &= ~FILE_A;
        }
        else{
            pos_moves &= ~FILE_H;
        }
        pos_moves &= ~PIECES & ~DZ;

        // todo: find a more efficient way to loop through the board (by being smart about it)
        bitset<64>moves(pos_moves);
        // loop through moves and append to list, if there are any
        if (moves != 0) {
            string ind_i = ind_x(k_bit, 0) + ind_y(k_bit, 0) + ">";
            for (int i = 0; i < 64; i++) {
                if (moves[i] == 1) {
                    string ind_f = ind_x(i, 0) + ind_y(i, 0);
                    wb_moves.emplace_back(ind_i + ind_f);
                }
            }
        }
}

void get_X_pawn_moves(string X, unsigned long long MASK, unsigned long long P, unsigned long long K, unsigned long long E_P, unsigned long long EMPTY, unsigned long long OPP_PIECES, unsigned long long checker_zone, vector<string>& moves){
    unsigned long long P_FORWARD_1, P_FORWARD_2, P_ATTACK_L, P_ATTACK_R, P_PROMO_1, P_PROMO_L, P_PROMO_R;
    bitset<64> bits;

    if (X == "B") {
        P_FORWARD_1 = (P >> 8) & EMPTY & ~RANK_1 & MASK & checker_zone;
        P_FORWARD_2 = (P >> 16) & EMPTY & (EMPTY >> 8) & RANK_5 & MASK & checker_zone;
        P_ATTACK_L = (P >> 9) & OPP_PIECES & ~RANK_1 & ~FILE_H & MASK & checker_zone;
        P_ATTACK_R = (P >> 7) & OPP_PIECES & ~RANK_1 & ~FILE_A & MASK & checker_zone;
        P_PROMO_1 = (P >> 8) & EMPTY & RANK_1 & MASK & checker_zone;
        P_PROMO_L = (P >> 9) & OPP_PIECES & RANK_1 & ~FILE_H & MASK & checker_zone;
        P_PROMO_R = (P >> 7) & OPP_PIECES & RANK_1 & ~FILE_A & MASK & checker_zone;

      //  viz_bb(P_PROMO_L);

        //CHECK TO SEE IF WE CAN MOVE 1 SPACE FORWARD
        if (P_FORWARD_1 > 0u) {
            bits = P_FORWARD_1;    //check to see if you can move 1
            for (int i = 8; i < 48; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 0));
                }
            }
        }

        if (P_FORWARD_2 > 0u) {
            bits = P_FORWARD_2;   //check to see if you can move 2
            for (int i = 32; i < 40; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 2, 0) + ">2");
                }
            }
        }

        if (P_ATTACK_L > 0u) {
            bits = P_ATTACK_L;     //check for attacks left
            for (int i = 8; i < 48; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 1));
                }
            }
        }

        if (P_ATTACK_R > 0u) {
            bits = P_ATTACK_R;    //check for attacks right
            for (int i = 8; i < 48; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, -1));
                }
            }
        }

        //check for promotion straight
        if (P_PROMO_1 > 0u) {
            bits = P_PROMO_1;
            for (int i = 0; i < 8; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_L > 0u) {
            bits = P_PROMO_L;      //check for promotion left
            for (int i = 0; i < 8; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_R > 0u) {
            bits = P_PROMO_R;      //check for promotion attack right
            for (int i = 0; i < 8; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PN");
                }
            }
        }

        if(E_P != 0) {
            //todo: specialize this for white
            if (checker_zone != FILLED and ((E_P << 8) & checker_zone) != 0){
                checker_zone |= (file_masks[loc_masks[(int)log2(checker_zone)][1]] & RANK_3);
            }
          //  viz_bb(E_P << 8);
          //  viz_bb(checker_zone);
           // if((E_P & checker_zone) =! 0)
            unsigned long long P_EP_L = (P >> 9) & E_P & ~FILE_H & MASK & checker_zone;
            unsigned long long P_EP_R = (P >> 7) & E_P & ~FILE_A & MASK & checker_zone;

            if (P_EP_L > 0u) {
                //check for en passant left
                bits = P_EP_L;
         //       viz_bb(E_P);
           //     viz_bb(MASK);
             //   viz_bb(checker_zone);
                for (int i = 16; i < 24; i++) {
                    if (bits[i] == 1) {
                        moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "EP");
                    }
                }
            }

            if (P_EP_R > 0u) {
                //check for en passant right
                bits = P_EP_R;
                for (int i = 16; i < 24; i++) {
                    if (bits[i] == 1) {
                        moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "EP");
                    }
                }
            }
        }
    }
    else {    //case for W

        P_FORWARD_1 = (P << 8) & EMPTY & ~RANK_8 & MASK & checker_zone;
        P_FORWARD_2 = (P << 16) & EMPTY & (EMPTY << 8) & RANK_4 & MASK & checker_zone;
        P_ATTACK_L = (P << 7) & OPP_PIECES & ~RANK_8 & ~FILE_H & MASK & checker_zone;
        P_ATTACK_R = (P << 9) & OPP_PIECES & ~RANK_8 & ~FILE_A & MASK & checker_zone;
        P_PROMO_1 = (P << 8) & EMPTY & RANK_8 & MASK & checker_zone;
        P_PROMO_L = (P << 7) & OPP_PIECES & RANK_8 & ~FILE_H & MASK & checker_zone;
        P_PROMO_R = (P << 9) & OPP_PIECES & RANK_8 & ~FILE_A & MASK & checker_zone;

        if (P_FORWARD_1 > 0u) {

            bits = P_FORWARD_1;   // check to see if you can move 1
            for (int i = 16; i < 56; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -1, 0));
                }
            }
        }

        if (P_FORWARD_2 > 0u) {
            bits = P_FORWARD_2;        // check to see if you can move 2
            for (int i = 24; i < 32; i++) {

                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -2, 0)+ ">2");
                }
            }
        }

        if (P_ATTACK_L > 0u) {
            bits = P_ATTACK_L;         //check for attacks left
            for (int i = 16; i < 56; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -1, 1));
                }
            }
        }

        if (P_ATTACK_R > 0u) {
            bits = P_ATTACK_R;         //check for attacks right
            for (int i = 16; i < 56; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -1, -1));
                }
            }
        }

        if (P_PROMO_1 > 0u) {
            bits = P_PROMO_1;          //check for promotion straight
            for (int i = 56; i < 64; i++) {
                if (bits[i] == 1) {
                    //todo: can i optimize by storing a ind string variable
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_L > 0u) {
            bits = P_PROMO_L;          //check for promotion left
            for (int i = 56; i < 64; i++) {
                if (bits[i] == 1) {
                    //todo: can i optimize by storing a ind string variable
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_R > 0u) {
            bits = P_PROMO_R;          //check for promotion attack right
            for (int i = 56; i < 64; i++) {
                if (bits[i] == 1) {
                    //todo: can i optimize by storing a ind string variable
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PN");
                }
            }
        }

        if(E_P != 0) {
            if (checker_zone != FILLED and ((E_P >> 8) & checker_zone) != 0){
                checker_zone |= (file_masks[loc_masks[(int)log2(checker_zone)][1]] & RANK_6);
            }

            unsigned long long P_EP_L = (P << 7) & E_P & ~FILE_H & MASK & checker_zone;
            unsigned long long P_EP_R = (P << 9) & E_P & ~FILE_A & MASK & checker_zone;

            if (P_EP_L != 0u) {
                //check for en passant left
                bits = P_EP_L;
                for (int i = 40; i < 48; i++) {
                    if (bits[i] == 1) {

                        moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "EP");

                    }
                }
            }

            if (P_EP_R != 0u) {
                //check for en passant right
                bits = P_EP_R;
                for (int i = 40; i < 48; i++) {
                    if (bits[i] == 1) {
                        moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "EP");
                    }
                }
            }
        }

    }
}
void get_B_pawn_moves(unsigned long long BP, unsigned long long BK, unsigned long long E_P, unsigned long long EMPTY, unsigned long long WHITE_PIECES, unsigned long long PINNED, unsigned long long checker_zone, unsigned long long E_P_SPECIAL, vector<string>& b_moves) {

    unsigned long long mask;
    unsigned long long pinned_pawns = (BP & PINNED);
    if (checker_zone == 0){checker_zone = FILLED;}
    if (pinned_pawns > 0u){  //we have at least 1 pawn pinned
     //   viz_bb((BP & PINNED));
        BP &= ~PINNED;
        vector<unsigned long long> pin_bbs; //individual bitboards for each piece
        ind_bbs(pinned_pawns, pin_bbs);
        for (auto bb: pin_bbs) {
            //cout<<bb<<endl;
            mask = (get_mask(bb, BK) | E_P_SPECIAL) ;
          //  viz_bb(bb);
            //viz_bb(mask);
          //  viz_bb(E_P_SPECIAL);
         //   viz_bb(E_P);
        // cout<<"pinned"<<endl;
            get_X_pawn_moves("B", mask, bb, BK, E_P, EMPTY, WHITE_PIECES, checker_zone, b_moves);


        }
    }

    if (BP > 0u){   // we have at least 1 non-pinned pawn
        mask = FILLED;
        get_X_pawn_moves("B", mask, BP, BK, E_P, EMPTY, WHITE_PIECES, checker_zone, b_moves);

    }
}
void get_W_pawn_moves(unsigned long long WP, unsigned long long WK, unsigned long long E_P, unsigned long long EMPTY, unsigned long long BLACK_PIECES, unsigned long long PINNED, unsigned long long checker_zone, unsigned long long E_P_SPECIAL, vector<string>& w_moves){

    unsigned long long mask;
    unsigned long long pinned_pawns = (WP & PINNED);
    if (checker_zone == 0){checker_zone = FILLED;}

    if (pinned_pawns > 0u){  //we have at least 1 pawn pinned
        WP &= ~PINNED;

        vector<unsigned long long> pin_bbs; //individual bitboards for each piece
        ind_bbs(pinned_pawns, pin_bbs);
        for (auto bb: pin_bbs) {
            mask = get_mask(bb, WK) | E_P_SPECIAL;

        //    if(bb & )

            get_X_pawn_moves("W", mask, bb, WK, E_P, EMPTY, BLACK_PIECES, checker_zone, w_moves);
        }
    }

    if (WP > 0u){   // we have at least 1 non-pinned pawn
        mask = FILLED;
        get_X_pawn_moves("W", mask, WP, WK, E_P, EMPTY, BLACK_PIECES, checker_zone, w_moves);
    }
}

void get_K_castle(bool CK, unsigned long long K, unsigned long long EMPTY, unsigned long long DZ, vector<string>& wb_moves){
    if(CK) {
        //todo: implement lookup table
        if(((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)) != 0u){
            int k_bit = (int) log2(((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)));
            wb_moves.emplace_back(b_2_ind(k_bit, 0, -2) + ">CK");
        }
    }
}
void get_Q_castle(bool QK, unsigned long long K, unsigned long long EMPTY, unsigned long long DZ, vector<string>& wb_moves){

    if(QK) {
     //   viz_bb(DZ);
       // viz_bb(((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1));
        //todo: implement lookup table
        if((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)) != 0u){
            int k_bit = (int) log2((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)));
            wb_moves.emplace_back(b_2_ind(k_bit, 0, 2) + ">CQ");
        }
    }
}

unsigned long long unsafe_for_XK(string X, unsigned long long P, unsigned long long R, unsigned long long N, \
                        unsigned long long B, unsigned long long Q, unsigned long long K, unsigned long long EK, unsigned long long OCCUPIED) {

    unsigned long long unsafe = 0u, D = B | Q, HV = R | Q;

    //pawn
    if (P != 0u) {
        if (X == "B") {
            unsafe = (P << 9) & ~FILE_A;  //capture right
            unsafe |= (P << 7) & ~FILE_H; //capture left
        }
        else {
            unsafe = (P >> 7) & ~FILE_A;  //capture right
            unsafe |= (P >> 9) & ~FILE_H; //capture left
        }
    }

    //knight
    unsigned long long pos_moves;
    if (N != 0u) {
        vector<unsigned long long> r_bbs; //individual bitboards for each piece
        ind_bbs(N, r_bbs);    //generate list of indivudal bitboards
        //todo: is it really efficient to redefine these everytime? maybe can optimize where this is defined
        //assuming knight is at bit 21 or F3 or (x3, y5)
        for (auto bb: r_bbs) {
            //todo: make this a lookup table for improved performance
            int kn_bit = (int) log2(bb);
            if (kn_bit > 21) { pos_moves = KNIGHT_MOVES << (kn_bit - 21); }
            else { pos_moves = KNIGHT_MOVES >> (21 - kn_bit); }
            if (kn_bit % 8 > 3) { pos_moves &= ~FILE_AB; }
            else { pos_moves &= ~FILE_GH; }
            //viz_bb(pos_n_moves);
            unsafe |= pos_moves;
        }



    }

    //diag pieces (Bishop, Queen)
    if (D != 0u) {
        vector<unsigned long long> r_bbs; //individual bitboards for each piece
        ind_bbs(D, r_bbs);    //generate list of indivudal bitboards
        for (auto bb: r_bbs) {
            //todo: make a lookup table to avoid the log calc
            int sl_bit = (int) log2(bb);
            unsafe |= diag_moves(bb, sl_bit, OCCUPIED, true, EK);
        }
    }

    //hv pieces (Rook, Queen)
    if (HV != 0u) {
        //todo: should i keep redeclaring this variable? Orsimply wipe the last instance?
        vector<unsigned long long> r_bbs; //individual bitboards for each piece
        ind_bbs(HV, r_bbs);    //generate list of indivudal bitboards
        for (auto bb: r_bbs) {
            //todo: make a lookup table to avoid the log calc
            int sl_bit = (int) log2(bb);
            unsafe |= h_v_moves(bb, sl_bit, OCCUPIED, true, EK);
        }

    }

    //king
    //todo: make this a lookup table for improved performance
    int k_bit = (int) log2(K);
    if (k_bit > 14) {pos_moves = KING_MOVES << (k_bit - 14);}
    else {pos_moves = KING_MOVES >> (14 - k_bit);}
    //todo: potential to make the FILEs more efficient
    if (k_bit % 8 > 3) {pos_moves &= ~FILE_A;}
    else {pos_moves &= ~FILE_H;}
    unsafe |= pos_moves;
    //end

    return unsafe;
}

void get_B_moves(unsigned long long BP, unsigned long long BR, unsigned long long BN, unsigned long long BB, \
                unsigned long long BQ, unsigned long long BK, unsigned long long WQ, unsigned long long WB, unsigned long long WR, unsigned long long WN, unsigned long long WP, unsigned long long WK, unsigned long long E_P, \
                bool& BCK, bool& BCQ, bool& CM, bool& SM, vector<string>& b_moves){

    unsigned long long BLACK_PIECES = BR | BN | BB | BQ | BK | BP, WHITE_PIECES = WR | WN | WB | WQ | WK | WP, OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    unsigned long long DZ = unsafe_for_XK("B", WP, WR, WN, WB, WQ, WK, BK, OCCUPIED);
    b_moves.clear();
    unsigned long long E_P_SPECIAL = 0u;

    //DZ is the danger zone. If the king is inside of it, its in check.
    int num_checkers = 0;
    unsigned long long PINNED = get_pinned_pieces(BK, BP, WQ, WB, WR, OCCUPIED, E_P, E_P_SPECIAL); //todo: need to put this to work. dont generate pinned moves if in check, skip that piece
    bool check = (DZ & BK) != 0u;
// ------------------
    unsigned long long checkers = 0, new_checker, checker_zone = 0;    //checker zone is the area that the piece is attacking through (applies only to sliders). We have the potential to block the check by moving  apiece in the line of fire (pinning your own piece)

    // ------------------
    if(check) {  //currently in check
    // todo: generate checkers_bb, update_num_checkers. create method.
        unsigned long long HV = WR | WQ;
        int k_bit = (int)log2(BK);
        unsigned long long K_moves;

        //check horizontal pieces
        K_moves = h_moves(BK, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u) {
            checkers |= new_checker;
            checker_zone |= h_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check vertical pieces
        K_moves = v_moves(BK, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= v_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        unsigned long long D = WB | WQ;
        //check down and to the right pieces
        K_moves = ddr_moves(BK, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= ddr_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check up and to the right pieces
        K_moves = dur_moves(BK, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= dur_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check for knight attacks
        if (k_bit > 21) {K_moves = KNIGHT_MOVES << (k_bit - 21);}
        else {K_moves = KNIGHT_MOVES >> (21 - k_bit);}
        if (k_bit % 8 > 3) {
            K_moves &= ~FILE_AB;
        } else {
            K_moves &= ~FILE_GH;
        }
        new_checker = K_moves & WN;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }


        //check for pawn right attack (from pawns perspective)
        K_moves = (BK >> 9) & ~FILE_H;
        //viz_bb(K_moves & WP);
        new_checker = K_moves & WP;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }

        //check for pawn left attack (from pawns perspective)
        K_moves = (BK >> 7) & ~FILE_A;
      //  viz_bb(K_moves & WP);
        new_checker = K_moves & WP;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }
        //  viz_bb(checkers);
        // viz_bb(checker_zone);

    }
    else{   //only search for castles if you aren't in check
        get_K_castle(BCK, BK, ~OCCUPIED, DZ, b_moves);
        get_Q_castle(BCQ, BK, ~OCCUPIED, DZ, b_moves);
    }

    checker_zone |= checkers;
   // viz_bb(checker_zone);
   // cout<<num_checkers<<endl;

// todo: pass check zones into the files

    if (num_checkers < 2 ) {

        get_B_pawn_moves(BP, BK, E_P, ~OCCUPIED, WHITE_PIECES, PINNED, checker_zone, E_P_SPECIAL, b_moves);
        get_rook_moves(BR, BK, BLACK_PIECES, OCCUPIED, PINNED, checker_zone, b_moves);
        get_bishop_moves(BB, BK, BLACK_PIECES, OCCUPIED, PINNED, checker_zone, b_moves);
        get_queen_moves(BQ, BK, BLACK_PIECES, OCCUPIED, PINNED, checker_zone,b_moves);
        get_knight_moves(BN, BK, BLACK_PIECES, PINNED, checker_zone, b_moves);

    }
    get_king_moves(BK, BLACK_PIECES, DZ, b_moves);

    if (b_moves.empty() and check){CM = true;}
    else if (b_moves.empty() and !check){SM = true;}
    else if ((BK | WK) == OCCUPIED){SM = true;}

   // return check;

}
void get_W_moves(unsigned long long WP, unsigned long long WR, unsigned long long WN, unsigned long long WB, \
                unsigned long long WQ, unsigned long long WK, unsigned long long BQ, unsigned long long BB, unsigned long long BR, unsigned long long BN, unsigned long long BP, unsigned long long BK, unsigned long long E_P,  \
                 bool& WCK, bool& WCQ, bool& CM, bool& SM, vector<string>& w_moves){

    unsigned long long BLACK_PIECES = BR | BN | BB | BQ | BK | BP, WHITE_PIECES = WR | WN | WB | WQ | WK | WP, OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    w_moves.clear();


    unsigned long long E_P_SPECIAL = 0u;

    unsigned long long DZ = unsafe_for_XK("W", BP, BR, BN, BB, BQ, BK, WK, OCCUPIED);
   // viz_bb(DZ);
    //DZ is the danger zone. If the king is inside of it, its in check.
    int num_checkers = 0;
    unsigned long long PINNED = get_pinned_pieces(WK, WP, BQ, BB, BR, OCCUPIED, E_P, E_P_SPECIAL); //todo: need to put this to work. dont generate pinned moves if in check, skip that piece
    bool check = (DZ & WK) != 0u;
// ------------------
    unsigned long long checkers = 0, new_checker, checker_zone = 0;    //checker zone is the area that the piece is attacking through (applies only to sliders). We have the potential to block the check by moving  apiece in the line of fire (pinning your own piece)
   // cout<<check<<endl;

    // ------------------
    if(check) {  //currently in check
        // todo: generate checkers_bb, update_num_checkers. create method.
        unsigned long long HV = BR | BQ;
        int k_bit = (int)log2(WK);
        unsigned long long K_moves;

        //check horizontal pieces
        K_moves = h_moves(WK, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u) {
            checkers |= new_checker;
            checker_zone |= h_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check vertical pieces
        K_moves = v_moves(WK, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= v_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        unsigned long long D = BB | BQ;
        //check down and to the right pieces
        K_moves = ddr_moves(WK, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= ddr_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check up and to the right pieces
        K_moves = dur_moves(WK, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= dur_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check for knight attacks
        if (k_bit > 21) {
            K_moves = KNIGHT_MOVES << (k_bit - 21);}
        else {
            K_moves = KNIGHT_MOVES >> (21 - k_bit);}

        if (k_bit % 8 > 3) {
            K_moves &= ~FILE_AB;
        } else {
            K_moves &= ~FILE_GH;
        }

        new_checker = K_moves & BN;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }

        //check for pawn right attack (from pawns perspective)
        K_moves = (WK << 7) & ~FILE_H;   //todo: verify
        //viz_bb(K_moves & WP);
        new_checker = K_moves & BP;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }
        //viz_bb(checkers);

        //check for pawn left attack (from pawns perspective)
        K_moves = (WK << 9) & ~FILE_A;  //todo: verify
        //  viz_bb(K_moves & WP);
        new_checker = K_moves & BP;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }
        //  viz_bb(checkers);
        // viz_bb(checker_zone);

    }
    else{   //only search for castles if you aren't in check
        get_K_castle(WCK, WK, ~OCCUPIED, DZ, w_moves);
        get_Q_castle(WCQ, WK, ~OCCUPIED, DZ, w_moves);
    }

    checker_zone |= checkers;
// todo: pass check zones into the files

    if (num_checkers < 2 ) {
        get_W_pawn_moves(WP, WK, E_P, ~OCCUPIED, BLACK_PIECES, PINNED, checker_zone, E_P_SPECIAL, w_moves);
        get_rook_moves(WR, WK, WHITE_PIECES, OCCUPIED, PINNED, checker_zone, w_moves);
        get_bishop_moves(WB, WK, WHITE_PIECES, OCCUPIED, PINNED, checker_zone, w_moves);
        get_queen_moves(WQ, WK, WHITE_PIECES, OCCUPIED, PINNED, checker_zone,w_moves);
        get_knight_moves(WN, WK, WHITE_PIECES, PINNED, checker_zone, w_moves);

    }
    get_king_moves(WK, WHITE_PIECES, DZ, w_moves);

    if (w_moves.empty() and check){CM = true;}
    else if (w_moves.empty() and !check){SM = true;}
    else if ((BK | WK) == OCCUPIED){SM = true;}

  //  return check;
}

void apply_move(bool& white_move, string move, unsigned long long& R, unsigned long long& N, unsigned long long& B, \
                unsigned long long& Q, unsigned long long& K, unsigned long long& P, unsigned long long& OR, \
                unsigned long long& ON, unsigned long long& OB, unsigned long long& OQ, unsigned long long& OK, \
                unsigned long long& OP, unsigned long long& E_P, bool& WCK, bool& WCQ, bool& BCK, bool& BCQ){

    unsigned long long WHITE_PIECES, BLACK_PIECES;

    if (white_move) {
        WHITE_PIECES = (R | N | B | Q | K | P);
        BLACK_PIECES = (OR | ON | OB | OQ | OK | OP);
    }
    else{
        BLACK_PIECES = (R | N | B | Q | K | P);
        WHITE_PIECES = (OR | ON | OB | OQ | OK | OP);
    }

    int x1 = stoi(move.substr(0,1)), y1 = stoi(move.substr(1,1));
    int x2 = stoi(move.substr(3,1)), y2 = stoi(move.substr(4,1));

    unsigned long long initial = pow(2,((x1 * 8) + (y1 % 8)));
    unsigned long long final = pow(2,((x2 * 8) + (y2 % 8)));


    //-----discover what piece is being moved-----
//    unsigned long long moving_piece;
//    if ((Q & initial) != 0){moving_piece = Q;}
//    else if ((B & initial) != 0){moving_piece = B;}
//    else if ((R & initial) != 0){moving_piece = R;}
//    else if ((N & initial) != 0){moving_piece = N;}
//    else if ((P & initial) != 0){moving_piece = P;}
//    else if ((K & initial) != 0){moving_piece = K;}

    //-----remove enemy pieces in the case of a capture-----
    unsigned long long capture;

    if (white_move){
        capture = (BLACK_PIECES & ~OK) & final;} else{ capture = WHITE_PIECES & ~OK & final;}

    if (capture != 0) {
        if ((OP & final) != 0) {
            OP &= ~final;
             //cap_counter++;
        } else if ((OR & final) != 0) {
            //   cout<<"-------PRE-MOVE-------"<<endl;if (white_move){print_board(OR, ON, OB, OQ, OK, OP, R, N, B, Q, K, P);}else {print_board(R, N, B, Q, K, P, OR, ON, OB, OQ, OK, OP);}

            if (white_move){

                if((OR & 9223372036854775808u & final) == 9223372036854775808u){BCK = false;
                }
                else if ((OR & 72057594037927936u & final) == 72057594037927936u){BCQ = false;
              //      cout<<"hi"<<endl;
              //
              }
            }
            else{
                if((OR & 128u & final) == 128u){WCK = false;}
                else if ((OR & 1u &final) == 1u){WCQ = false;}
            }

            OR &= ~final;
            // cout<<"-------POST-MOVE-------"<<endl;if (white_move){print_board(OR, ON, OB, OQ, OK, OP, R, N, B, Q, K, P);}else {print_board(R, N, B, Q, K, P, OR, ON, OB, OQ, OK, OP);}
            //cap_counter++;
        } else if ((ON & final) != 0) {
            ON &= ~final;
          //  cap_counter++;
        } else if ((OB & final) != 0) {
            OB &= ~final;
         //   cap_counter++;
//            cout << "opponent bishops" << endl;
//            viz_bb(OB & final);
//            cout << "init" << endl;
//            viz_bb(initial);
//            cout << "final" << endl;
//
//            viz_bb(final);
        } else if ((OQ & final) != 0) {
            OQ &= ~final;
         //   cap_counter++;
            // ;
        }

    }
    else if (E_P == final and ((initial & P) != 0)){ //this means there was an en passant capture
     //   cout<<"-------PRE-MOVE-------"<<endl;if (white_move){print_board(OR, ON, OB, OQ, OK, OP, R, N, B, Q, K, P);}else {print_board(R, N, B, Q, K, P, OR, ON, OB, OQ, OK, OP);}

        if (white_move){OP &= ~(final >> 8);}
        else{OP &= ~(final << 8);}
       // cap_counter++;

    //    cout<<"-------POST-MOVE-------"<<endl;if (white_move){print_board(OR, ON, OB, OQ, OK, OP, R, N, B, Q, K, P);}else {print_board(R, N, B, Q, K, P, OR, ON, OB, OQ, OK, OP);}

       // cout<<"HEY"<<endl;
       // viz_bb(final);
    }
   // -----this concludes removing enemy pieces from board-----


   // need to move piece to the final position and also remove the initial position
   if (move.length() == 5){
    //   cout<<"hello"<<endl;
       //viz_bb(P);
    //todo: search for which piece is moving
        if ((Q & initial) != 0){
            Q |= final;
            Q &= ~initial;
        }
        else if ((B & initial) != 0){
            B |= final;
            B &= ~initial;
        }
        else if ((R & initial) != 0){

            if (WCK and white_move and (R & 128u & initial) == 128u){WCK = false;}
            else if (WCQ and white_move and (R & 1u & initial) == 1u){WCQ = false;}
            else if (BCK and !white_move and (R & 9223372036854775808u & initial) == 9223372036854775808u){BCK = false;}
            else if (BCQ and !white_move and (R & 72057594037927936u & initial) == 72057594037927936u){BCQ = false;

            //cout<<"HEY"<<endl;
            }
            R |= final;
            R &= ~initial;
        }
        else if ((N & initial) != 0){
            N |= final;
            N &= ~initial;
        }
        else if ((P & initial) != 0){
         //   cout<<"hello2"<<endl;

            P |= final;
            P &= ~initial;
        }
        else if ((K & initial) != 0){
            if ((WCK or WCQ) and white_move){WCK = false; WCQ = false;}
            if ((BCK or BCQ) and !white_move){BCK = false; BCQ = false;}
          //  cout<<WCK<<endl;
            K = final;
        }

        E_P = 0u;
   }
   else{
       if (move.substr(6,1) == "C"){    //castling
           if(white_move){
               if (move.substr(7,1) == "K"){ //kingside castle for white

                   K = 64u; R -= 128u; R += 32u;
                   WCK = false; WCQ = false;
               }
               else{    //queenside castle for black
                   K = 4u; R -= 1u; R += 8u;
                   WCQ = false; WCK = false;
               }
           }
           else{

               if (move.substr(7,1) == "K"){ //kingside castle for black
                   K = 4611686018427387904u; R -= 9223372036854775808u; R += 2305843009213693952u;
                   BCK = false; BCQ = false;
               }
               else{    //queenside castle for black
                   K = 288230376151711744u; R -= 72057594037927936u; R += 576460752303423488u;
                   BCQ = false; BCK = false;
               }
           }
           E_P = 0u;
       }
       else if (move.substr(6,1) == "P"){   //promotion
           P &= ~initial;
           if(move.substr(7,1) == "Q"){Q |= final;}
           else if (move.substr(7,1) == "R"){R |= final;}
           else if (move.substr(7,1) == "N"){N |= final;}
           else if (move.substr(7,1) == "B"){B |= final;}
           E_P = 0u;
       }
       else if (move.substr(6,1) == "E"){    //en passant capture

           P |= final;
           P &= ~initial;
           E_P = 0u;
       }
       else { // pawn push 2

           P |= final;
           P &= ~initial;
           if (white_move){E_P = (final >> 8);}
           else{E_P = (final << 8);}


       }
   }

   white_move = !white_move;   //alternate the turn


}



void print_moves(bool white_move, vector<string> b_moves, vector<string> w_moves){

    if (white_move){
        cout<< "WHITE'S MOVE: "<<endl;
        for (int i = 0; i < w_moves.size(); i++) {
            cout << i + 1 << ": " + w_moves[i] << endl;
        }
      //  cout << "total moves: " << w_moves.size() << endl;
    }
    else{
        cout<< "BLACK'S MOVE: "<<endl;
        for (int i = 0; i < b_moves.size(); i++) {
            cout << i + 1 << ": " + b_moves[i] << endl;
        }
    }
}

bool aa = false;

int perft(long &nodes, int& cap_counter, bool& white_move, unsigned long long& WR, unsigned long long& WN, unsigned long long& WB, \
                unsigned long long& WQ, unsigned long long& WK, unsigned long long& WP, unsigned long long& BR, \
                unsigned long long& BN, unsigned long long& BB, unsigned long long& BQ, unsigned long long& BK, \
                unsigned long long& BP, unsigned long long WHITE_PIECES, unsigned long long BLACK_PIECES, unsigned long long OCCUPIED, vector<string> moves, unsigned long long& E_P, bool& WCK, bool& WCQ, bool& BCK, bool& BCQ, bool CM, bool SM, int depth, int orig_depth, string n){


            BLACK_PIECES = (BR | BN | BB | BQ | BK | BP), WHITE_PIECES = (WR | WN | WB | WQ | WK | WP), OCCUPIED = (
                    BLACK_PIECES | WHITE_PIECES);
            bool check = false;
            if (white_move) {
                get_W_moves(WP, WR, WN, WB, WQ, WK, BQ, BB, BR, BN, BP, BK, E_P, WCK, WCQ, CM, SM, moves);
            } else {
                get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ, CM, SM, moves);
            }


            //    nodes++; //cap_counter += cap_count_temp;
            //nodes++;
            if (depth != 0) {

                //cout<<


                for (int i = 0; i < moves.size(); i++) {
                    // nodes++;
                    //cout<<i<<endl;
                    int cap_count_temp = 0;

                    //   cout<<i<<endl;

//                    if (depth == 2) { cout << "  d1: " << moves[i] << endl; }
//                    if (depth == 1) { cout << i << "     d2: " << moves[i] << endl; }
//                    if (depth == 1) {
//                        if (moves[i].size() > 5 and moves[i].substr(6, 1) == "E") {
//                            cout << "EP" << endl;



                    unsigned long long BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, BLACK_PIECESt = BLACK_PIECES, WHITE_PIECESt = WHITE_PIECES, OCCUPIEDt = OCCUPIED, E_Pt = E_P;
                    bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;
                    if (white_movet) {
                        apply_move(white_movet, moves[i], WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt,
                                   BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt);
                    } else {
                        apply_move(white_movet, moves[i], BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt,
                                   WQt, WKt, WPt, E_Pt, WCKt, WCQt, BCKt, BCQt);
                    }



                    //  cout<<"depth: "<< depth<<endl;
                    //  cout<<"nodes: "<< nodes<<endl;

                    if (depth == 1) {
                        nodes++;
                      //  cap_counter += cap_count_temp;

                    }
                    //  else if (CMt or)
                    perft(nodes, cap_counter, white_movet, WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt, BQt, BKt, BPt,
                          WHITE_PIECESt, BLACK_PIECESt, OCCUPIEDt, moves, E_Pt, WCKt, WCQt, BCKt, BCQt, CMt, SMt,
                          depth - 1, orig_depth, n);

                    if (depth == orig_depth) {
                        //          viz_bb( WPt);
                        if (n == "total") {
                        cout << round(((i * 100 / moves.size()))) << "% complete... -> d1:" << moves[i]
                             << "--------------------------------------------------" << endl;
                            }
                        else if (n == "node") {
                               cout<<i <<":"<<moves[i] <<" "<<nodes<<  endl;
                              nodes = 0;
                        }

                    }else if (depth == orig_depth - 1 and false){
                        if (n == "total") {
                            cout << round(((i * 100 / moves.size()))) << "% complete... -> d1:" << moves[i]
                                 << "--------------------------------------------------" << endl;
                        }
                        else if (n == "node") {
                            cout<< "     "<<i <<":"<<moves[i] <<" "<<nodes<<  endl;
                         //   nodes = 0;
                        }
                    }

                }
            }

}

int nodes2 = 0;

double eval(unsigned long long WR, unsigned long long WN, unsigned long long WB, unsigned long long WQ, unsigned long long WK, \
            unsigned long long WP, unsigned long long BR, unsigned long long BN, unsigned long long BB, unsigned long long BQ, \
            unsigned long long BK, unsigned long long BP){

           //material
           double counter = 0;
            counter += ((double)bitset<64>(WP).count() - (double)bitset<64>(BP).count()) * 100;
    counter += ((double)bitset<64>(WB).count() - (double)bitset<64>(BB).count()) * 300;  //todo: add special case regarding number of bishops
    counter += ((double)bitset<64>(WN).count() - (double)bitset<64>(BN).count()) * 300;
    counter += ((double)bitset<64>(WR).count() - (double)bitset<64>(BR).count()) * 500;
    counter += ((double)bitset<64>(WQ).count() - (double)bitset<64>(BQ).count()) * 900;

    return counter;

}


AI_return minimax(bool& white_move, unsigned long long WR, unsigned long long WN, unsigned long long WB, \
                unsigned long long WQ, unsigned long long WK, unsigned long long WP, unsigned long long BR, \
                unsigned long long BN, unsigned long long BB, unsigned long long BQ, unsigned long long BK, \
                unsigned long long BP, unsigned long long E_P, bool WCK, bool WCQ, bool BCK, bool BCQ, bool CM, bool SM, int depth, bool my_turn, double alpha=-100000000, double beta=100000000) {


   // cout<<"alpha: "<<alpha<<". beta: "<<beta<<"."<<endl;


  //  cout<<depth<<endl;
    if (depth == 0) {  //todo: add a conditon for game over
        //todo add evaluation function
      //  cout<<"HEYYYYYYYYYYYY"<<endl;
        nodes2++;
        AI_return leaf = {"string generico", eval(WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP)};
    //    cout<<leaf.value<<endl;
        return leaf;
    }


    if (my_turn) {
        vector<string> w_moves;

        string max_move = " ";
        double max_val = -10000000;
        AI_return a;


        get_W_moves(WP, WR, WN, WB, WQ, WK, BQ, BB, BR, BN, BP, BK, E_P, WCK, WCQ, CM, SM, w_moves);
        if (CM) {// cout << "CHECKMATE. BLACK WINS" << endl;
            AI_return leaf = {"CM", -10000};
            return leaf;}
        if (SM) { //cout << "STALEMATE." << endl;
            AI_return leaf = {"SM", 0};
            return leaf;}

        for (int i = 0; i < w_moves.size(); i++) {
          //   if((depth != 1 and depth != 2) or false){cout << "DEPTH: " << depth << " W move " << i + 1 << ": " << w_moves[i] << endl;}
            //      unsigned long long BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, BLACK_PIECESt = BLACK_PIECES, WHITE_PIECESt = WHITE_PIECES, OCCUPIEDt , E_Pt = E_P;
            //    bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;
            unsigned long long BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, E_Pt = E_P;
            bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;

            apply_move(white_movet, w_moves[i], WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt,
                       BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt);
//
//            if (depth == 3) {
//                //  cout << i << ":" << w_moves[i] << " " << nodes << endl;
//                if ( w_moves[i] == "13>33>2"){
//                    // print_board(BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt, WQt, WKt, WPt);
//                    cout<<"SET EP"<<endl;
//                    viz_bb(E_Pt);
//                    cout<<"WHITE MOVE? "<<white_move<<endl;
//                }}
            a = minimax(white_movet, WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt, BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt, CMt, SMt, depth - 1, !my_turn, alpha, beta);




            if (a.value > max_val) {
                max_val = a.value;
                max_move = w_moves[i];
            }

            alpha = max(alpha, a.value);
            if(beta <= alpha){break;}


        }

        AI_return leaf_node;
        leaf_node.value = max_val;
        leaf_node.move = max_move;
        return leaf_node;

    } else {
        vector<string> b_moves;

        string min_move = " ";
        double min_val = 10000000;
        AI_return a;
        get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ, CM, SM, b_moves);
        if (CM) { //cout << "CHECKMATE. WHITE WINS" << endl;
            AI_return leaf = {"CM", 10000};
            return leaf;}
        if (SM) {// cout << "STALEMATE." << endl;
            AI_return leaf = {"SM", 0};
            return leaf;}

        for (int j = 0; j < b_moves.size(); j++) {
            //   if(depth == 1){nodes ++;}
       //     cout << "DEPTH: " << depth << " B move " << j + 1 << ": " << b_moves[j] << endl;


            unsigned long long BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, E_Pt = E_P;
            bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;

//            if (depth == 2) {
//                if ( b_moves[j] == "63>43>2"){
//                    print_board(BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt, WQt, WKt, WPt);
//                    debug = true;
//                }else{debug = false;}

            apply_move(white_movet, b_moves[j], BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt,
                       WQt, WKt, WPt, E_Pt, WCKt, WCQt, BCKt, BCQt);


            a = minimax(white_movet, WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt, BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt, CMt, SMt, depth - 1, !my_turn, alpha, beta);
            // cout<<"NOOO GOD PLEASE NO"<<endl;
//            print_board(BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt, WQt, WKt, WPt);

            //   cout<<" "<<nodes<<endl;
            //  nodes = 0;
            //   cout << j << ":" << b_moves[j] << " " << nodes << endl;



            if (a.value < min_val) {
                min_val = a.value;
                min_move = b_moves[j];
            }

            beta = min(beta, a.value);
            if(beta <= alpha){break;}
        }

        AI_return leaf_node;
        leaf_node.value = min_val;
        leaf_node.move = min_move;

        return leaf_node;
    }

        //todo: implement the eval function

    }





void read_FEN(char g[8][8], string FEN, bool& white_move, bool& WCK, bool& WCQ, bool& BCK, bool& BCQ){
int row = 0;
int col = 0;


for (int i = 0; i < FEN.length(); i++ ){

    if(row > 7){

        //board has been read in
     //   cout<<to_string(FEN[i])<<endl;
        if(FEN[i] == 'w'){ white_move = true;}
        if(FEN[i] == 'b'){ white_move = false;}
        if(FEN[i] == 'k'){ BCK = true;}
        if(FEN[i] == 'q'){ BCQ = true;}
        if(FEN[i] == 'K'){ WCK = true;}
        if(FEN[i] == 'Q'){ WCQ = true;}



       // break;


    }else {

        if (isdigit(FEN[i])) {
           // cout<<FEN[i]<<endl;
            for (int j = 0; j < (int)(FEN[i]) - 48; j++) {
                g[row][col] = ' ';
              //  cout<<row<<","<<col<<endl;
                col = (col + 1) % 8;
                if (col == 0) { row += 1; }
                //cout<<g[row][col]<<endl;

            }

        } else if (FEN[i] == '/'){}
        else{
            g[row][col] = FEN[i];
           // cout<<row<<","<<col<<endl;
            col = (col + 1) % 8;
            if (col == 0) { row += 1; }
        }
    }

}


}

void generate_board(string name, int diff) {
//
    cout<<"GAME START"<<endl;

    char grid[8][8] ={
            //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

    string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    //FEN = "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1";
    //FEN= "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2";
    bool BCK = false, BCQ = false, WCK = false, WCQ = false, CM = false, SM = false, white_to_move;
    //todo: add E_P functionality to read FEN
    unsigned long long BR = 0u, BN = 0u, BB = 0u, BQ = 0u, BK = 0u, BP = 0u, WR = 0u, WN = 0u, WB = 0u, WQ = 0u, WK = 0u, WP = 0u, BLACK_PIECES, WHITE_PIECES, OCCUPIED, E_P = 0u;//pow(2,20);;

    read_FEN(grid, FEN, white_to_move, WCK, WCQ, BCK, BCQ);

    grid_to_bbs(grid, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
    BLACK_PIECES = BR | BN | BB | BQ | BK | BP, WHITE_PIECES = WR | WN | WB | WQ | WK | WP, OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    AI_return AI_choice;

    int depth;
    if(diff == 1){depth = 4;}
    else if(diff == 2){depth = 5;}
    else if(diff == 3){depth = 6;}
    
    //This is the GAME
    //for now, the AI is only white
    //todo: implement AI for both colors
    srand(time(nullptr));

    Player p = Player(true);
    Player p2 = Player(true, true);
    cout<<p.color<<endl;

    while (!CM and !SM and true){

        BLACK_PIECES = BR | BN | BB | BQ | BK | BP, WHITE_PIECES = WR | WN | WB | WQ | WK | WP, OCCUPIED = BLACK_PIECES | WHITE_PIECES;
       // c

        if(white_to_move){
            print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

            cout<<"WHITE'S MOVE: "<<endl;
            cout<<"AI Agent thinking... wait a few seconds."<<endl;
            auto start = std::chrono::high_resolution_clock::now();
           // cout<<"WHITES MOVE (SHOULD BE 1): "<<white_to_move<<endl;
            AI_choice = minimax(white_to_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, E_P, WCK, WCQ, BCK, BCQ, CM, SM, depth, true);
            auto end = std::chrono::high_resolution_clock::now();

           // cout<<"help"<<endl;
            cout<<"Move chosen: "<<AI_choice.move<<endl;
            cout<<AI_choice.value<<endl;
            cout<<"WHITES MOVE (SHOULD BE 1): "<<white_to_move<<endl;
          //  cout<<"nodes: "<<nodes2<<endl;

            apply_move(white_to_move, AI_choice.move, WR, WN, WB, WQ, WK, WP, BR, BN, BB,
                       BQ, BK, BP, E_P, WCK, WCQ, BCK, BCQ);


            cout<<"depth: "<<depth<<". time elapsed: "<<(double)(end - start).count()/1000000000<<" s. nodes searched: "<<nodes2<<"."<<endl;
            cout<<"NPS: "<< nodes2/((double)(end - start).count()/1000000000)<<endl;
            cout<<" "<<endl;
           // break;
        }
        else{
            print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
            cout<<"BLACK'S MOVE: "<<endl;

            //todo: create a player class for their choosing mechanism
            vector<string> b_moves;

            get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ,
                        CM, SM, b_moves);

//            if (depth == 2) {
//                if ( b_moves[j] == "63>43>2"){
//                    print_board(BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt, WQt, WKt, WPt);
//                    debug = true;
//                }else{debug = false;}
                cout<<"Please select your move: "<<endl;
                print_moves( white_to_move, b_moves, b_moves);

                int user_choice;
                cin >> user_choice;

                apply_move(white_to_move, b_moves[user_choice - 1], BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP, E_P, WCK, WCQ, BCK, BCQ);

                cout<<"Move chosen: "<<b_moves[user_choice - 1]<<endl;
                cout<<" "<<endl;


        }
    }




   // print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);



    //generate_bit();
    int turn_counter = 0, turns, game_counter = 0; string chosen_move;
  //  auto start = std::chrono::high_resolution_clock::now();

    srand (time(nullptr));

//todo: get rid of all pow functions. use bitshifting instead
        vector<string> moves;
      //


    turns = 0;
        for (int i = 0; i < 0; i++) {




            if (white_to_move) {
                get_W_moves(WP, WR, WN, WB, WQ, WK, BQ, BB, BR, BN, BP, BK, E_P, WCK, WCQ, CM, SM, moves);
            } else {
                get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ, CM, SM, moves);
            }

            //check to see if game has ended in checkmate or stalemate
            if (CM and white_to_move) {
             //   cout << "CHECKMATE. BLACK WINS." << endl;
                break;
            }
            else if (CM and !white_to_move) {
            //    cout << "CHECKMATE. WHITE WINS." << endl;
                break;
            }
            else if (SM) {
            //    cout << "STALEMATE. IT'S A DRAW." << endl;
                break;
            }

            //print methods for validation only
           // print_moves(white_to_move, moves, moves);

            //for now, just choose a random move for each player
        //    r = (int) (rand() % moves.size());
        //    chosen_move = moves[r];
          //  cout<<r<<endl;
          //  cout << "MOVE #" << r + 1 << " CHOSEN." << endl;
            int cap_counter = 0;

            // apply the move and update the board
            if (white_to_move) {
                apply_move(white_to_move, chosen_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, E_P, WCK, WCQ, BCK, BCQ);
            } else {
                apply_move(white_to_move, chosen_move, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP, E_P, WCK, WCQ, BCK, BCQ);
            }
            turns++;

            // b_moves.clear(); w_moves.clear();
           // print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

           // moves.clear();
        }

        game_counter++;
        turn_counter += turns;



if (false) {
    long nodes = 0;
    string tests = " ";
    bool new_line = false;
    string FEN_;
    bool FEN_done = false;
    string d_num;
    int num = 0;
    for (int i = 0; i < tests.length(); i++) {
        if (tests[i] == ';' or FEN_done) {
            FEN_done = true;
            //cout<<tests[i]<<tests[i + 1]<<endl;
            if (tests[i] == 'D' and tests[i + 1] == '3') {
                for (int j = 0; j < 100; j++) {
                    // cout<<tests[i + 1 + 2 + j]<<endl;
                    if (isdigit(tests[i + 1 + 2 + j])) { d_num += tests[i + 1 + 2 + j]; }
                    else { break; }
                }
                num = stoi(d_num);
            }
        } else if (FEN_done == false) { FEN_ += tests[i]; }
        if (tests[i] == '\n') {

            cout << FEN_ << "-----> D3: " << d_num << endl;
            FEN_done = false;

            //todo: add code here

            char grid[8][8] = {
                    //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

            BR = 0u, BN = 0u, BB = 0u, BQ = 0u, BK = 0u, BP = 0u, WR = 0u, WN = 0u, WB = 0u, WQ = 0u, WK = 0u, WP = 0u, E_P = 0u;//pow(2,20);;


            int depth = 5;
            // string n = "total";
            string n = "total";

            // cout<<1<<endl;
            nodes = 0;
            int cap_counter = 0;
            BCK = false, BCQ = false, WCK = false, WCQ = false, CM = false, SM = false, white_to_move = true;

            string FEN = FEN_;
            //  string FEN = "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1";
            // FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
            //     FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
            //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1NnPP/RNBQK2R b KQ - 0 8";
            //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1N1PP/RNBQK2n w Q - 0 9";

            read_FEN(grid, FEN, white_to_move, WCK, WCQ, BCK, BCQ);

            grid_to_bbs(grid, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
            // viz_bb(BR);
            //cout<<"hey"<<endl;
            print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

            //  BCK = true, BCQ = true, WCK = true, WCQ = true, CM = false, SM = false, check = false, white_to_move = true;

            // E_P = pow(2,21);
            // throw;

            perft(nodes, cap_counter, white_to_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, WHITE_PIECES,
                  BLACK_PIECES, OCCUPIED, moves, E_P, WCK, WCQ, BCK, BCQ, CM, SM, depth, depth, n);

            if (nodes != num) {
                cout << "generated nodes: " << nodes << endl;
                cout << "expected nodes: " << num << endl;

                throw;
            }

            cout << "depth " << depth << ":" << endl;
            cout << "total nodes: " << nodes << endl;
            cout << "total captures: " << cap_counter << endl;


            FEN_ = "";
            d_num = "";
        }
    }
}

if (false){

        char grid[8][8] = {
                //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

        int depth = 3;
        // string n = "total";
        string n = "total";

        // cout<<1<<endl;
        long nodes = 0;
        int cap_counter = 0;
        BCK = false, BCQ = false, WCK = false, WCQ = false, CM = false, SM = false, white_to_move = true;
        //E_P = pow(2,19);
        //string FEN = FEN_;
         // string FEN = "8/3K4/2p5/p2b2r1/5k2/8/8/1q6 b - 1 67";
          string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
//          FEN = "8/2p5/3p4/KP5r/1R3p1k/4P3/6P1/8 b - - 0 1";
//          FEN = "8/2p5/8/KP1p3r/1R3p1k/4P3/6P1/8 w - - 0 2";
//          FEN = "8/2p5/8/KP1p3r/1R3pPk/4P3/8/8 b - g3 0 2";
        // FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
        //     FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
        //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1NnPP/RNBQK2R b KQ - 0 8";
        //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1N1PP/RNBQK2n w Q - 0 9";

        read_FEN(grid, FEN, white_to_move, WCK, WCQ, BCK, BCQ);

        grid_to_bbs(grid, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
        // viz_bb(BR);
        //cout<<"hey"<<endl;
        print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

        //  BCK = true, BCQ = true, WCK = true, WCQ = true, CM = false, SM = false, check = false, white_to_move = true;

        // E_P = pow(2,21);
        // throw;

        perft(nodes, cap_counter,white_to_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, WHITE_PIECES, BLACK_PIECES, OCCUPIED, moves, E_P, WCK, WCQ, BCK, BCQ, CM, SM, depth, depth, n);


        cout<<"depth "<<depth<<":"<<endl;
        cout<<"total nodes: "<<nodes<<endl;
        cout<<"total captures: "<<cap_counter<<endl;
    }


}

//  for (int i = 0; i <10000000; i++) {
//    a = (unsigned long long) 1u << 63;
// }


//    auto end = std::chrono::high_resolution_clock::now();
//    cout<<"bitshifting: "<<(end - start).count()/10000<<endl;
//
//     start = std::chrono::high_resolution_clock::now();
//
//    for (int i = 0; i <10000000; i++) {
//        a = pow(2, 63);
//    }
//    end = std::chrono::high_resolution_clock::now();
//    cout<<"power: "<<(end - start).count()/10000<<endl;




