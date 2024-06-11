#include "board.h"
#include "helper_functions.h"
#include <iostream>
#include <regex>
#include <string>

/** Fills out char grid according to the game state.
 *
 * @param game_state: Game state.
 * @param grid: Character grid.
 */
void populateGridFromGameState(const GameState &game_state, char grid[8][8]) {
  for (int8_t row = 7; row >= 0; row--) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t bit = ((7 - row) * 8) + col;
      if (game_state.white.pawn & (1ULL << bit)) {
        grid[row][col] = 'P';
      } else if (game_state.white.rook & (1ULL << bit)) {
        grid[row][col] = 'R';
      } else if (game_state.white.knight & (1ULL << bit)) {
        grid[row][col] = 'N';
      } else if (game_state.white.bishop & (1ULL << bit)) {
        grid[row][col] = 'B';
      } else if (game_state.white.queen & (1ULL << bit)) {
        grid[row][col] = 'Q';
      } else if (game_state.white.king & (1ULL << bit)) {
        grid[row][col] = 'K';
      } else if (game_state.black.pawn & (1ULL << bit)) {
        grid[row][col] = 'p';
      } else if (game_state.black.rook & (1ULL << bit)) {
        grid[row][col] = 'r';
      } else if (game_state.black.knight & (1ULL << bit)) {
        grid[row][col] = 'n';
      } else if (game_state.black.bishop & (1ULL << bit)) {
        grid[row][col] = 'b';
      } else if (game_state.black.queen & (1ULL << bit)) {
        grid[row][col] = 'q';
      } else if (game_state.black.king & (1ULL << bit)) {
        grid[row][col] = 'k';
      } else {
        grid[row][col] = ' ';
      }
    }
  }
}

void printBoard(const GameState &game_state) {
  char grid[8][8];
  populateGridFromGameState(game_state, grid);

  std::string dividing_line =
      "|---|-----|-----|-----|-----|-----|-----|-----|-----|";

  std::string line;
  for (uint8_t i = 0; i <= 7; i++) {
    std::cout << dividing_line << std::endl;
    line = "| " + std::to_string(8 - i) + " |  ";
    for (uint8_t j = 0; j < 8; j++) {
      line.push_back(grid[i][j]);
      if (j != 7) {
        line += "  |  ";
      }
    }
    std::cout << line + "  |" << std::endl;
  }
  std::cout << dividing_line << std::endl;

  std::cout << "|   |  a  |  b  |  c  |  d  |  e  |  f  |  g  |  h  |"
            << std::endl;
  std::cout << dividing_line << std::endl;
}

uint64_t generatePlayerOccupiedBitboard(const PlayerState &player_state) {
  return player_state.pawn | player_state.rook | player_state.knight |
         player_state.bishop | player_state.queen | player_state.king;
}

uint64_t generateWhiteOccupiedBitboard(const GameState &game_state) {
  return generatePlayerOccupiedBitboard(game_state.white);
}

uint64_t generateBlackOccupiedBitboard(const GameState &game_state) {
  return generatePlayerOccupiedBitboard(game_state.black);
}

// TODO REFACTOR?
void fenToGameState(const std::string fen, GameState &gamestate) {
  std::string regex_string =
      "^ *([rnbqkpRNBQKP1-8]+/){7}([rnbqkpRNBQKP1-8]+) ([wb]) "
      "(([kqKQ]{1,4})|(-)) (([a-h][36])|(-)) *(.*)$";

  if (!regex_match(fen, std::regex(regex_string))) {
    logErrorAndExit("ERROR: Not a valid FEN.");
    return;
  }

  // Counters to validate FEN is a legal and possible position.
  // TODO: validate the castling flags and rook/king location.
  // TODO: make sure kings are separated by at least 1 square.
  // TODO: the sum of the pieces and empty squares sum to 8 per rank.
  // TODO: there should be no consecutive numbers.
  // TODO: non active color is not in check.
  // For more info:
  // https://chess.stackexchange.com/questions/1482/how-do-you-know-when-a-fen-position-is-legal

  uint8_t n_p = 0, n_r = 0, n_n = 0, n_b = 0, n_q = 0, n_k = 0, n_P = 0,
          n_R = 0, n_N = 0, n_B = 0, n_Q = 0, n_K = 0;

  uint8_t byte = 7;
  uint8_t bit = 0;

  // Populate piece positions.
  for (uint8_t i = 0; i < fen.find(' '); i++) {
    // Check for empty positions.
    if (isdigit(fen[i])) {
      bit += fen[i] - '0';
      continue;
    }
    switch (fen[i]) {
    case 'p':
      if (byte == 0 || byte == 7) {
        logErrorAndExit("ERROR: Pawns cannot be on rank 1 or rank 8.");
        memset(&gamestate, 0, sizeof(GameState));
        return;
      }
      gamestate.black.pawn += 1ull << (byte * 8 + bit);
      n_p++;
      break;
    case 'r':
      gamestate.black.rook += (1ull << (byte * 8 + bit));
      n_r++;
      break;
    case 'n':
      gamestate.black.knight += 1ull << (byte * 8 + bit);
      n_n++;
      break;
    case 'b':
      gamestate.black.bishop += 1ull << (byte * 8 + bit);
      n_b++;
      break;
    case 'q':
      gamestate.black.queen += 1ull << (byte * 8 + bit);
      n_q++;
      break;
    case 'k':
      gamestate.black.king += 1ull << (byte * 8 + bit);
      n_k++;
      break;
    case 'P':
      if (byte == 0 || byte == 7) {
        logErrorAndExit("ERROR: Pawns cannot be on rank 1 or rank 8.");
        memset(&gamestate, 0, sizeof(GameState));
        return;
      }
      gamestate.white.pawn += 1ull << (byte * 8 + bit);
      n_P++;
      break;
    case 'R':
      gamestate.white.rook += 1ull << (byte * 8 + bit);
      n_R++;
      break;
    case 'N':
      gamestate.white.knight += 1ull << (byte * 8 + bit);
      n_N++;
      break;
    case 'B':
      gamestate.white.bishop += 1ull << (byte * 8 + bit);
      n_B++;
      break;
    case 'Q':
      gamestate.white.queen += 1ull << (byte * 8 + bit);
      n_Q++;
      break;
    case 'K':
      gamestate.white.king += 1ull << (byte * 8 + bit);
      n_K++;
      break;
    case '/':
      byte -= 1;
      bit = 0;
      continue;
    default:
      logErrorAndExit("ERROR: Unknown character in FEN string!");
      break;
    }
    bit++;
  }

  // Piece count sanity checks.
  int8_t n_p_missing = 8 - n_p;
  int8_t n_P_missing = 8 - n_P;
  uint8_t n_promoted = (n_r > 2 ? n_r - 2 : 0) + (n_n > 2 ? n_n - 2 : 0) +
                       (n_b > 2 ? n_b - 2 : 0) + (n_q > 1 ? n_q - 1 : 0);
  uint8_t n_Promoted = (n_R > 2 ? n_R - 2 : 0) + (n_N > 2 ? n_N - 2 : 0) +
                       (n_B > 2 ? n_B - 2 : 0) + (n_Q > 1 ? n_Q - 1 : 0);
  if (n_P > 8 || n_p > 8 || n_K != 1 || n_k != 1 || n_p_missing < n_promoted ||
      n_P_missing < n_Promoted) {
    logErrorAndExit("ERROR: FEN position not legal/possible.");
    memset(&gamestate, 0, sizeof(GameState));
    return;
  }

  // Populate extra game state data.
  // TODO: Add half move and full move functionality. The 4th and 5th fields.
  uint8_t field = 1; // 1: turn, 2: castling flags, 3: en passant.
  for (uint8_t i = fen.find(' ') + 1; i < fen.length(); i++) {
    switch (field) {
    case 1:
      if (fen[i] == 'w') {
        gamestate.whites_turn = true;
        field++;
      } else if (fen[i] == 'b') {
        gamestate.whites_turn = false;
        field++;
      }
      continue;
    case 2:
      if (((gamestate.white.can_king_side_castle ||
            gamestate.white.can_queen_side_castle ||
            gamestate.black.can_king_side_castle ||
            gamestate.black.can_queen_side_castle) &&
           fen[i] == ' ') ||
          fen[i] == '-') {
        field++;
      } else if (fen[i] == 'K') {
        gamestate.white.can_king_side_castle = true;
      } else if (fen[i] == 'Q') {
        gamestate.white.can_queen_side_castle = true;
      } else if (fen[i] == 'k') {
        gamestate.black.can_king_side_castle = true;
      } else if (fen[i] == 'q') {
        gamestate.black.can_queen_side_castle = true;
      }
      continue;
    case 3:
      if (fen[i] >= 'a' && fen[i] <= 'h') {
        uint8_t col = fen[i] - 'a';
        uint8_t row = fen[i + 1] - '0' - 1;
        gamestate.en_passant = 1ull << (row * 8 + col);
      }
      continue;
    default:
      logErrorAndExit("ERROR: Unexpected field value.");
      break;
    }
  }
}
