#ifndef CONFIG_H
#define CONFIG_H

#define INSTRUCTIONS_PER_SECOND 550 // Put your desired emulator speed here (instruction cycles per second)

// Audio
#define SAMPLE_RATE 44100
#define WAVE_FREQ 440 // Frequency of our beeper (440hz square wave)
#define SAMPLES 512
#define VOLUME 3000 // Maximum volume: INT16_MAX

#endif