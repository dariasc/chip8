#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdio.h>
#include "render.h"

typedef unsigned short uint16;
typedef unsigned char uint8;

uint16 ip = 0x200;
uint8 memory[4096];

void chip_execute() {
  ip += 2;
  uint16 op = memory[ip] << 8 | memory[ip+1];
  uint8 instr = op >> 12;
  printf("instr %02x\n", instr);
}

void chip_execute_frame() {
  // we want to run 700 at instructions per second,
  // because we have 60 frames per second we run
  // 700 / 60 instructions per frame
  for (int i = 0; i < 700/60; i++) {
    chip_execute();
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("bad arguments");
    return 1;
  }

  // reset memory
  memset(memory, 0, sizeof memory);

  // load rom from file into memory at offset 0x200
  FILE* fd = fopen(argv[1], "rb");
  fread(memory + 0x200, 1, sizeof memory - 0x200, fd);
  chip_execute();

  // init sdl window stuff
  sdl_init();

  SDL_Event event;
  int quit = 0;
  int last = 0;
  while (!quit) {
    // only run loop 60 times a second
    if (SDL_GetTicks() - last < 1000/60) {
      continue;
    }
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
	quit = 1;
      }
    }

    sdl_draw();
    // chip_execute_frame();
    last = SDL_GetTicks();
  }

  sdl_destroy();
  return 0;
}

