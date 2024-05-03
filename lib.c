#include <SDL.h>

#include "cpu.h"

const uint8_t keymap[16] = {
        SDL_SCANCODE_X, // 0
        SDL_SCANCODE_1, // 1
        SDL_SCANCODE_2, // 2
        SDL_SCANCODE_3, // 3
        SDL_SCANCODE_Q, // 4
        SDL_SCANCODE_W, // 5
        SDL_SCANCODE_E, // 6
        SDL_SCANCODE_A, // 7
        SDL_SCANCODE_S, // 8
        SDL_SCANCODE_D, // 9
        SDL_SCANCODE_Z, // A
        SDL_SCANCODE_C, // B
        SDL_SCANCODE_4, // C
        SDL_SCANCODE_R, // D
        SDL_SCANCODE_F, // E    
        SDL_SCANCODE_V  // F
    };

void handleInput(SDL_Event ev, char *arg, uint8_t *f) {
    if(ev.type == SDL_KEYDOWN) {
        switch(ev.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                *f = 1;
                break;
            case SDL_SCANCODE_L: // Reset
                cpuInitialization(arg); 
                break;
        //  case SDL_SCANCODE_P: // Pause
        //      if(cpu.pauseFlag == 0)
        //          cpu.pauseFlag = 1;
        //      else
        //          cpu.pauseFlag = 0;
        //      break;
            default:
                for(int i = 0; i < 16; i++) {
                    if(ev.key.keysym.scancode == keymap[i])
                        cpu.keypad[i] = 1;  // If a key is down, set its corresponding value in our array to 0
                }
        }
    }

    if (ev.type == SDL_KEYUP) {
        for (int i = 0; i < 16; i++) {
            if (ev.key.keysym.scancode == keymap[i])
                cpu.keypad[i] = 0; // If a key is up, set its corresponding value in our array to 0
        }
    }
    
    if(ev.type == SDL_QUIT)
        *f = 1;

    return;
}

void audioCallback(void* udata, uint8_t *stream, int len) {
    int16_t *fstream = (int16_t *)stream;
    static uint32_t sampleIndex = 0;
    const int32_t period = SAMPLE_RATE / WAVE_FREQ;
    const int32_t halfPeriod = period / 2;

    // Divide len by 2: we're filling the buffer two bytes at a time
    for(int i = 0; i < len / 2; i++)
        fstream[i] = ((sampleIndex++ / halfPeriod) % 2) ? VOLUME : -VOLUME;
    
    return;
}