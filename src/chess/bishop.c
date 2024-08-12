#include "chess.h"

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

