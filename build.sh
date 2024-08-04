#!/bin/bash

mkdir -p bin

gcc -Wall -o bin/main main.c -lm -lSDL2 -lSDL2_image

if [[ -z $1 ]]; then
    ./bin/main
fi
