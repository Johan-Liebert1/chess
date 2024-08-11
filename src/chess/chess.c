#include "chess.h"
#include <assert.h>
#include <stdio.h>

static int KnightRows[4] = {2, 1, -1, -2};
static int KnightCols[4][2] = {{-1, 1}, {-2, 2}, {-2, 2}, {-1, 1}};
static int KnightRowsLen = sizeof(KnightRows) / sizeof(KnightRows[0]);
static int KnightColsLen = sizeof(KnightCols[0]) / sizeof(KnightCols[0][0]);

static int KingRows[3] = {1, -1, 0};
static int KingCols[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
static int KingRowsLen = sizeof(KingRows) / sizeof(KingRows[0]);
static int KingColsLen = sizeof(KingCols[0]) / sizeof(KingCols[0][0]);

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

const char *color_diplay(enum Color color) {
    switch (color) {
        case Black:
            return "Black";
        case White:
            return "White";
    }

    assert(false && "Unknown color");
}

const char *piece_type_diplay(enum PieceType type) {
    switch (type) {
        case UndefPieceType:
            return "UndefPieceType";
        case King:
            return "King";
        case Queen:
            return "Queen";
        case Rook:
            return "Rook";
        case Bishop:
            return "Bishop";
        case Knight:
            return "Knight";
        case Pawn:
            return "Pawn";
    }

    assert(false && "Unknown piece type");
}

void print_piece(Piece *piece) {
    printf("{ pos: (%d, %d), PieceType: %s, color: %s, has_moved: %d, is_protected: %d, piece_moves: %p }\n", piece->pos.row, piece->pos.col,
           piece_type_diplay(piece->type), color_diplay(piece->color), piece->has_moved, piece->is_protected, (void *)piece->moves);
}

static inline bool is_cell_empty(ChessBoard *board, int row, int col) {
    return pos_within_bounds(row, col) && (*board)[row][col].piece.type == UndefPieceType;
}

// Only cheks whether piece can capture other piece at (row, col). Does not check whether (row, col)
// is a valid move
static inline bool can_piece_capture(ChessBoard *board, Piece *piece, int row, int col) {
    if (!pos_within_bounds(row, col)) {
        return false;
    }

    Piece *other = &(*board)[row][col].piece;

    return other->type != UndefPieceType && other->color != piece->color;
}

void add_move_to_piece(Chess *game, Piece *piece, int row, int col) {
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

void Chess_calculate_moves(Chess *chess) {
    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            chess->board[row][col].underAttack[0] = false;
            chess->board[row][col].underAttack[1] = false;

            chess->board[row][col].piece.is_protected = false;
        }
    }

    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            Chess_calculate_moves_for_piece(chess, (Pos){.row = row, .col = col});
        }
    }
}

void Chess_init_board(Chess *chess) {
    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            if ((row + col) % 2 == 0) {
                chess->board[row][col].color = White;
            }
            chess->board[row][col].piece.pos.row = row;
            chess->board[row][col].piece.pos.col = col;
        }
    }

    PUT_PIECE(chess->board, 0, 0, Rook, White, 4);
    PUT_PIECE(chess->board, 0, 1, Knight, White, 3);
    PUT_PIECE(chess->board, 0, 2, Bishop, White, 2);
    PUT_PIECE(chess->board, 0, 3, Queen, White, 1);
    PUT_PIECE(chess->board, 0, 4, King, White, 0);
    PUT_PIECE(chess->board, 0, 5, Bishop, White, 2);
    PUT_PIECE(chess->board, 0, 6, Knight, White, 3);
    PUT_PIECE(chess->board, 0, 7, Rook, White, 4);

    PUT_PIECE(chess->board, 7, 0, Rook, Black, 10);
    PUT_PIECE(chess->board, 7, 1, Knight, Black, 9);
    PUT_PIECE(chess->board, 7, 2, Bishop, Black, 8);
    PUT_PIECE(chess->board, 7, 3, Queen, Black, 7);
    PUT_PIECE(chess->board, 7, 4, King, Black, 6);
    PUT_PIECE(chess->board, 7, 5, Bishop, Black, 8);
    PUT_PIECE(chess->board, 7, 6, Knight, Black, 9);
    PUT_PIECE(chess->board, 7, 7, Rook, Black, 10);

    for (int i = 0; i < CHESS_BOARD_COLS; i++) {
        PUT_PIECE(chess->board, 1, i, Pawn, White, 5);
        PUT_PIECE(chess->board, 6, i, Pawn, Black, 11);
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

void Chess_calculate_rook_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    // Top
    process_moves(game, piece, -1, 0);
    // Left
    process_moves(game, piece, 0, -1);
    // Bottom
    process_moves(game, piece, 1, 0);
    // Right
    process_moves(game, piece, 0, 1);
}

// num_moves = where to start filling the moves from
// for the Queen
void Chess_calculate_bishop_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    // Top Right
    process_moves(game, piece, -1, 1);
    // Top Left
    process_moves(game, piece, -1, -1);
    // Bottom Right
    process_moves(game, piece, 1, 1);
    // Bottom Left
    process_moves(game, piece, 1, -1);
}

void Chess_calculate_pawn_moves(Chess *game, Piece *piece, int num_moves) {
    // TODO: Handle en-passant and promotion
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    // White is always at the top. Even when rotating the board, we only display it upside down
    int row_adder = piece->color == White ? 1 : -1;

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
}

// Calculates moves of the piece that has been clicked
// and returns a pointer to that piece
// returns NULL if the clicked square does not have a piece
Piece *Chess_calculate_moves_for_piece(Chess *game, Pos pos) {
    Piece *piece = &game->board[pos.row][pos.col].piece;

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

void Chess_make_move(Chess *game, Piece *piece, Pos pos) {
    Cell *move_from = &game->board[piece->pos.row][piece->pos.col];
    Cell *move_to = &game->board[pos.row][pos.col];

    for (int i = 0; i < piece->num_moves; i++) {
        if (piece->moves[i].row == pos.row && piece->moves[i].col == pos.col) {
            move_to->piece = PUT_PIECE(game->board, pos.row, pos.col, move_from->piece.type, move_from->piece.color, move_from->piece.sprite_number);
            move_to->piece.has_moved = true;
            move_to->piece.moves = move_from->piece.moves;

            move_from->piece = (Piece){0};
            break;
        }
    }

    Chess_calculate_moves(game);
}
