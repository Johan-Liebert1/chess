#include "chess.h"
#include <stdio.h>

static inline bool Chess_is_move_available(Chess *game, Piece *piece, int row, int col) {
    // Legal moves for king is already handled
    if (!game->kingInCheck[piece->color] || piece->type == King) {
        return true;
    }

    Pos *moves = NULL;
    int num_moves = 0;

    switch (piece->color) {
        case ColorBlack: {
            moves = game->only_available_black_moves;
            num_moves = game->num_available_black_moves;
            break;
        }

        case ColorWhite: {
            moves = game->only_available_white_moves;
            num_moves = game->num_available_white_moves;
            break;
        }
    }

    for (int i = 0; i < num_moves; i++) {
        if (moves[i].row == row && moves[i].col == col)
            return true;
    }

    return false;
}

void add_move_to_piece(Chess *game, Piece *piece, int row, int col) {
    if (!Chess_is_move_available(game, piece, row, col)) {
        return;
    }

    Cell *cell = &game->board[row][col];

    piece->moves[piece->num_moves++] = (Pos){row, col};

    // Pawn's attacking moves are quite different
    if (piece->type == Pawn) {
        return;
    }

    if (cell->piece.type == UndefPieceType) {
        cell->underAttack[piece->color] = true;
        return;
    }

    if (cell->piece.color == piece->color) {
        cell->piece.is_protected = true;
        return;
    }
}

// Calculates moves of the piece that has been clicked
// and returns a pointer to that piece
// returns NULL if the clicked square does not have a piece
Piece *Chess_calculate_moves_for_piece(Chess *game, Piece *piece) {
    switch (piece->type) {
        case UndefPieceType:
            break;

        case King:
            Chess_calculate_king_moves(game, piece, 0);
            break;

        case Queen:
            Chess_calculate_rook_moves(game, piece, 0);
            Chess_calculate_bishop_moves(game, piece, piece->num_moves);
            break;

        case Rook:
            Chess_calculate_rook_moves(game, piece, 0);
            break;

        case Bishop:
            Chess_calculate_bishop_moves(game, piece, 0);
            break;

        case Knight:
            Chess_calculate_knight_moves(game, piece, 0);
            break;

        case Pawn:
            Chess_calculate_pawn_moves(game, piece, 0);
            break;
    }

    return piece->type == UndefPieceType ? NULL : piece;
}

void add_move_to_only_moves(int row, int col, Pos *move_array, int *move_idx) {
    move_array[*move_idx] = (Pos){row, col};
    (*move_idx)++;
}

void process_only_moves(Chess *game, Piece *piece, int row_adder, int col_adder, Pos *move_array, int *move_idx) {
    for (int row = piece->pos.row + row_adder, col = piece->pos.col + col_adder;
         row >= 0 && row < CHESS_BOARD_ROWS && col >= 0 && col < CHESS_BOARD_COLS; row += row_adder, col += col_adder) {

        add_move_to_only_moves(row, col, move_array, move_idx);

        if (game->board[row][col].piece.type != UndefPieceType) {
            // we break after adding the Capturing move
            break;
        }
    }
}

void handle_moves_queen_bishop_rook_check(Chess *game, Piece *king, Piece *pieceCheckingKing, Pos *move_array, int *move_idx) {
    // available moves are
    // 1. Blocking the check
    // 2. Capturing the piece, unless it's a double check

    int row_adder = 0;
    int col_adder = 0;

    if (king->pos.row == pieceCheckingKing->pos.row) {
        col_adder = (pieceCheckingKing->pos.col - king->pos.col) / abs(pieceCheckingKing->pos.col - king->pos.col);
    } else if (king->pos.col == pieceCheckingKing->pos.col) {
        row_adder = (pieceCheckingKing->pos.row - king->pos.row) / abs(pieceCheckingKing->pos.row - king->pos.row);
    } else {
        col_adder = (pieceCheckingKing->pos.col - king->pos.col) / abs(pieceCheckingKing->pos.col - king->pos.col);
        row_adder = (pieceCheckingKing->pos.row - king->pos.row) / abs(pieceCheckingKing->pos.row - king->pos.row);
    }

    process_only_moves(game, king, row_adder, col_adder, move_array, move_idx);
}

void Chess_get_available_moves_for_color(Chess *game, enum Color color, Piece *king, Piece *pieceCheckingKing) {
    (void)king;

    if (pieceCheckingKing == NULL) {
        return;
    }

    Pos *moves_array = NULL;
    int *num_moves;

    switch (color) {
        case ColorBlack: {
            if (game->only_available_black_moves == NULL) {
                game->only_available_black_moves = arena_alloc(&game->arena, 64);
            }

            game->num_available_black_moves = 0;
            moves_array = game->only_available_black_moves;
            num_moves = &game->num_available_black_moves;
            break;
        }

        case ColorWhite: {
            if (game->only_available_white_moves == NULL) {
                game->only_available_white_moves = arena_alloc(&game->arena, 64);
            }

            game->num_available_white_moves = 0;
            moves_array = game->only_available_white_moves;
            num_moves = &game->num_available_white_moves;
            break;
        }
    }

    switch (pieceCheckingKing->type) {
        case UndefPieceType: {
            assert(false && "pieceCheckingKing is not an actual piece");
        }

        case King: {
            assert(false && "King cannot check another king");
        }

        case Queen:
        case Rook:
        case Bishop: {
            handle_moves_queen_bishop_rook_check(game, king, pieceCheckingKing, moves_array, num_moves);
            break;
        }

        // If a Knight is checking a king, the only moves are to move the king or take the Knight
        case Knight:
        case Pawn: {
            add_move_to_only_moves(pieceCheckingKing->pos.row, pieceCheckingKing->pos.col, moves_array, num_moves);
            break;
        }
    }
}

void process_moves(Chess *game, Piece *piece, int row_adder, int col_adder) {
    for (int row = piece->pos.row + row_adder, col = piece->pos.col + col_adder;
         row >= 0 && row < CHESS_BOARD_ROWS && col >= 0 && col < CHESS_BOARD_COLS; row += row_adder, col += col_adder) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            add_move_to_piece(game, piece, row, col);
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            add_move_to_piece(game, piece, row, col);
        }

        break;
    }
}

void Chess_calculate_moves(Chess *chess) {
    Piece *whiteKing = NULL;
    Piece *blackKing = NULL;

    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            chess->board[row][col].underAttack[0] = false;
            chess->board[row][col].underAttack[1] = false;

            chess->board[row][col].piece.is_protected = false;

            if (Chess_is_piece(&chess->board[row][col].piece, King, ColorWhite)) {
                whiteKing = &chess->board[row][col].piece;
            }

            if (Chess_is_piece(&chess->board[row][col].piece, King, ColorBlack)) {
                blackKing = &chess->board[row][col].piece;
            }
        }
    }

    chess->kingInCheck[0] = NULL;
    chess->kingInCheck[1] = NULL;

    Chess_check_for_checks_after_move(chess, whiteKing);
    Chess_check_for_checks_after_move(chess, blackKing);

    if (chess->kingInCheck[ColorBlack] != NULL) {
        Chess_get_available_moves_for_color(chess, ColorBlack, blackKing, chess->kingInCheck[ColorBlack]);
    }

    if (chess->kingInCheck[ColorWhite] != NULL) {
        Chess_get_available_moves_for_color(chess, ColorWhite, whiteKing, chess->kingInCheck[ColorWhite]);
    }

    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            // calculate king moves at the end as we need to take into account
            // all attacked squares
            if (chess->board[row][col].piece.type != King) {
                Chess_calculate_moves_for_piece(chess, &chess->board[row][col].piece);
            }
        }
    }

    Chess_calculate_moves_for_piece(chess, whiteKing);
    Chess_calculate_moves_for_piece(chess, blackKing);
}

void swap_pieces(Cell *move_from, Cell *move_to, Chess *game) {
    move_to->piece = PUT_PIECE(game->board, move_to->piece.pos.row, move_to->piece.pos.col, move_from->piece.type, move_from->piece.color,
                               move_from->piece.sprite_number);

    move_to->piece.has_moved = true;
    move_to->piece.moves = move_from->piece.moves;

    move_from->piece = (Piece){.pos = move_from->piece.pos};
}

// returns whether the move was legal or not
bool Chess_make_move(Chess *game, Piece *piece, Pos pos) {
    Cell *move_from = &game->board[piece->pos.row][piece->pos.col];
    Cell *move_to = &game->board[pos.row][pos.col];

    bool legal = false;
    bool is_castling = false;

    for (int i = 0; i < piece->num_moves; i++) {
        if (piece->moves[i].row == pos.row && piece->moves[i].col == pos.col) {
            is_castling = piece->type == King && abs(pos.col - piece->pos.col) == 2;

            swap_pieces(move_from, move_to, game);

            legal = true;
        }
    }

    if (legal) {
        if (is_castling) {
            // O-O-O
            if (piece->pos.col > pos.col) {
                swap_pieces(&game->board[pos.row][0], &game->board[pos.row][pos.col + 1], game);
            } else {
                // O-O
                swap_pieces(&game->board[pos.row][CHESS_BOARD_COLS - 1], &game->board[pos.row][pos.col - 1], game);
            }
        }

        Chess_calculate_moves(game);
        game->current_turn = 1 - game->current_turn;
    }

    return legal;
}
