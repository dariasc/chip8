#include "render.h"
#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <stdio.h>

#define WIN_MULT 12

SDL_Window *win;
SDL_Renderer *ren;

uint8 buffer[64][32];

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

  sdl_clear();
}

void sdl_destroy() {
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

void sdl_clear() {
  memset(buffer, 0, sizeof buffer);
}

int sdl_draw(int x, int y, uint8 sprite[], uint8 size) {
  // we want to wrap the starting position
  x %= 64;
  y %= 32;

  int unset = 0;
  // pull the data from the sprite bits and push it to our buffer
  for (int i = 0; i < size; i++) {
    if (y + i >= 32) break;

    for (int j = 0; j < 8; j++) {
      if (x+j >= 64) continue;

      uint8 bit = (sprite[i]) & (0x80 >> j);
      if (!bit) {
	continue;
      }
      // if both the sprite and buffer have this pixel as on
      // we have to flip it and change V[F] register
      if (buffer[x+j][y+i]) {
	unset = 1;
      }
      buffer[x+j][y+i] ^= 1;
    }
  }

  return unset;
}

void sdl_paint() { 
  SDL_SetRenderDrawColor(ren, 0x0, 0x0, 0x0, 0x0);
  SDL_RenderClear(ren);

  SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
  for (int x = 0; x < 64; x++) {
    for (int y = 0; y < 32; y++) {
      uint8 bit = buffer[x][y];
      if (!bit) continue;

      SDL_Rect rect;
      rect.x = x * WIN_MULT;
      rect.y = y * WIN_MULT;
      rect.h = WIN_MULT;
      rect.w = WIN_MULT;
      SDL_RenderFillRect(ren, &rect); 
    }
  }
  SDL_RenderPresent(ren);
}

