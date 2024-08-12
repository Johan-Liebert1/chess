#include "chess.h"

void Chess_calculate_pawn_moves(Chess *game, Piece *piece, int num_moves) {
    // TODO: Handle en-passant and promotion
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    // White is always at the top. Even when rotating the board, we only display it upside down
    int row_adder = piece->color == ColorWhite ? 1 : -1;

    int row = piece->pos.row + row_adder;
    int col = piece->pos.col;

    if (is_cell_empty(&game->board, row, col)) {
        add_move_to_piece(game, piece, row, col);
    }

    row = piece->pos.row + row_adder * 2;
    if (!piece->has_moved && is_cell_empty(&game->board, row, col)) {
        add_move_to_piece(game, piece, row, col);
    }

    // Capture
    int col_adder[2] = {-1, 1};
    row = piece->pos.row + row_adder;

    for (int i = 0; i < 2; i++) {
        col = piece->pos.col + col_adder[i];

        if (can_piece_capture(&game->board, piece, row, col)) {
            add_move_to_piece(game, piece, row, col);
        }

        if (pos_within_bounds(row, col)) {
            Cell *cell = &game->board[row][col];

            if (cell->piece.type == UndefPieceType) {
                cell->underAttack[piece->color] = true;
            } else if (cell->piece.color == piece->color) {
                cell->piece.is_protected = true;
            }
        }
    }
}

