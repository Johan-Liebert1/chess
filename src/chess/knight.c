#include "chess.h"

static int KnightRows[4] = {2, 1, -1, -2};
static int KnightCols[4][2] = {{-1, 1}, {-2, 2}, {-2, 2}, {-1, 1}};
static int KnightRowsLen = sizeof(KnightRows) / sizeof(KnightRows[0]);
static int KnightColsLen = sizeof(KnightCols[0]) / sizeof(KnightCols[0][0]);

// num_moves -> for consistancy
void Chess_calculate_knight_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    for (int row_idx = 0; row_idx < KnightRowsLen; row_idx++) {
        for (int col_idx = 0; col_idx < KnightColsLen; col_idx++) {
            int row_adder = KnightRows[row_idx];
            int col_adder = KnightCols[row_idx][col_idx];

            Pos potential_move = (Pos){.row = piece->pos.row + row_adder, .col = piece->pos.col + col_adder};

            if (pos_within_bounds(potential_move.row, potential_move.col)) {
                Cell cell = game->board[potential_move.row][potential_move.col];

                if (cell.piece.type == UndefPieceType || cell.piece.color != piece->color) {
                    add_move_to_piece(game, piece, potential_move.row, potential_move.col);
                }
            }
        }
    }
}
