#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TILE_SIZE 20

typedef struct { int x, y; } Point;
typedef struct { Point body[SCREEN_WIDTH * SCREEN_HEIGHT / (TILE_SIZE * TILE_SIZE)]; int length; Point direction; } Snake;

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *file) {
    SDL_Surface *surface = IMG_Load(file);
    if (!surface) { printf("IMG_Load Error: %s\n", IMG_GetError()); return NULL; }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color, int x, int y) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool show_menu(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *menuBackground) {
    SDL_Event e;
    bool running = true;
    bool startGame = false;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_RETURN: 
                        startGame = true;
                        running = false;
                        break;
                    case SDLK_ESCAPE: 
                        running = false;
                        break;
                }
            }
        }

        SDL_RenderCopy(renderer, menuBackground, NULL, NULL);

        render_text(renderer, font, "Snake Game", (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 4);
        render_text(renderer, font, "Press 'Enter' to Start", (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 20);
        render_text(renderer, font, "Press 'esc' to Exit", (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 20);

        SDL_RenderPresent(renderer);
    }

    return startGame;
}

bool show_game_over(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *background, int score) {
    SDL_Event e;
    bool running = true;
    bool replay = false;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_r: 
                        replay = true;
                        running = false;
                        break;
                    case SDLK_ESCAPE: 
                        running = false;
                        break;
                }
            }
        }

        SDL_RenderCopy(renderer, background, NULL, NULL);

        render_text(renderer, font, "Game Over", (SDL_Color){255, 0, 0, 255}, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 4);
        char scoreText[64];
        sprintf(scoreText, "Your Score: %d", score);
        render_text(renderer, font, scoreText, (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20);
        render_text(renderer, font, "Press 'R' to Replay", (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20);
        render_text(renderer, font, "Press esc to Exit", (SDL_Color){255, 255, 255, 255}, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 60);

        SDL_RenderPresent(renderer);
    }

    return replay;
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() == -1 || Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 1;
    SDL_Window *window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window || !renderer) return 1;

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    Mix_Chunk *eatSound = Mix_LoadWAV("eat.wav");
    SDL_Texture *foodTexture = load_texture(renderer, "food.png");
    SDL_Texture *snakeTexture = load_texture(renderer, "snake.png");
    SDL_Texture *backgroundTexture = load_texture(renderer, "bg.png");
    SDL_Texture *menuBackgroundTexture = load_texture(renderer, "frontpage.png");
    if (!font || !eatSound || !foodTexture || !snakeTexture || !backgroundTexture || !menuBackgroundTexture) return 1;

    if (!show_menu(renderer, font, menuBackgroundTexture)) {
        SDL_DestroyTexture(menuBackgroundTexture); SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(foodTexture); SDL_DestroyTexture(snakeTexture);
        Mix_FreeChunk(eatSound); TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); Mix_CloseAudio(); TTF_Quit(); SDL_Quit();
        return 0;
    }

    bool running = true;
    while (running) {
        srand(time(NULL));
        Snake snake = {{0}, 5, {1, 0}};
        for (int i = 0; i < snake.length; ++i) snake.body[i] = (Point){snake.length - i - 1, 0};
        Point food = {rand() % (SCREEN_WIDTH / TILE_SIZE), rand() % (SCREEN_HEIGHT / TILE_SIZE)};
        int score = 0;

        bool gameActive = true;
        SDL_Event e;

        while (gameActive) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    gameActive = false;
                    running = false;
                }
                if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_UP: if (snake.direction.y == 0) snake.direction = (Point){0, -1}; break;
                        case SDLK_DOWN: if (snake.direction.y == 0) snake.direction = (Point){0, 1}; break;
                        case SDLK_LEFT: if (snake.direction.x == 0) snake.direction = (Point){-1, 0}; break;
                        case SDLK_RIGHT: if (snake.direction.x == 0) snake.direction = (Point){1, 0}; break;
                    }
                }
            }

            for (int i = snake.length - 1; i > 0; --i) snake.body[i] = snake.body[i - 1];
            snake.body[0].x += snake.direction.x; snake.body[0].y += snake.direction.y;

            if (snake.body[0].x < 0 || snake.body[0].x >= SCREEN_WIDTH / TILE_SIZE || snake.body[0].y < 0 || snake.body[0].y >= SCREEN_HEIGHT / TILE_SIZE) gameActive = false;
            for (int i = 1; i < snake.length; ++i) if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y) gameActive = false;

            if (snake.body[0].x == food.x && snake.body[0].y == food.y) {
                snake.length++; score++; food = (Point){rand() % (SCREEN_WIDTH / TILE_SIZE), rand() % (SCREEN_HEIGHT / TILE_SIZE)};
                Mix_PlayChannel(-1, eatSound, 0);
            }

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

            SDL_Rect foodRect = {food.x * TILE_SIZE, food.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(renderer, foodTexture, NULL, &foodRect);

            for (int i = 0; i < snake.length; ++i) {
                SDL_Rect snakeRect = {snake.body[i].x * TILE_SIZE, snake.body[i].y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderCopy(renderer, snakeTexture, NULL, &snakeRect);
            }

            char scoreText[32]; sprintf(scoreText, "Score: %d", score);
            render_text(renderer, font, scoreText, (SDL_Color){255, 255, 255, 255}, 10, 10);

            SDL_RenderPresent(renderer);
            SDL_Delay(100);
        }

        if (!show_game_over(renderer, font, menuBackgroundTexture, score)) {
            running = false;
        }
    }

    SDL_DestroyTexture(menuBackgroundTexture); 
    SDL_DestroyTexture(backgroundTexture); 
    SDL_DestroyTexture(foodTexture); 
    SDL_DestroyTexture(snakeTexture);
    Mix_FreeChunk(eatSound); 
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer); 
    SDL_DestroyWindow(window); 
    Mix_CloseAudio(); 
    TTF_Quit(); 
    SDL_Quit();
    return 0;
}
