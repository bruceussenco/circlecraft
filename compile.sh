#!/bin/bash
gcc src/main.c src/planet.c -o game -Iinclude -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
