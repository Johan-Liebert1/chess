#include "chess.h"
#include <stdio.h>

// clang-format off
static int directions[8][2] = {
    {-1, 0} ,
    {0 , 1} ,
    {1 , 0} ,
    {0 , -1},
    {-1, -1},
    {-1, 1} ,
    {1 , 1} ,
    {-1, -1}
};
// clang-format on

static int KingRows[3] = {1, -1, 0};
static int KingCols[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
static int KingRowsLen = sizeof(KingRows) / sizeof(KingRows[0]);
static int KingColsLen = sizeof(KingCols[0]) / sizeof(KingCols[0][0]);

void Chess_calculate_king_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    for (int row_idx = 0; row_idx < KingRowsLen; row_idx++) {
        for (int col_idx = 0; col_idx < KingColsLen; col_idx++) {
            int row_adder = KingRows[row_idx];
            int col_adder = KingCols[row_idx][col_idx];

            Pos potential_move = (Pos){.row = piece->pos.row + row_adder, .col = piece->pos.col + col_adder};

            if (pos_within_bounds(potential_move.row, potential_move.col)) {
                Cell cell = game->board[potential_move.row][potential_move.col];

                if ((cell.piece.type == UndefPieceType && !cell.underAttack[1 - piece->color]) ||
                    (cell.piece.type != UndefPieceType && cell.piece.color != piece->color && !cell.piece.is_protected)) {
                    add_move_to_piece(game, piece, potential_move.row, potential_move.col);
                }
            }
        }
    }

    // Castling
    if (piece->has_moved)
        return;

    // O-O
    for (int col = piece->pos.col + 1; col < CHESS_BOARD_COLS; col++) {
        Cell cell = game->board[piece->pos.row][col];

        // Can't castle through a check
        if (cell.underAttack[1 - piece->color] && col <= piece->pos.col + 2) {
            break;
        }

        if (cell.piece.type == UndefPieceType)
            continue;

        if (cell.piece.type != Rook) {
            break;
        } else {
            // rook has moved, can't castle
            if (cell.piece.has_moved) {
                break;
            }

            add_move_to_piece(game, piece, piece->pos.row, piece->pos.col + 2);
        }
    }

    // O-O-O
    for (int col = piece->pos.col - 1; col >= 0; col--) {
        Cell cell = game->board[piece->pos.row][col];

        // Can't castle through a check
        if (cell.underAttack[1 - piece->color] && col >= piece->pos.col - 2) {
            break;
        }

        if (cell.piece.type == UndefPieceType) {
            continue;
        }

        if (cell.piece.type != Rook) {
            break;
        } else {
            // rook has moved, can't castle
            if (cell.piece.has_moved) {
                break;
            }

            add_move_to_piece(game, piece, piece->pos.row, piece->pos.col - 2);
        }
    }
}

// Handle double checks
void Chess_rook_bishop_queen_check_check(Chess *game, Piece *king, int row_adder, int col_adder) {
    for (int row = king->pos.row + row_adder, col = king->pos.col + col_adder;
         row >= 0 && row < CHESS_BOARD_ROWS && col >= 0 && col < CHESS_BOARD_COLS; row += row_adder, col += col_adder) {
        Cell *cell = &game->board[row][col];

        switch (cell->piece.type) {
            case UndefPieceType: {
                continue;
            }

            case Rook:
            case Bishop:
            case Queen: {
                if (cell->piece.color != king->color) {
                    game->kingInCheck[king->color] = &cell->piece;
                    return;
                }

                return;
            }

            default:
                return;
        };
    }
}

void Chess_knight_check_check(Chess *game, Piece *king) {
    for (int row_idx = 0; row_idx < KnightRowsLen; row_idx++) {
        for (int col_idx = 0; col_idx < KnightColsLen; col_idx++) {
            int row_adder = KnightRows[row_idx];
            int col_adder = KnightCols[row_idx][col_idx];

            Pos potential_check = (Pos){.row = king->pos.row + row_adder, .col = king->pos.col + col_adder};

            if (pos_within_bounds(potential_check.row, potential_check.col)) {
                Cell *cell = &game->board[potential_check.row][potential_check.col];

                if (cell->piece.type == Knight && cell->piece.color != king->color) {
                    game->kingInCheck[king->color] = &cell->piece;
                    return;
                }
            }
        }
    }
}

void Chess_check_for_checks_after_move(Chess *chess, Piece *king) {
    for (int i = 0; i < (int)(sizeof(directions) / sizeof(directions[0])); i++) {
        Chess_rook_bishop_queen_check_check(chess, king, directions[i][0], directions[i][1]);
    }

    Chess_knight_check_check(chess, king);
}
