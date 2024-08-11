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
    Black,
    White,
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
    // idx 0 = 1 -> under attack by white
    // idx 1 = 1 -> under attack by black
    bool underAttack[2];
};
typedef struct _Cell Cell;

typedef Cell ChessBoard[CHESS_BOARD_ROWS][CHESS_BOARD_COLS];

struct _Chess {
    ChessBoard board;
    Arena arena;
    Piece *clicked_piece;
};
typedef struct _Chess Chess;

static inline bool pos_within_bounds(int row, int col) { return row >= 0 && row < CHESS_BOARD_ROWS && col >= 0 && col < CHESS_BOARD_COLS; }

const char* piece_type_diplay(enum PieceType type);
Piece * Chess_calculate_moves_for_piece(Chess *game, Pos pos);
void Chess_calculate_moves(Chess *game);
void Chess_make_move(Chess *game, Piece *piece, Pos pos);
void Chess_init_board(Chess *chess);

#endif // !CHESS__
