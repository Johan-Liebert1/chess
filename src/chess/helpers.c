#include "chess.h"

bool is_cell_empty(ChessBoard *board, int row, int col) {
    return pos_within_bounds(row, col) && (*board)[row][col].piece.type == UndefPieceType;
}

// Only cheks whether piece can capture other piece at (row, col). Does not check whether (row, col)
// is a valid move
bool can_piece_capture(ChessBoard *board, Piece *piece, int row, int col) {
    if (!pos_within_bounds(row, col)) {
        return false;
    }

    Piece *other = &(*board)[row][col].piece;

    return other->type != UndefPieceType && other->color != piece->color;
}

const char *color_diplay(enum Color color) {
    switch (color) {
        case ColorBlack:
            return "Black";
        case ColorWhite:
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
