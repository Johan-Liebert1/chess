#!/bin/bash

mkdir -p bin

gcc -Wall -Wextra -Wpedantic -g -o bin/main src/*.c src/chess/*.c -lm -lSDL2 -lSDL2_image -lSDL2_ttf -std=c11

if [[ -z $1 ]]; then
    ./bin/main
fi
