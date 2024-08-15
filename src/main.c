#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "chess/chess.h"

#define COLOR_BLACK ((SDL_Color){0, 0, 0, 255})
#define COLOR_WHITE ((SDL_Color){255, 255, 255, 255})

#define CELL_SIZE 120
#define CELL_CAPTURE_TRIANGLE_SIZE (CELL_SIZE / 5.)
#define CELL_FONT_SIZE (CELL_SIZE / 5.)

#define CELL_CAPTURE_TRIANGE_COLOR ((SDL_Color){255, 0, 0, 150})

#define BOARD_POS_X_START 10
#define BOARD_POS_Y_START 40

#define WINDOW_SCALE_FACTOR 120

#define FPS (1000. / 60.)

int mouse_x;
int mouse_y;

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
    SDL_Surface *sprites_surface = IMG_Load("./src/assets/chesssprites.png");

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

SDL_Vertex vertices[12] = {0};

static inline void get_vertices_for_capturable_piece(int cell_coord_x, int cell_coord_y) {
    // top left
    vertices[0] = (SDL_Vertex){{(float)cell_coord_x, (float)cell_coord_y}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[1] = (SDL_Vertex){{(float)(cell_coord_x + CELL_CAPTURE_TRIANGLE_SIZE), (float)cell_coord_y}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[2] = (SDL_Vertex){{(float)cell_coord_x, (float)(cell_coord_y + CELL_CAPTURE_TRIANGLE_SIZE)}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};

    // top right
    vertices[3] = (SDL_Vertex){{(float)cell_coord_x + CELL_SIZE, (float)cell_coord_y}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[4] =
        (SDL_Vertex){{(float)cell_coord_x + CELL_SIZE - CELL_CAPTURE_TRIANGLE_SIZE, (float)cell_coord_y}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[5] =
        (SDL_Vertex){{(float)cell_coord_x + CELL_SIZE, (float)cell_coord_y + CELL_CAPTURE_TRIANGLE_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};

    // bottom right
    vertices[6] = (SDL_Vertex){{(float)cell_coord_x + CELL_SIZE, (float)cell_coord_y + CELL_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[7] = (SDL_Vertex){
        {(float)cell_coord_x + CELL_SIZE - CELL_CAPTURE_TRIANGLE_SIZE, (float)cell_coord_y + CELL_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[8] = (SDL_Vertex){
        {(float)cell_coord_x + CELL_SIZE, (float)cell_coord_y + CELL_SIZE - CELL_CAPTURE_TRIANGLE_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};

    // bottom left
    vertices[9] = (SDL_Vertex){{(float)cell_coord_x, (float)cell_coord_y + CELL_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[10] =
        (SDL_Vertex){{(float)cell_coord_x, (float)cell_coord_y + CELL_SIZE - CELL_CAPTURE_TRIANGLE_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
    vertices[11] =
        (SDL_Vertex){{(float)cell_coord_x + CELL_CAPTURE_TRIANGLE_SIZE, (float)cell_coord_y + CELL_SIZE}, CELL_CAPTURE_TRIANGE_COLOR, {1, 1}};
}

void draw_chess_board(Chess *game, SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *sprites_texture, Pos pos) {
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

    char buf[2];

    for (int row = 0; row < CHESS_BOARD_ROWS; row++) {
        for (int col = 0; col < CHESS_BOARD_COLS; col++) {
            Cell cell = (game->board)[row][col];

            if (cell.color == ColorWhite) {
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

            if (game->kingInCheck[0] && Chess_is_piece(&cell.piece, King, ColorBlack)) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);

                // Draw 4 triangles inside the cell
                get_vertices_for_capturable_piece(x, y);
                if (SDL_RenderGeometry(renderer, NULL, vertices, 12, NULL, 0) != 0) {
                    printf("SDL_RenderGeometry error: %s\n", SDL_GetError());
                }
            }

            if (game->kingInCheck[1] && Chess_is_piece(&cell.piece, King, ColorWhite)) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);

                // Draw 4 triangles inside the cell
                get_vertices_for_capturable_piece(x, y);
                if (SDL_RenderGeometry(renderer, NULL, vertices, 12, NULL, 0) != 0) {
                    printf("SDL_RenderGeometry error: %s\n", SDL_GetError());
                }
            }

            if (row == CHESS_BOARD_ROWS - 1) {
                snprintf(buf, 2, "%c", 'a' + col);
                SDL_Surface *surfaceMessage = TTF_RenderText_Blended(font, buf, cell.color == ColorWhite ? COLOR_BLACK : COLOR_WHITE);
                SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

                cell_dst.x = x + CELL_SIZE - CELL_FONT_SIZE;
                cell_dst.y = y + CELL_SIZE - CELL_FONT_SIZE * 2;
                cell_dst.w = CELL_FONT_SIZE;
                cell_dst.h = CELL_FONT_SIZE * 2;

                SDL_RenderCopy(renderer, message, NULL, &cell_dst);

                SDL_FreeSurface(surfaceMessage);
                SDL_DestroyTexture(message);
            }

            if (col == 0) {
                snprintf(buf, 2, "%d", CHESS_BOARD_ROWS - row);
                SDL_Surface *surfaceMessage = TTF_RenderText_Blended(font, buf, cell.color == ColorWhite ? COLOR_BLACK : COLOR_WHITE);
                SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

                cell_dst.x = x;
                cell_dst.y = y + CELL_SIZE - CELL_FONT_SIZE * 2;
                cell_dst.w = CELL_FONT_SIZE;
                cell_dst.h = CELL_FONT_SIZE * 2;

                SDL_RenderCopy(renderer, message, NULL, &cell_dst);

                SDL_FreeSurface(surfaceMessage);
                SDL_DestroyTexture(message);
            }

            x += CELL_SIZE;
        }

        x = BOARD_POS_X_START;
        y += CELL_SIZE;
    }
}

void show_piece_moves(SDL_Renderer *renderer, Chess *game, Piece *piece) {
    for (int i = 0; i < piece->num_moves; i++) {
        Vec2 cell_coord = get_cell_coordinate(piece->moves[i].row, piece->moves[i].col);

        SDL_Rect move = (SDL_Rect){
            .x = cell_coord.x + CELL_SIZE / 2 - (CELL_SIZE / 8),
            .y = cell_coord.y + CELL_SIZE / 2 - (CELL_SIZE / 8),
            .w = CELL_SIZE / 4,
            .h = CELL_SIZE / 4,
        };

        if (game->board[piece->moves[i].row][piece->moves[i].col].piece.type == UndefPieceType) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 150);
            SDL_RenderFillRect(renderer, &move);
        } else if (game->board[piece->moves[i].row][piece->moves[i].col].piece.type != King) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);

            // Draw 4 triangles inside the cell
            get_vertices_for_capturable_piece(cell_coord.x, cell_coord.y);
            if (SDL_RenderGeometry(renderer, NULL, vertices, 12, NULL, 0) != 0) {
                printf("SDL_RenderGeometry error: %s\n", SDL_GetError());
            }
        }
    }
}

void handle_mouse_click(Chess *game, Pos *pos) {
    if (!pos_within_bounds(pos->row, pos->col)) {
        return;
    }

    Cell *cell = &game->board[pos->row][pos->col];

    if (game->game_mode) {
        if (game->clicked_piece == NULL && cell->piece.color != game->current_turn) {
            return;
        }
    }

    if (game->clicked_piece == NULL && cell->piece.type != UndefPieceType) {
        game->clicked_piece = &cell->piece;
        return;
    }

    if (game->clicked_piece == &cell->piece) {
        // clicked on the same piece
        game->clicked_piece = NULL;
        return;
    }

    if (game->clicked_piece != NULL) {
        // Player has alredy clicked a piece, now he's clicked again. Might be a move
        if (Chess_make_move(game, game->clicked_piece, *pos)) {
            // move was leagal
            game->clicked_piece = NULL;
        } else if (cell->piece.type != UndefPieceType) {
            // move was illegal. Reset the clicked_piece if click was on another piece
            game->clicked_piece = &cell->piece;
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Init();
    // this opens a font style and sets a size
    TTF_Font *font = TTF_OpenFont("./src/assets/Arial.ttf", 10);

    if (font == NULL) {
        printf("Failed to load font with error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event event;

    Chess game = {0};
    game.game_mode = false;
    game.white_at_bottom = true;

    // enough to store moves for all pieces
    game.arena = arena_init(CHESS_BOARD_COLS * CHESS_BOARD_ROWS * sizeof(Pos) * 35);

    Chess_init_board(&game);
    Chess_calculate_moves(&game);

    SDL_Texture *sprites_texture = init_sprites(renderer);

    uint32_t a = SDL_GetTicks();
    uint32_t b = SDL_GetTicks();

    float delta = 0;

    while (!quit) {
        a = SDL_GetTicks();
        delta += a - b;

        if (delta < FPS) {
            continue;
        }

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
                    handle_mouse_click(&game, &pos);
                }
            }

            draw_chess_board(&game, renderer, font, sprites_texture, pos);

            if (game.clicked_piece != NULL) {
                show_piece_moves(renderer, &game, game.clicked_piece);
            }

            SDL_RenderPresent(renderer);
        }

        b = SDL_GetTicks();
    }

    TTF_CloseFont(font);
    SDL_Quit();
}
