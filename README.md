# flor8 💕
A Chip8 emulator written in C!

You can customize audio related things and the number of instruction cycles ran per second by editing config.h. <br/>
I used SDL2 as backend and you can compile flor8 with: <br/>
```gcc -o flor8 main.c cpu.c lib.c `sdl2-config --cflags --libs```

Quit with ESC or reset the interpreter with L.
