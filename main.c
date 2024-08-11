#include "arena.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define SPRITE_SHEET_WIDTH 800
#define SPRITE_SHEET_HEIGHT 267

#define SPRITE_SHEET_ROWS 2
#define SPRITE_SHEET_COLS 6

#define SPRITE_WIDTH (SPRITE_SHEET_WIDTH / SPRITE_SHEET_COLS)
#define SPRITE_HEIGHT (SPRITE_SHEET_HEIGHT / SPRITE_SHEET_ROWS)

#define COLOR_BLACK 0, 0, 0, 255
#define COLOR_WHITE 255, 255, 255, 255

#define CHESS_BOARD_ROWS 8
#define CHESS_BOARD_COLS 8
#define CELL_SIZE 100

#define BOARD_POS_X_START 10
#define BOARD_POS_Y_START 40

#define WINDOW_SCALE_FACTOR 100

static int KnightRows[4] = {2, 1, -1, -2};
static int KnightCols[4][2] = {{-1, 1}, {-2, 2}, {-2, 2}, {-1, 1}};

int mouse_x;
int mouse_y;

enum PieceType { UndefPieceType, King, Queen, Rook, Bishop, Knight, Pawn };

enum Color {
    Black,
    White,
};

struct _Vec2 {
    int x;
    int y;
};
typedef struct _Vec2 Vec2;

struct _Pos {
    int row;
    int col;
};
typedef struct _Pos Pos;

struct _Piece {
    Pos pos;
    SDL_Rect sprite_loc;
    enum PieceType type;
    enum Color color;
    bool has_moved;
    bool is_protected;
    uint8_t num_moves;
    // it's 2024, memory is cheap, screw it
    Pos *moves;
    int sprite_number;
};
typedef struct _Piece Piece;

struct _Cell {
    Piece piece;
    enum Color color;
    // idx 0 = 1 -> under attack by white
    // idx 1 = 1 -> under attack by black
    bool underAttack[2];
};
typedef struct _Cell Cell;

typedef Cell ChessBoard[CHESS_BOARD_ROWS][CHESS_BOARD_COLS];

struct _Chess {
    ChessBoard board;
    Arena arena;
    Piece *clicked_piece;
};
typedef struct _Chess Chess;

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

static inline bool pos_within_bounds(int row, int col) { return row >= 0 && row < CHESS_BOARD_ROWS && col >= 0 && col < CHESS_BOARD_COLS; }

// This expects bounds checks to be done before calling it
static inline bool is_cell_empty(ChessBoard *board, int row, int col) { return (*board)[row][col].piece.type == UndefPieceType; };

void Chess_init_board(Chess *chess) {
    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            if ((row + col) % 2 == 0) {
                chess->board[row][col].color = White;
            }
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

void Chess_calculate_rook_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    int row = piece->pos.row;
    int col = piece->pos.col;

    // Top
    for (int row = piece->pos.row - 1; row >= 0; row--) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
        } else {
            break;
        }
    }

    // Left
    for (int col = piece->pos.col - 1; col >= 0; col--) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
        } else {
            break;
        }
    }

    // Bottom
    for (int row = piece->pos.row + 1; row < CHESS_BOARD_ROWS; row++) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
        } else {
            break;
        }
    }

    // Right
    for (int col = piece->pos.col + 1; col < CHESS_BOARD_COLS; col++) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            break;
        } else {
            break;
        }
    }
}

// num_moves = where to start filling the moves from
// for the Queen
void Chess_calculate_bishop_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    // Top Right
    for (int row = piece->pos.row - 1, col = piece->pos.col + 1; row >= 0 && col < CHESS_BOARD_COLS; row--, col++) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
        } else {
            break;
        }
    }

    // top left
    for (int row = piece->pos.row - 1, col = piece->pos.col - 1; row >= 0 && col >= 0; row--, col--) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
        } else {
            break;
        }
    }

    // bottom left
    for (int row = piece->pos.row + 1, col = piece->pos.col - 1; row < CHESS_BOARD_ROWS && col >= 0; row++, col--) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
        } else {
            break;
        }
    }

    // bottom right
    for (int row = piece->pos.row + 1, col = piece->pos.col + 1; row < CHESS_BOARD_ROWS && col < CHESS_BOARD_COLS; row++, col++) {
        Cell cell = game->board[row][col];

        if (cell.piece.type == UndefPieceType) {
            // NO piece on this square, can move here
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            continue;
        }

        if (cell.piece.color != piece->color) {
            // Opposite colored piece, can capture
            piece->moves[piece->num_moves] = (Pos){.col = col, .row = row};
            piece->num_moves += 1;
            break;
        } else {
            break;
        }
    }
}

void Chess_calculate_pawn_moves(Chess *game, Piece *piece, int num_moves) {
    // TODO: Handle en-passant and promotion
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    // White is always at the top. Even when rotating the board, we only display it upside down
    int row_adder = piece->color == White ? 1 : -1;

    printf("row_adder: %d\n", row_adder);

    int row = piece->pos.row + row_adder;
    int col = piece->pos.col;

    if (pos_within_bounds(row, col) && is_cell_empty(&game->board, row, col)) {
        piece->moves[piece->num_moves] = (Pos){.row = row, .col = col};
        piece->num_moves += 1;
    }

    row = piece->pos.row + row_adder * 2;
    if (!piece->has_moved && pos_within_bounds(row, col) && is_cell_empty(&game->board, row, col)) {
        piece->moves[piece->num_moves] = (Pos){.row = row, .col = col};
        piece->num_moves += 1;
    }
}

// num_moves -> for consistancy
void Chess_calculate_knight_moves(Chess *game, Piece *piece, int num_moves) {
    piece->num_moves = num_moves;

    if (piece->moves == NULL) {
        piece->moves = arena_alloc(&game->arena, sizeof(Pos) * 64);
    }

    for (int row_idx = 0; row_idx < 4; row_idx++) {
        for (int col_idx = 0; col_idx < 2; col_idx++) {
            int row_adder = KnightRows[row_idx];
            int col_adder = KnightCols[row_idx][col_idx];

            Pos potential_move = (Pos){.row = piece->pos.row + row_adder, .col = piece->pos.col + col_adder};

            if (pos_within_bounds(potential_move.row, potential_move.col)) {
                Cell cell = game->board[potential_move.row][potential_move.col];

                if (cell.piece.type == UndefPieceType || cell.piece.color != piece->color) {
                    piece->moves[piece->num_moves] = potential_move;
                    piece->num_moves += 1;
                }
            }
        }
    }
}

static inline Pos mouse_pos_to_cell() {
    Pos pos = {.row = -1, .col = -1};

    if (mouse_x > BOARD_POS_X_START && mouse_y > BOARD_POS_Y_START && mouse_x < BOARD_POS_X_START + CELL_SIZE * CHESS_BOARD_COLS &&
        mouse_y < BOARD_POS_Y_START + CELL_SIZE * CHESS_BOARD_ROWS) {

        pos.col = (mouse_x - BOARD_POS_X_START) / CELL_SIZE;
        pos.row = (mouse_y - BOARD_POS_Y_START) / CELL_SIZE;
    }

    return pos;
}

// returns the top left corner of cell at (row,col)
static inline Vec2 get_cell_coordinate(int row, int col) {
    Vec2 pos = {.x = BOARD_POS_X_START + col * CELL_SIZE, .y = BOARD_POS_Y_START + row * CELL_SIZE};
    return pos;
}

SDL_Texture *init_sprites(SDL_Renderer *renderer) {
    SDL_Surface *sprites_surface = IMG_Load("./assets/chesssprites.png");

    if (sprites_surface == NULL) {
        printf("Failed to load sprites. Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *sprites_texture = SDL_CreateTextureFromSurface(renderer, sprites_surface);

    if (sprites_texture == NULL) {
        printf("Failed to load sprites texture. Error: %s\n", SDL_GetError());
        exit(1);
    }

    return sprites_texture;
}

void draw_chess_board(Chess *game, SDL_Renderer *renderer, SDL_Texture *sprites_texture, Pos pos) {
    // Draw a border around a square if it's under the mouse cursor
    if (pos.col != -1 && pos.row != -1) {
        // find the cell
        SDL_Rect cell_dst =
            (SDL_Rect){.x = BOARD_POS_X_START + pos.col * CELL_SIZE, .y = BOARD_POS_Y_START + pos.row * CELL_SIZE, .w = CELL_SIZE, .h = CELL_SIZE};

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &cell_dst);
    }

    int x = BOARD_POS_X_START;
    int y = BOARD_POS_Y_START;

    for (int row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (int col = 0; col < CHESS_BOARD_COLS; col++) {
            Cell cell = (game->board)[row][col];

            if (cell.color == White) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }

            SDL_Rect cell_dst = (SDL_Rect){.x = x, .y = y, .w = CELL_SIZE, .h = CELL_SIZE};

            if (row == pos.row && col == pos.col) {
                cell_dst.x += 5;
                cell_dst.y += 5;
                cell_dst.w -= 10;
                cell_dst.h -= 10;
            }

            SDL_RenderFillRect(renderer, &cell_dst);

            if (cell.piece.type != UndefPieceType) {
                SDL_RenderCopy(renderer, sprites_texture, &cell.piece.sprite_loc, &cell_dst);
            }

            x += CELL_SIZE;
        }

        x = BOARD_POS_X_START;
        y += CELL_SIZE;
    }
}

void show_piece_moves(SDL_Renderer *renderer, Piece *piece) {
    for (int i = 0; i < piece->num_moves; i++) {
        Vec2 cell_coord = get_cell_coordinate(piece->moves[i].row, piece->moves[i].col);
        SDL_Rect move = (SDL_Rect){
            .x = cell_coord.x,
            .y = cell_coord.y,
            .w = CELL_SIZE / 4,
            .h = CELL_SIZE / 4,
        };

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &move);
    }
}

// Calculates moves of the piece that has been clicked
// and returns a pointer to that piece
// returns NULL if the clicked square does not have a piece
Piece *Chess_calculate_moves(Chess *game, Pos pos) {
    Piece *piece = &game->board[pos.row][pos.col].piece;

    switch (piece->type) {
        case UndefPieceType:
            return NULL;

        case King:
            assert(false && "Moves for king is not implemtend");
            return NULL;

        case Queen:
            Chess_calculate_rook_moves(game, piece, 0);
            Chess_calculate_bishop_moves(game, piece, piece->num_moves);
            return piece;

        case Rook:
            Chess_calculate_rook_moves(game, piece, 0);
            return piece;

        case Bishop:
            Chess_calculate_bishop_moves(game, piece, 0);
            return piece;

        case Knight:
            Chess_calculate_knight_moves(game, piece, 0);
            return piece;

        case Pawn:
            Chess_calculate_pawn_moves(game, piece, 0);
            return piece;
    }

    return NULL;
}

void Chess_make_move(Chess *game, Piece *piece, Pos pos) {
    printf("Chess_make_move. Move from: (%d, %d). Move to: (%d, %d)\n", piece->pos.row, piece->pos.col, pos.row, pos.col);

    Cell *move_from = &game->board[piece->pos.row][piece->pos.col];
    Cell *move_to = &game->board[pos.row][pos.col];

    for (int i = 0; i < piece->num_moves; i++) {
        if (piece->moves[i].row == pos.row && piece->moves[i].col == pos.col) {
            move_to->piece = PUT_PIECE(game->board, pos.row, pos.col, move_from->piece.type, move_from->piece.color, move_from->piece.sprite_number);
            move_to->piece.has_moved = true;

            move_from->piece = (Piece){0};
            break;
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed! %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 16 * WINDOW_SCALE_FACTOR,
                                          9 * WINDOW_SCALE_FACTOR, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("Failed to create renderer with error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event event;

    Chess game = {0};

    // enough to store moves for all pieces
    game.arena = arena_init(CHESS_BOARD_COLS * CHESS_BOARD_ROWS * sizeof(Pos) * 32);

    Chess_init_board(&game);

    SDL_Texture *sprites_texture = init_sprites(renderer);

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&mouse_x, &mouse_y);

            Pos pos = mouse_pos_to_cell();

            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;

                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        case SDLK_q: {
                            quit = 1;
                            break;
                        }
                    }

                    break;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    if (!pos_within_bounds(pos.row, pos.col)) {
                        break;
                    }

                    Cell *cell = &game.board[pos.row][pos.col];

                    if (game.clicked_piece == NULL) {
                        game.clicked_piece = Chess_calculate_moves(&game, pos);
                    } else if (game.clicked_piece == &cell->piece) {
                        // clicked on the same piece
                        game.clicked_piece = NULL;
                    } else {
                        // Player has alredy clicked a piece, now he's clicked again. Might be a move
                        Chess_make_move(&game, game.clicked_piece, pos);
                        game.clicked_piece = NULL;
                    }

                    break;
                }
            }

            draw_chess_board(&game, renderer, sprites_texture, pos);

            if (game.clicked_piece != NULL) {
                show_piece_moves(renderer, game.clicked_piece);
            }

            SDL_RenderPresent(renderer);
        }
    }

    SDL_Quit();
}
