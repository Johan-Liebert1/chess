#!/bin/bash

mkdir -p bin

gcc -Wall -Wextra -o bin/main main.c arena.c -lm -lSDL2 -lSDL2_image -std=c11

if [[ -z $1 ]]; then
    ./bin/main
fi
