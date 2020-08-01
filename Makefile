#this was tested on Ubuntu 18.04.4 LTS with SDL 2.0.10
#this was tested on Windows 10 with SDL 2.0.12 (mintty)

CC=gcc
name=./run
src=src/*.c
SDL_CONFIG ?= /usr/local/bin/sdl2-config

all :
	${CC} ${src} -I./include `${SDL_CONFIG} --cflags --libs` -lSDL2_image -lm -o ${name}

