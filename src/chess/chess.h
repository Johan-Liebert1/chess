#include "../arena.h"
#include <SDL2/SDL_rect.h>
#include <assert.h>
#include <stdbool.h>

#ifndef CHESS__
#define CHESS__

#define SPRITE_SHEET_WIDTH 800
#define SPRITE_SHEET_HEIGHT 267

#define SPRITE_SHEET_ROWS 2
#define SPRITE_SHEET_COLS 6

#define SPRITE_WIDTH (SPRITE_SHEET_WIDTH / SPRITE_SHEET_COLS)
#define SPRITE_HEIGHT (SPRITE_SHEET_HEIGHT / SPRITE_SHEET_ROWS)

#define CHESS_BOARD_ROWS 8
#define CHESS_BOARD_COLS 8

enum PieceType { UndefPieceType, King, Queen, Rook, Bishop, Knight, Pawn };

enum Color {
    ColorBlack,
    ColorWhite,
};

struct _Vec2 {
    int x;
    int y;
};
typedef struct _Vec2 Vec2;

struct _Pos {
    int row;
    int col;
};
typedef struct _Pos Pos;

struct _Piece {
    Pos pos;
    SDL_Rect sprite_loc;
    enum PieceType type;
    enum Color color;
    bool has_moved;
    bool is_protected;
    uint8_t num_moves;
    // it's 2024, memory is cheap, screw it
    Pos *moves;
    int sprite_number;
};
typedef struct _Piece Piece;

struct _Cell {
    Piece piece;
    enum Color color;
    // idx 0 = 1 -> under attack by black
    // idx 1 = 1 -> under attack by white
    bool underAttack[2];
};
typedef struct _Cell Cell;

typedef Cell ChessBoard[CHESS_BOARD_ROWS][CHESS_BOARD_COLS];

struct _Chess {
    ChessBoard board;
    Arena arena;
    Piece *clicked_piece;

    int num_available_white_moves;
    Pos *only_available_white_moves;

    int num_available_black_moves;
    Pos *only_available_black_moves;

    // idx 0 != NULL -> black king in check
    // idx 1 != NULL -> white king in check
    Piece *kingInCheck[2];

    // if in game_mode, only then the current_turn is taken into account
    bool game_mode;
    enum Color current_turn;

    bool white_at_bottom;
};
typedef struct _Chess Chess;

#define PUT_PIECE(board, row_val, col_val, piece_type, color_val, sprite_number_val)                                                                 \
    board[row_val][col_val].piece = (Piece) {                                                                                                        \
        .pos = {.row = row_val, .col = col_val}, .type = piece_type, .color = color_val,                                                             \
        .sprite_loc =                                                                                                                                \
            (SDL_Rect){.x = (sprite_number_val < SPRITE_SHEET_COLS ? sprite_number_val : sprite_number_val % SPRITE_SHEET_COLS) * SPRITE_WIDTH,      \
                       .y = sprite_number_val < SPRITE_SHEET_COLS ? 0 : SPRITE_HEIGHT,                                                               \
                       .w = SPRITE_WIDTH,                                                                                                            \
                       .h = SPRITE_HEIGHT},                                                                                                          \
        .sprite_number = sprite_number_val                                                                                                           \
    }


extern int KnightRows[4];
extern int KnightCols[4][2];
extern int KnightRowsLen;
extern int KnightColsLen;

static inline bool pos_within_bounds(int row, int col) { return row >= 0 && row < CHESS_BOARD_ROWS && col >= 0 && col < CHESS_BOARD_COLS; }

const char *piece_type_diplay(enum PieceType type);
Piece *Chess_calculate_moves_for_piece(Chess *game, Piece *piece);
Piece *Chess_find_piece(Chess *game, enum PieceType type, enum Color pieceColor);
bool Chess_is_piece(Piece *piece, enum PieceType type, enum Color pieceColor);
void Chess_calculate_moves(Chess *game);
bool Chess_make_move(Chess *game, Piece *piece, Pos pos);
void Chess_init_board(Chess *chess);

void Chess_calculate_king_moves(Chess *game, Piece *piece, int num_moves);
void Chess_calculate_knight_moves(Chess *game, Piece *piece, int num_moves);
void Chess_calculate_rook_moves(Chess *game, Piece *piece, int num_moves);
void Chess_calculate_bishop_moves(Chess *game, Piece *piece, int num_moves);
void Chess_calculate_pawn_moves(Chess *game, Piece *piece, int num_moves);

void process_moves(Chess *game, Piece *piece, int row_adder, int col_adder);
void add_move_to_piece(Chess *game, Piece *piece, int row, int col);

void Chess_check_for_checks_after_move(Chess *chess, Piece *king);

bool is_cell_empty(ChessBoard *board, int row, int col);
const char *color_diplay(enum Color color);
const char *piece_type_diplay(enum PieceType type) ;
void print_piece(Piece *piece);
bool can_piece_capture(ChessBoard *board, Piece *piece, int row, int col);

#endif // !CHESS__
