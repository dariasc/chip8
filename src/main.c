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

uint8 delay_timer = 0x0;
uint8 sound_timer = 0x0;

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
      // jmp to NNN
      ip = nnn;
      break;
    case 0xb:
      // jump to NNN
      ip = nnn + registers[0];
      break;
    case 0x3:
      // skip instruction if register X == NN
      if (registers[x] == nn) {
	ip += 2;
      }
      break;
    case 0x4:
      // skip instruction if register X != NN
      if (registers[x] != nn) {
	ip +=2;
      }
      break;
    case 0x5:
      // skip instruction if register X == register Y
      if (registers[x] == registers[y]) {
	ip += 2;
      }
      break;
    case 0x9:
      // skip instruction if register X != register Y
      if (registers[x] != registers[y]) {
	ip += 2;
      }
      break;
    case 0x6:
      // set register X to NN
      registers[x] = nn;
      break;
    case 0x7:
      // add NN to register X
      registers[x] += nn;
      break;
    case 0x8:
      // register operations
      switch (n) {
	case 0x0:
	  registers[x] = registers[y];
	  break;
	// bitwise operations
	case 0x1:
	  registers[x] |= registers[y]; 
	  break;
	case 0x2:
	  registers[x] &= registers[y];
	  break;
	case 0x3:
	  registers[x] ^= registers[y];
	  break;
	case 0x4:
	  registers[0xF] = 0;
	  if ((int) registers[x] + (int) registers[y] > 256) {
	    registers[0xF] = 1;
	  }
	  registers[x] += registers[y];
	  break;
	case 0x5:
	  registers[0xF] = 0;
	  if (registers[x] > registers[y]) {
	    registers[0xF] = 1;
	  }
	  registers[x] -= registers[y];
	  break;
	case 0x7:
	  registers[0xF] = 0;
	  if (registers[y] > registers[x]) {
	    registers[0xF] = 1;
	  }
	  registers[x] = registers[y] - registers[x];
	  break;
	default:
	  printf("[*] register instruction not implemented 0x%04x\n", op);
      }
      break;
    case 0xa:
      // set index register to NNN
      index_register = nnn; 
      break;
    case 0xd:
      // draw()
      {
	uint8 sprite[n];
	memcpy(sprite, memory + index_register, sizeof(uint8) * n);
	uint8 unset = sdl_draw(registers[x], registers[y], sprite, n);
	registers[0xF] = unset;
      }
      break;
    case 0xf:
      switch (nn) {
	case 0x07:
	  // store delay_timer into register X
	  registers[x] = delay_timer;
	  break;
	case 0x15:
	  // set delay_timer to register X
	  delay_timer = registers[x];
	  break;
	case 0x18:
	  // set sound_timer to register X
	  sound_timer = registers[x];
	  break;
	case 0x1e:
	  // add register X to index register
	  index_register += registers[x];
	  break;
	case 0x55:
	  // store values from register 0 -> X into memory at I
	  for (int i = 0; i <= x; i++) {
	    memory[index_register+i] = registers[i];
	  }
	  break;
	case 0x65:
	  // load values into register 0 -> X from memory at I
	  for (int i = 0; i <= x; i++) {
	    registers[i] = memory[index_register+i];
	  }
	  break;
	default:
	  printf("[+] special instruction not implemented 0x%04x\n", op);
      }
      break;
    default:
      printf("[*] instruction not implemented 0x%04x\n", op);
  }
}

void chip_execute_frame() {
  // we want to run 700 at instructions per second,
  // because we have 60 frames per second we run
  // 700 / 60 instructions per frame
  for (int i = 0; i < 700/60; i++) {
    chip_execute();
  }
  
  if (delay_timer != 0x0)
    delay_timer--;

  if (sound_timer != 0x0)
    sound_timer--;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("bad arguments\n");
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

