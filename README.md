# visual_sort
This is a simple visualisation of common sorting algorithms written in C99 and SDL.  
It has actually been my very first C project, so that maybe explains some horrendous parts of the code.

![Screenshot of the program](screenshot.png)

## Compiling
If you are running Windows, you can download the compiled binaries from [releases](../../releases).  
In order to compile the code, you need to have `SDL2` and `SDL2_image` installed. 
All it takes then is to set the environment variable `SDL_CONFIG` to the path to the `sdl-config` file (for *nix users, it is defaulted to `/usr/local/bin/sdl2-config`)
and run the `Makefile`. The compilation was tested on both Ubuntu and Windows (MinGW64).
