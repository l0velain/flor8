#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#include "cpu.h"

Chip8 cpu;

void cpuInitialization(char *fileName) {    
    memset(cpu.memory, 0, sizeof(cpu.memory));
    
    // Load ROM
    FILE *f = fopen(fileName, "rb");
    if(f == NULL) {
        fprintf(stderr, "Missing file.\n");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    uint16_t size = ftell(f); 
    if(!size || size + 512 > 4096) {
        fprintf(stderr, "ROM size is too big!\n");
        fclose(f);
        exit(2);
    }
    rewind(f);

    fread(&cpu.memory[512], 1, size, f);

    // Initializing
    memset(cpu.stack, 0, sizeof(cpu.stack));
    memset(cpu.V, 0, sizeof(cpu.V));
    memset(cpu.screen, 0, sizeof(cpu.screen));
    memset(cpu.keypad, 0, sizeof(cpu.keypad));
    cpu.pc = 0x200; // 512
    cpu.soundTimer = 0;
    cpu.delayTimer = 0;
    cpu.I = 0;
    cpu.sp = 0;
//  cpu.pauseFlag = 0;
    cpu.drawFlag = 0;

    // Randomness!
    srand(time(NULL));

    // Loading font
    const uint8_t font[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for(int i = 0; i < 80; i++)
        cpu.memory[i] = font[i];
    
    return;
}

// Following http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0
void cpuCycle() {
    cpu.drawFlag = 0;
    cpu.opcode = cpu.memory[cpu.pc] << 8 | cpu.memory[cpu.pc + 1];
    uint16_t nnn = cpu.opcode & 0x0FFF;
    uint8_t n = cpu.opcode & 0x000F;
    uint8_t x = cpu.opcode >> 8 & 0x0F;
    uint8_t y = cpu.opcode >> 4 & 0x00F;
    uint8_t kk = cpu.opcode & 0x00FF;

    switch(cpu.opcode & 0xF000) {
        case 0x0000:
            switch(kk) {
                case 0xE0:
                    memset(cpu.screen, 0, sizeof(cpu.screen));
                    cpu.pc += 2;
                    break;
                case 0xEE:
                    cpu.pc = cpu.stack[cpu.sp];
                    cpu.sp--;
                    cpu.pc += 2;
                    break;
                default:
                    fprintf(stderr, "UNSUPPORTED OPCODE 0x0000: %04X\n", cpu.opcode);
                    exit(2);
            }
            break;
        case 0x1000:
            cpu.pc = nnn;
            break;
        case 0x2000:
            cpu.sp++;
            cpu.stack[cpu.sp] = cpu.pc;
            cpu.pc = nnn;
            break;
        case 0x3000:
            if(cpu.V[x] == kk)
                cpu.pc += 2;
            cpu.pc += 2;
            break;
        case 0x4000:
            if(cpu.V[x] != kk)
                cpu.pc += 2;
            cpu.pc += 2;
            break;
        case 0x5000:
            if(cpu.V[x] == cpu.V[y])
                cpu.pc += 2;
            cpu.pc += 2;
            break;
        case 0x6000:
            cpu.V[x] = kk;
            cpu.pc += 2;
            break;
        case 0x7000:
            cpu.V[x] += kk;
            cpu.pc += 2;
            break;
        case 0x8000:
            switch(n) {
                case 0x0:
                    cpu.V[x] = cpu.V[y];
                    cpu.pc += 2;
                    break;
                case 0x1:
                    cpu.V[x] |= cpu.V[y];
                    cpu.pc += 2;
                    break;
                case 0x2:
                    cpu.V[x] &= cpu.V[y];
                    cpu.pc += 2;
                    break;
                case 0x3:
                    cpu.V[x] ^= cpu.V[y];
                    cpu.pc += 2;
                    break;
                case 0x4:
                    uint8_t temp = cpu.V[x] + cpu.V[y];
                    cpu.V[0xF] = temp > 255 ? 1 : 0;
                    cpu.V[x] = temp & 0x00FF;
                    cpu.pc += 2;
                    break;
                case 0x5:
                    cpu.V[0xF] = cpu.V[x] > cpu.V[y] ? 1 : 0;
                    cpu.V[x] -= cpu.V[y];
                    cpu.pc += 2;
                    break;
                case 0x6:
                    cpu.V[0xF] = cpu.V[x] & 0x01 ? 1 : 0;
                    cpu.V[x] >>= 1; // Right-shifting is the same as dividing by two
                    cpu.pc += 2;
                    break;
                case 0x7:
                    cpu.V[0xF] = cpu.V[y] > cpu.V[x] ? 1 : 0;
                    cpu.V[x] = cpu.V[y] - cpu.V[x];
                    cpu.pc += 2;
                    break;
                case 0xE:
                    cpu.V[0xF] = cpu.V[x] & 0x80 ? 1 : 0;
                    cpu.V[x] <<= 1; // Left-shifting is the same as multiplying by two
                    cpu.pc += 2;
                    break;
                default:
                    fprintf(stderr, "UNSUPPORTED OPCODE 0x8000: %04X\n", cpu.opcode);
                    exit(2);
            }
            break;
        case 0x9000:
            if(cpu.V[x] != cpu.V[y])
                cpu.pc += 2;
            cpu.pc += 2;
            break;
        case 0xA000:
            cpu.I = nnn;
            cpu.pc += 2;
            break;
        case 0xB000:
            cpu.pc = nnn + cpu.V[0];
            break;
        case 0xC000:
            cpu.V[x] = (rand() % 255) & kk;
            cpu.pc += 2;
            break;
        case 0xD000:
            cpu.drawFlag = 1;
            cpu.V[0xF] = 0;
            for(int rows = 0; rows < n; rows++) {
                uint8_t pixel = cpu.memory[cpu.I + rows];
                for(int p = 0; p < 8; p++) {
                    if((pixel & (0x80 >> p)) != 0) {
                        if(cpu.screen[(cpu.V[x] + p + ((cpu.V[y] + rows) * 64))] == 1)
                            cpu.V[0xF] = 1;
                        cpu.screen[cpu.V[x] + p + ((cpu.V[y] + rows) * 64)] ^= 1;
                    }
                }
            }
            cpu.pc += 2;
            break;
        case 0xE000:
            switch(kk) {
                case 0x9E:
                    if(cpu.keypad[cpu.V[x]] == 1)
                        cpu.pc += 2;
                    cpu.pc += 2;
                    break;
                case 0xA1:
                    if(cpu.keypad[cpu.V[x]] == 0)
                        cpu.pc += 2;
                    cpu.pc += 2;
                    break;
                default:
                    fprintf(stderr, "UNSUPPORTED OPCODE 0xE000: %04X\n", cpu.opcode);
                    exit(2);
            }
            break;
        case 0xF000:
            switch(kk) {
                case 0x07:
                    cpu.V[x] = cpu.delayTimer;
                    cpu.pc += 2;
                    break;
                case 0x0A:
                    for(int i = 0; i < 16; i++) {
                        if(cpu.keypad[i] == 1)
                            cpu.V[x] = cpu.keypad[i];
                    }
                    cpu.pc += 2;
                    break;
                case 0x15:
                    cpu.delayTimer = cpu.V[x];
                    cpu.pc += 2;
                    break;
                case 0x18:
                    cpu.soundTimer = cpu.V[x];
                    cpu.pc += 2;
                    break;
                case 0x1E:
                    cpu.I += cpu.V[x];
                    cpu.pc += 2;
                    break;
                case 0x29:
                    cpu.I = cpu.V[x] * 5;
                    cpu.pc += 2;
                    break;
                case 0x33:
                    cpu.memory[cpu.I] = (cpu.V[x] / 100) % 19;
                    cpu.memory[cpu.I + 1] = (cpu.V[x] / 10) % 10;
                    cpu.memory[cpu.I + 2] = cpu.V[x] % 10;
                    cpu.pc += 2;
                    break;
                case 0x55:
                    for(int counter = 0x0; counter <= x; counter++) 
                        cpu.memory[cpu.I + counter] = cpu.V[counter];
                    cpu.pc += 2;
                    break;
                case 0x65:
                    for(int counter = 0x0; counter <= x; counter++)
                        cpu.V[counter] = cpu.memory[cpu.I + counter];
                    cpu.pc += 2;
                    break;
                default:
                    fprintf(stderr, "UNSUPPORTED OPCODE 0xF000: %04X\n", cpu.opcode);
                    exit(2);
            }
            break;
        default:
            fprintf(stderr, "UNSUPPORTED OPCODE: %04X", cpu.opcode);
            exit(2);
    }
    return;
}

void updateTimers(SDL_AudioDeviceID *device) {
    if(cpu.delayTimer > 0)
        cpu.delayTimer--;
    if(cpu.soundTimer > 0) {
        SDL_PauseAudioDevice(*device, 0);
        cpu.soundTimer--;
    } else
        SDL_PauseAudioDevice(*device, 1);
}
