#!/bin/bash

# Compile the Context Engine with SDL2
g++ -std=c++17 -Wall -Wextra -o test test.cpp context-engine.cpp -lSDL2 -lSDL2_ttf

echo "Compilation finished. Run ./test to start the application." 