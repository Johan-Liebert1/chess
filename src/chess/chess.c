#include "chess.h"
#include <assert.h>
#include <stdio.h>

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


Piece *Chess_find_piece(Chess *game, enum PieceType type, enum Color pieceColor) {
    for (int i = 0; i < CHESS_BOARD_ROWS; i++) {
        for (int j = 0; j < CHESS_BOARD_ROWS; j++) {
            if (game->board[i][j].piece.type == type && game->board[i][j].piece.color == pieceColor) {
                return &game->board[i][j].piece;
            }
        }
    }

    return NULL;
}

bool Chess_is_piece(Piece *piece, enum PieceType type, enum Color pieceColor) { return piece->color == pieceColor && piece->type == type; }

void Chess_init_board(Chess *chess) {
    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            if ((row + col) % 2 == 0) {
                chess->board[row][col].color = ColorWhite;
            }
            chess->board[row][col].piece.pos.row = row;
            chess->board[row][col].piece.pos.col = col;
        }
    }

    PUT_PIECE(chess->board, 0, 0, Rook, ColorWhite, 4);
    PUT_PIECE(chess->board, 0, 1, Knight, ColorWhite, 3);
    PUT_PIECE(chess->board, 0, 2, Bishop, ColorWhite, 2);
    PUT_PIECE(chess->board, 0, 3, Queen, ColorWhite, 1);
    PUT_PIECE(chess->board, 0, 4, King, ColorWhite, 0);
    PUT_PIECE(chess->board, 0, 5, Bishop, ColorWhite, 2);
    PUT_PIECE(chess->board, 0, 6, Knight, ColorWhite, 3);
    PUT_PIECE(chess->board, 0, 7, Rook, ColorWhite, 4);

    PUT_PIECE(chess->board, 7, 0, Rook, ColorBlack, 10);
    PUT_PIECE(chess->board, 7, 1, Knight, ColorBlack, 9);
    PUT_PIECE(chess->board, 7, 2, Bishop, ColorBlack, 8);
    PUT_PIECE(chess->board, 7, 3, Queen, ColorBlack, 7);
    PUT_PIECE(chess->board, 7, 4, King, ColorBlack, 6);
    PUT_PIECE(chess->board, 7, 5, Bishop, ColorBlack, 8);
    PUT_PIECE(chess->board, 7, 6, Knight, ColorBlack, 9);
    PUT_PIECE(chess->board, 7, 7, Rook, ColorBlack, 10);

    for (int i = 0; i < CHESS_BOARD_COLS; i++) {
        PUT_PIECE(chess->board, 1, i, Pawn, ColorWhite, 5);
        PUT_PIECE(chess->board, 6, i, Pawn, ColorBlack, 11);
    }
}

// returns whether the move was legal or not
bool Chess_make_move(Chess *game, Piece *piece, Pos pos) {
    Cell *move_from = &game->board[piece->pos.row][piece->pos.col];
    Cell *move_to = &game->board[pos.row][pos.col];

    bool legal = false;

    for (int i = 0; i < piece->num_moves; i++) {
        if (piece->moves[i].row == pos.row && piece->moves[i].col == pos.col) {
            move_to->piece = PUT_PIECE(game->board, pos.row, pos.col, move_from->piece.type, move_from->piece.color, move_from->piece.sprite_number);
            move_to->piece.has_moved = true;
            move_to->piece.moves = move_from->piece.moves;

            move_from->piece = (Piece){0};
            legal = true;
        }
    }

    if (legal) {
        // Chess_check_for_checks_after_move(game, &move_to->piece);
        Chess_calculate_moves(game);
    }

    return legal;
}
