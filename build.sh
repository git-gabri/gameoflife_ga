#!/bin/bash

g++ -O2 -Iinclude -c main.cpp -o ./obj/main.o
g++ -O2 -Iinclude -c ./src/gameoflife_t.cpp -o ./obj/gameoflife_t.o
g++ -O2 -Iinclude -c ./src/player.cpp -o ./obj/player.o


g++ -o gameoflife_ga ./obj/main.o ./obj/gameoflife_t.o ./obj/player.o -lpthread -s
