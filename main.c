#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <sys/types.h>

#define COLOR_BLACK 0, 0, 0, 255
#define COLOR_WHITE 255, 255, 255, 255

#define CHESS_BOARD_ROWS 8
#define CHESS_BOARD_COLS 8
#define CELL_SIZE 100

#define WINDOW_SCALE_FACTOR 100

enum PieceType { King, Queen, Rook, Bishop, Knight, Pawn };

enum Color {
    Black,
    White,
};

struct Pos {
    int row;
    int col;
};

struct Piece {
    struct Pos pos;
    enum PieceType type;
    enum Color color;
};

struct Cell {
    struct Piece *piece;
    enum Color color;
};

typedef struct Cell ChessBoard[CHESS_BOARD_ROWS][CHESS_BOARD_COLS];

struct Chess {
    ChessBoard board;
};

void draw_chess_board(ChessBoard *board, SDL_Renderer *renderer) {
    int x = 0;
    int y = 0;

    // Draw a border around the board
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){.x = x, .y = y, .w = CELL_SIZE * CHESS_BOARD_COLS + 1, .h = CELL_SIZE * CHESS_BOARD_ROWS + 1});

    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            struct Cell cell = (*board)[row][col];

            if (cell.color == White) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }

            SDL_RenderFillRect(renderer, &(SDL_Rect){.x = x, .y = y, .w = CELL_SIZE, .h = CELL_SIZE});

            x += CELL_SIZE;
        }

        x = 0;
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

    int quit = 0;
    SDL_Event event;

    struct Chess game = {0};

    for (size_t row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (size_t col = 0; col < CHESS_BOARD_COLS; col++) {
            if ((row + col) % 2 != 0) {
                game.board[row][col].color = White;
            }
        }
    }

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    quit = 1;
                    break;
                }
            }

            draw_chess_board(&game.board, renderer);
            SDL_RenderPresent(renderer);
        }
    }
}
