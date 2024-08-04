#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdbool.h>
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

int mouse_x;
int mouse_y;

enum PieceType { UndefPieceType, King, Queen, Rook, Bishop, Knight, Pawn };

enum Color {
    Black,
    White,
};

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
};
typedef struct _Piece Piece;

struct _Cell {
    Piece piece;
    enum Color color;
};
typedef struct _Cell Cell;

typedef Cell ChessBoard[CHESS_BOARD_ROWS][CHESS_BOARD_COLS];

struct _Chess {
    ChessBoard board;
};
typedef struct _Chess Chess;

#define PUT_PIECE(board, row_val, col_val, piece_type, color_val, sprite_number)                                                                     \
    board[row_val][col_val].piece = (Piece) {                                                                                                        \
        .pos = {.row = row_val, .col = col_val}, .type = piece_type, .color = color_val, .sprite_loc = (SDL_Rect) {                                  \
            .x = (sprite_number < SPRITE_SHEET_COLS ? sprite_number : sprite_number % SPRITE_SHEET_COLS) * SPRITE_WIDTH,                             \
            .y = sprite_number < SPRITE_SHEET_COLS ? 0 : SPRITE_HEIGHT, .w = SPRITE_WIDTH, .h = SPRITE_HEIGHT                                        \
        }                                                                                                                                            \
    }

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

    // for (int i = 0; i < CHESS_BOARD_COLS; i++) {
    //     PUT_PIECE(chess->board, 1, i, Pawn, Black, 5);
    //     PUT_PIECE(chess->board, 6, i, Pawn, Black, 11);
    // }
}

void Chess_calculate_bishop_moves(Chess *game, Piece *piece) {
    // Top Right
    for (int row = piece->pos.row - 1; row >= 0; row--) {
        for (int col = piece->pos.col + 1; col <= CHESS_BOARD_COLS; col++) {
            Cell cell = game->board[row][col];

            if (cell.piece.type == UndefPieceType) {
                // NO piece on this square, can move here
                printf("Move: %d, %d\n", row, col);
                continue;
            }

            if (cell.piece.color != piece->color) {
                // Opposite colored piece, can capture
                printf("Capture: %d, %d\n", row, col);
            } else {
                goto top_left;
            }
        }
    }

top_left:
    for (int row = piece->pos.row - 1; row >= 0; row--) {
        for (int col = piece->pos.col - 1; col >= 0; col--) {
            Cell cell = game->board[row][col];

            if (cell.piece.type == UndefPieceType) {
                // NO piece on this square, can move here
                printf("row: %d, col: %d\n", row, col);
                continue;
            }

            if (cell.piece.color != piece->color) {
                // Opposite colored piece, can capture
                printf("row: %d, col: %d\n", row, col);
            } else {
                goto bottom_left;
            }
        }
    }

bottom_left:
    for (int row = piece->pos.row + 1; row < CHESS_BOARD_ROWS; row++) {
        for (int col = piece->pos.col - 1; col >= 0; col--) {
            Cell cell = game->board[row][col];

            if (cell.piece.type == UndefPieceType) {
                // NO piece on this square, can move here
                printf("row: %d, col: %d\n", row, col);
                continue;
            }

            if (cell.piece.color != piece->color) {
                // Opposite colored piece, can capture
                printf("row: %d, col: %d\n", row, col);
            } else {
                goto bottom_right;
            }
        }
    }

bottom_right:
    for (int row = piece->pos.row - 1; row >= 0; row--) {
        for (int col = piece->pos.col + 1; col <= CHESS_BOARD_COLS; col++) {
            Cell cell = game->board[row][col];

            if (cell.piece.type == UndefPieceType) {
                // NO piece on this square, can move here
                printf("row: %d, col: %d\n", row, col);
                continue;
            }

            if (cell.piece.color != piece->color) {
                // Opposite colored piece, can capture
                printf("row: %d, col: %d\n", row, col);
            } else {
                goto out;
            }
        }
    }

out:
    return;
}

void Chess_calculate_moves(Chess *game, Pos pos) {
    Piece piece = game->board[pos.row][pos.col].piece;

    switch (piece.type) {
        case UndefPieceType:
            return;

        case King:

        case Queen:

        case Rook:

        case Bishop:
            return Chess_calculate_bishop_moves(game, &piece);

        case Knight:

        case Pawn:
            break;
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

    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
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
    Chess_init_board(&game);

    SDL_Texture *sprites_texture = init_sprites(renderer);

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            SDL_GetMouseState(&mouse_x, &mouse_y);

            Pos pos = mouse_pos_to_cell();

            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;

                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        case SDLK_q: {
                            quit = 1;
                            break;
                        }

                        case SDL_MOUSEBUTTONDOWN: {
                            Chess_calculate_moves(&game, pos);
                        }
                    }
                }
            }

            draw_chess_board(&game, renderer, sprites_texture, pos);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_Quit();
}
