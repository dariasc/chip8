#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdio.h>
#include <string.h>
#include "render.h"

typedef unsigned short uint16;
typedef unsigned char uint8;

uint16 ip = 0x200;
uint16 index_register = 0x0;

uint8 memory[4096];
uint8 registers[0x10];

void chip_execute() {
  uint16 op = memory[ip] << 8 | memory[ip+1];
  ip += 2;
  
  uint16 nnn = op & 0xFFF;
  uint8 nn = op & 0xFF;
  uint8 n = op & 0xF;
  uint8 x = (op & 0xF00) >> 8;
  uint8 y = (op & 0xF0) >> 4;

  // printf("[+] -> executing op %08x\n", op);
  switch (op >> 12) {
    case 0x0:
      // clear screen
      sdl_clear();
      break;
    case 0x1:
      // jmp
      ip = nnn;
      break;
    case 0x6:
      // set register X to NN
      registers[x] = nn;
      break;
    case 0x7:
      // add NN to register X
      registers[x] += nn;
      break;
    case 0xa:
      // set index register to NNN
      index_register = nnn; 
      break;
    case 0xd:
      // draw()
      {
	registers[0xF] = 0;
	uint8 sprite[n];
	memcpy(sprite, memory + index_register, sizeof(uint8) * n);
	uint8 unset = sdl_draw(registers[x], registers[y], sprite, n);
	registers[0xF] = unset;
      }
      break;
    default:
      printf("unknown instruction\n");
  }
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

    sdl_paint();
    chip_execute_frame();
    last = SDL_GetTicks();
  }

  sdl_destroy();
  return 0;
}

