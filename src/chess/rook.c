#include "chess.h"

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

