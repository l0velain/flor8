#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "config.h"

#define GFX_L 64
#define GFX_H 32

typedef struct Chip8 {
	uint16_t opcode;
	uint8_t memory[4096];
	uint16_t stack[16];
	uint8_t V[16]; // From 0 to F
	uint16_t I;
	uint16_t pc; // Program counter
	uint8_t sp; // Stack pointer
	uint8_t soundTimer;
	uint8_t delayTimer;
	uint8_t screen[GFX_L * GFX_H];
//  uint8_t pauseFlag;
	uint8_t drawFlag;
	uint8_t keypad[16];
} Chip8;

extern Chip8 cpu;

void cpuInitialization(char *fileName);
void cpuCycle();
void updateTimers();

// SDL input handling and audio
void handleInput(SDL_Event ev, char *arg, uint8_t *f);

extern SDL_AudioSpec want;
extern SDL_AudioSpec have;
extern SDL_AudioDeviceID dev;
void audioCallback(void* udata, uint8_t *stream, int lens);

#endif