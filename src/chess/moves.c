#include "chess.h"

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
        if (piece->type == Pawn)
            printf("move (%d, %d) is not available\n", row, col);
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

    if (cell->piece.type == King) {
        game->kingInCheck[1 - piece->color] = piece;
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

void Chess_get_available_moves_for_color(Chess *game, enum Color color, Piece *king, Piece *pieceCheckingKing) {
    (void)king;

    Pos *moves = NULL;
    int *move_num;

    switch (color) {
        case ColorBlack: {
            if (game->only_available_black_moves == NULL) {
                game->only_available_black_moves = arena_alloc(&game->arena, 64);
            }

            game->num_available_black_moves = 0;
            moves = game->only_available_black_moves;
            move_num = &game->num_available_black_moves;
            break;
        }

        case ColorWhite: {
            if (game->only_available_white_moves == NULL) {
                game->only_available_white_moves = arena_alloc(&game->arena, 64);
            }

            game->num_available_white_moves = 0;
            moves = game->only_available_white_moves;
            move_num = &game->num_available_white_moves;
            break;
        }
    }

    printf("Piece checking king\n");
    print_piece(pieceCheckingKing);

    switch (pieceCheckingKing->type) {
        case UndefPieceType: {
            assert(false && "pieceCheckingKing is not an actual piece");
        }

        case King: {
            assert(false && "King cannot check another king");
        }

        case Queen: {
            break;
        }

        case Rook: {
            break;
        }

        case Bishop: {
            break;
        }

        // If a Knight is checking a king, the only moves are to move the king or take the Knight
        case Knight: {
            add_move_to_only_moves(pieceCheckingKing->pos.row, pieceCheckingKing->pos.col, moves, move_num);
            break;
        }

        case Pawn: {
            add_move_to_only_moves(pieceCheckingKing->pos.row, pieceCheckingKing->pos.col, moves, move_num);
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
            Chess_calculate_moves_for_piece(chess, &chess->board[row][col].piece);
        }
    }
}
