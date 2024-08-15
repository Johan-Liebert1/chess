#include "chess.h"
#include <assert.h>
#include <stdio.h>

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

    int white_pawn_row = chess->white_at_bottom ? CHESS_BOARD_ROWS - 1 : 0;

    PUT_PIECE(chess->board, white_pawn_row, 0, Rook, ColorWhite, 4);
    PUT_PIECE(chess->board, white_pawn_row, 1, Knight, ColorWhite, 3);
    PUT_PIECE(chess->board, white_pawn_row, 2, Bishop, ColorWhite, 2);
    PUT_PIECE(chess->board, white_pawn_row, 3, Queen, ColorWhite, 1);
    PUT_PIECE(chess->board, white_pawn_row, 4, King, ColorWhite, 0);
    PUT_PIECE(chess->board, white_pawn_row, 5, Bishop, ColorWhite, 2);
    PUT_PIECE(chess->board, white_pawn_row, 6, Knight, ColorWhite, 3);
    PUT_PIECE(chess->board, white_pawn_row, 7, Rook, ColorWhite, 4);

    int black_pawn_row = chess->white_at_bottom ? 0 : CHESS_BOARD_ROWS - 1;

    PUT_PIECE(chess->board, black_pawn_row, 0, Rook, ColorBlack, 10);
    PUT_PIECE(chess->board, black_pawn_row, 1, Knight, ColorBlack, 9);
    PUT_PIECE(chess->board, black_pawn_row, 2, Bishop, ColorBlack, 8);
    PUT_PIECE(chess->board, black_pawn_row, 3, Queen, ColorBlack, 7);
    PUT_PIECE(chess->board, black_pawn_row, 4, King, ColorBlack, 6);
    PUT_PIECE(chess->board, black_pawn_row, 5, Bishop, ColorBlack, 8);
    PUT_PIECE(chess->board, black_pawn_row, 6, Knight, ColorBlack, 9);
    PUT_PIECE(chess->board, black_pawn_row, 7, Rook, ColorBlack, 10);

    white_pawn_row = chess->white_at_bottom ? white_pawn_row - 1 : white_pawn_row + 1;
    black_pawn_row = chess->white_at_bottom ? black_pawn_row + 1 : black_pawn_row - 1;

    for (int i = 0; i < CHESS_BOARD_COLS; i++) {
        PUT_PIECE(chess->board, white_pawn_row, i, Pawn, ColorWhite, 5);
        PUT_PIECE(chess->board, black_pawn_row, i, Pawn, ColorBlack, 11);
    }
}
