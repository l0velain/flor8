#include <stdio.h>
#include <time.h>
#include <SDL.h>

#include "cpu.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: %s [ROM]\n", argv[0]);
        return 1;
    }
    cpuInitialization(argv[1]);
    puts("Welcome to flor8!\nQuit with ESC and reload your rom with L.");

    // SDL initialization...
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL initialization error: %s\n", SDL_GetError());
        return -1;
    }
    
    #ifdef __unix__
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    #endif

    // Initializing video...
    SDL_Window *pWindow = SDL_CreateWindow("flor8 <3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GFX_L * 8, GFX_H * 8, SDL_WINDOW_SHOWN);
    if(pWindow == NULL) {
        SDL_Quit();
        fprintf(stderr, "SDL window creation error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
    if(pRenderer == NULL) {
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        fprintf(stderr, "SDL renderer creation error: %s\n", SDL_GetError());
        return -1;
    }

    // Initializing audio...
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16LSB; // Signed 16 bit little endian
    want.channels = 1; // 1 = mono, 2 = stereo
    want.samples = SAMPLES;
    want.callback = audioCallback;
    want.userdata = NULL;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if(dev == 0) {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        fprintf(stderr, "Couldn't open audio device: %s\n", SDL_GetError());
        return -1;
    }

    if(want.format != have.format || want.channels != have.channels) {
        SDL_CloseAudioDevice(dev);
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        fprintf(stderr, "Couldn't get the required SDL_AudioSpec.\n");
    }

    // Initializing stuff for nanosleep()...
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 16666667; // 60Hz framerate

    uint8_t finish = 0;

    // Main loop
    while(finish == 0) {
        SDL_Event event;
        while(SDL_PollEvent(&event))
            handleInput(event, argv[1], &finish);

    // Infinite loop = 100% CPU usage. I want to find a better way to pause the emulator
    //  if(cpu.pauseFlag == 1) {
    //      continue;
        
        for(int i = 0; i < INSTRUCTIONS_PER_SECOND / 60; i++) {
            cpuCycle();
            if(cpu.drawFlag) {
                SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
                SDL_RenderClear(pRenderer);
                SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
                for(int cY = 0; cY < 32; cY++) {
                    for(int cX = 0; cX < 64; cX++) {    
                        if(cpu.screen[cX + (cY * 64)]) {
                            SDL_Rect rectangle;
                            rectangle.x = cX * 8;
                            rectangle.y = cY * 8;
                            rectangle.w = 8;
                            rectangle.h = 8;
                            SDL_RenderFillRect(pRenderer, &rectangle);
                        }
                    }
                }
                SDL_RenderPresent(pRenderer);
            }
        }

        updateTimers(&dev);
        nanosleep(&t, &t);

    }

    puts("Shutting down...");

    SDL_CloseAudioDevice(dev);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return 0;
}
