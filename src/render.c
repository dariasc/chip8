#include <SDL2/SDL.h>
#include <stdio.h>

#define WIN_MULT 8

SDL_Window *win;
SDL_Renderer *ren;

void sdl_init() { 
  printf("sdl_init() started\n");
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("sdl_init error: %s\n", SDL_GetError());
    exit(1);
  }

  win = SDL_CreateWindow("chip_8", 100, 100, 64 * WIN_MULT, 32 * WIN_MULT, SDL_WINDOW_SHOWN);
  if (win == NULL) {
    printf("sdl_create_window error: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  ren = SDL_CreateRenderer(win, -2, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (ren == NULL) {
    SDL_DestroyWindow(win);
    printf("sdl_create_renderer error: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }
}

void sdl_destroy() {
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

void sdl_draw() {

}

