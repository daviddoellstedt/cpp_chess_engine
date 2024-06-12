#include "board.h"
#include "helper_functions.h"
#include "move.h"
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

// TODO REFACTOR?
void fenToGameState(const std::string fen, GameState &game_state) {
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
        memset(&game_state, 0, sizeof(GameState));
        return;
      }
      game_state.black.pawn += 1ull << (byte * 8 + bit);
      n_p++;
      break;
    case 'r':
      game_state.black.rook += (1ull << (byte * 8 + bit));
      n_r++;
      break;
    case 'n':
      game_state.black.knight += 1ull << (byte * 8 + bit);
      n_n++;
      break;
    case 'b':
      game_state.black.bishop += 1ull << (byte * 8 + bit);
      n_b++;
      break;
    case 'q':
      game_state.black.queen += 1ull << (byte * 8 + bit);
      n_q++;
      break;
    case 'k':
      game_state.black.king += 1ull << (byte * 8 + bit);
      n_k++;
      break;
    case 'P':
      if (byte == 0 || byte == 7) {
        logErrorAndExit("ERROR: Pawns cannot be on rank 1 or rank 8.");
        memset(&game_state, 0, sizeof(GameState));
        return;
      }
      game_state.white.pawn += 1ull << (byte * 8 + bit);
      n_P++;
      break;
    case 'R':
      game_state.white.rook += 1ull << (byte * 8 + bit);
      n_R++;
      break;
    case 'N':
      game_state.white.knight += 1ull << (byte * 8 + bit);
      n_N++;
      break;
    case 'B':
      game_state.white.bishop += 1ull << (byte * 8 + bit);
      n_B++;
      break;
    case 'Q':
      game_state.white.queen += 1ull << (byte * 8 + bit);
      n_Q++;
      break;
    case 'K':
      game_state.white.king += 1ull << (byte * 8 + bit);
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
    memset(&game_state, 0, sizeof(GameState));
    return;
  }

  // Populate extra game state data.
  // TODO: Add half move and full move functionality. The 4th and 5th fields.
  uint8_t field = 1; // 1: turn, 2: castling flags, 3: en passant.
  for (uint8_t i = fen.find(' ') + 1; i < fen.length(); i++) {
    switch (field) {
    case 1:
      if (fen[i] == 'w') {
        game_state.whites_turn = true;
        field++;
      } else if (fen[i] == 'b') {
        game_state.whites_turn = false;
        field++;
      }
      continue;
    case 2:
      if (((game_state.white.can_king_side_castle ||
            game_state.white.can_queen_side_castle ||
            game_state.black.can_king_side_castle ||
            game_state.black.can_queen_side_castle) &&
           fen[i] == ' ') ||
          fen[i] == '-') {
        field++;
      } else if (fen[i] == 'K') {
        game_state.white.can_king_side_castle = true;
      } else if (fen[i] == 'Q') {
        game_state.white.can_queen_side_castle = true;
      } else if (fen[i] == 'k') {
        game_state.black.can_king_side_castle = true;
      } else if (fen[i] == 'q') {
        game_state.black.can_queen_side_castle = true;
      }
      continue;
    case 3:
      if (fen[i] >= 'a' && fen[i] <= 'h') {
        uint8_t col = fen[i] - 'a';
        uint8_t row = fen[i + 1] - '0' - 1;
        game_state.en_passant = 1ull << (row * 8 + col);
      }
      continue;
    default:
      logErrorAndExit("ERROR: Unexpected field value.");
      break;
    }
  }
}

/** Checks for captured pieces and updates the enemy player state accordingly.
 *
 * @param white_to_move: Flag that denotes turn.
 * @param P: Active player's pawn bitboard.
 * @param enemy_player: Enemy player's state.
 * @param E_P: Bitboard with the en passant bit set (if applicable).
 * @param initial: Bitboard of the moving piece, pre-move.
 * @param final: Bitboard of the moving piece, post-move.
 */
void handleCapturedPiece(bool white_to_move, uint64_t P,
                         ColorState &enemy_player, uint64_t E_P,
                         uint64_t initial, uint64_t final) {
  uint64_t ENEMY_PIECES = enemy_player.getOccupiedBitboard();

  if (ENEMY_PIECES & final) {
    if (enemy_player.pawn & final) {
      enemy_player.pawn &= ~final;
      return;
    }
    if (enemy_player.knight & final) {
      enemy_player.knight &= ~final;
      return;
    }
    if (enemy_player.bishop & final) {
      enemy_player.bishop &= ~final;
      return;
    }
    if (enemy_player.queen & final) {
      enemy_player.queen &= ~final;
      return;
    }
    if (enemy_player.rook & final) {
      enemy_player.rook &= ~final;
      if (final & (white_to_move ? BLACK_ROOK_STARTING_POSITION_KINGSIDE
                                 : WHITE_ROOK_STARTING_POSITION_KINGSIDE)) {
        enemy_player.can_king_side_castle = false;
      } else if (final &
                 (white_to_move ? BLACK_ROOK_STARTING_POSITION_QUEENSIDE
                                : WHITE_ROOK_STARTING_POSITION_QUEENSIDE)) {
        enemy_player.can_queen_side_castle = false;
      }
      return;
    }
  }
  if ((E_P & final) && (P & initial)) {
    enemy_player.pawn &= (white_to_move ? ~(final >> 8) : ~(final << 8));
    return;
  }
}

/** Updates the game state with the respective moved piece.
 *
 * @param white_to_move: Flag that denotes turn.
 * @param active_player: Active player's state.
 * @param E_P: Bitboard with the en passant bit set (if applicable).
 * @param initial: Bitboard of the moving piece, pre-move.
 * @param final: Bitboard of the moving piece, post-move.
 * @param special: Special move type, if applicable.
 */
void realizeMovedPiece(bool white_to_move, ColorState &active_player,
                       uint64_t &E_P, uint64_t initial, uint64_t final,
                       SpecialMove special) {
  switch (special) {
  case NONE:
    if (active_player.queen & initial) {
      active_player.queen |= final;
      active_player.queen &= ~initial;
    } else if (active_player.bishop & initial) {
      active_player.bishop |= final;
      active_player.bishop &= ~initial;
    } else if (active_player.knight & initial) {
      active_player.knight |= final;
      active_player.knight &= ~initial;
    } else if (active_player.pawn & initial) {
      active_player.pawn |= final;
      active_player.pawn &= ~initial;
    } else if (active_player.king & initial) {
      active_player.can_king_side_castle = false;
      active_player.can_queen_side_castle = false;
      active_player.king = final;
    } else if (active_player.rook & initial) {
      if (initial & (white_to_move ? WHITE_ROOK_STARTING_POSITION_KINGSIDE
                                   : BLACK_ROOK_STARTING_POSITION_KINGSIDE)) {
        active_player.can_king_side_castle = false;
      } else if (initial &
                 (white_to_move ? WHITE_ROOK_STARTING_POSITION_QUEENSIDE
                                : BLACK_ROOK_STARTING_POSITION_QUEENSIDE)) {
        active_player.can_queen_side_castle = false;
      }
      active_player.rook |= final;
      active_player.rook &= ~initial;
    }
    E_P = 0;
    return;
  case CASTLE_KINGSIDE:
    active_player.king <<= 2;
    active_player.rook |= white_to_move
                              ? WHITE_ROOK_POST_KINGSIDE_CASTLE_POSITION
                              : BLACK_ROOK_POST_KINGSIDE_CASTLE_POSITION;
    active_player.rook &= white_to_move
                              ? ~WHITE_ROOK_STARTING_POSITION_KINGSIDE
                              : ~BLACK_ROOK_STARTING_POSITION_KINGSIDE;
    active_player.can_king_side_castle = false;
    active_player.can_queen_side_castle = false;
    E_P = 0;
    return;
  case CASTLE_QUEENSIDE:
    active_player.king >>= 2;
    active_player.rook |= white_to_move
                              ? WHITE_ROOK_POST_QUEENSIDE_CASTLE_POSITION
                              : BLACK_ROOK_POST_QUEENSIDE_CASTLE_POSITION;
    active_player.rook &= white_to_move
                              ? ~WHITE_ROOK_STARTING_POSITION_QUEENSIDE
                              : ~BLACK_ROOK_STARTING_POSITION_QUEENSIDE;
    active_player.can_king_side_castle = false;
    active_player.can_queen_side_castle = false;
    E_P = 0;
    return;
  case PROMOTION_QUEEN:
    active_player.pawn &= ~initial;
    active_player.queen |= final;
    E_P = 0;
    return;
  case PROMOTION_ROOK:
    active_player.pawn &= ~initial;
    active_player.rook |= final;
    E_P = 0;
    return;
  case PROMOTION_BISHOP:
    active_player.pawn &= ~initial;
    active_player.bishop |= final;
    E_P = 0;
    return;
  case PROMOTION_KNIGHT:
    active_player.pawn &= ~initial;
    active_player.knight |= final;
    E_P = 0;
    return;
  case EN_PASSANT:
    active_player.pawn |= final;
    active_player.pawn &= ~initial;
    E_P = 0;
    return;
  case PAWN_PUSH_2:
    active_player.pawn |= final;
    active_player.pawn &= ~initial;
    E_P = white_to_move ? final >> 8 : final << 8;
    return;
  default:
    logErrorAndExit("ERROR: Unexpected SpecialMove value!");
    return;
  }
}

/** Applies the white player's move.
 *
 * @param game_state: Game state.
 * @param move: Move.
 * @param initial: Bitboard of the moving piece, pre-move.
 * @param final: Bitboard of the moving piece, post-move.
 * @param special: Special move type, if applicable.
 */
void applyWhiteMove(GameState &game_state, const Move &move, uint64_t initial,
                    uint64_t final, SpecialMove special) {
  handleCapturedPiece(game_state.whites_turn, game_state.white.pawn,
                      game_state.black, game_state.en_passant, initial, final);
  realizeMovedPiece(game_state.whites_turn, game_state.white,
                    game_state.en_passant, initial, final, special);
}

/** Applies the black player's move.
 *
 * @param game_state: Game state.
 * @param move: Move.
 * @param initial: Bitboard of the moving piece, pre-move.
 * @param final: Bitboard of the moving piece, post-move.
 * @param special: Special move type, if applicable.
 */
void applyBlackMove(GameState &game_state, const Move &move, uint64_t initial,
                    uint64_t final, SpecialMove special) {
  handleCapturedPiece(game_state.whites_turn, game_state.black.pawn,
                      game_state.white, game_state.en_passant, initial, final);
  realizeMovedPiece(game_state.whites_turn, game_state.black,
                    game_state.en_passant, initial, final, special);
}

void applyMove(Move move, GameState &game_state) {
  const uint64_t initial = move.getInitialBitboard();
  const uint64_t final = move.getFinalBitboard();
  const SpecialMove special = move.getSpecial();
  if (game_state.whites_turn) {
    applyWhiteMove(game_state, move, initial, final, special);
  } else {
    applyBlackMove(game_state, move, initial, final, special);
  }
  game_state.whites_turn = !game_state.whites_turn;
}
