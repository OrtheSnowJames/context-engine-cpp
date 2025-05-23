#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Create build directory if it doesn't exist
mkdir -p build

# Compile function with error checking
compile() {
    echo -e "${YELLOW}Compiling $1...${NC}"
    if $2; then
        echo -e "${GREEN}Successfully compiled $1${NC}"
        return 0
    else
        echo -e "${RED}Failed to compile $1${NC}"
        return 1
    fi
}

# Clean build
if [ "$1" == "clean" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build/*
    echo -e "${GREEN}Clean complete${NC}"
    
    # If only clean was requested, exit
    if [ "$2" == "" ]; then
        exit 0
    fi
fi

# Determine OS for correct SDL flags
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    SDL_CFLAGS=$(sdl2-config --cflags)
    SDL_LIBS=$(sdl2-config --libs)
    SDL_TTF_LIBS="-lSDL2_ttf"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    SDL_CFLAGS=$(sdl2-config --cflags)
    SDL_LIBS=$(sdl2-config --libs)
    SDL_TTF_LIBS="-lSDL2_ttf"
else
    # Windows or other
    echo -e "${RED}Unsupported OS. Please modify this script for your platform.${NC}"
    exit 1
fi

# Library compilation
compile "Context Engine" "g++ -c context-engine.cpp -o build/context-engine.o $SDL_CFLAGS -std=c++17 -Wall -Wextra"
if [ $? -ne 0 ]; then exit 1; fi

# Compile and link the test executable
compile "Test" "g++ -c test.cpp -o build/test.o $SDL_CFLAGS -std=c++17 -Wall -Wextra"
if [ $? -ne 0 ]; then exit 1; fi

compile "Test Executable" "g++ build/context-engine.o build/test.o -o build/test $SDL_LIBS $SDL_TTF_LIBS -std=c++17"
if [ $? -ne 0 ]; then exit 1; fi

# Copy assets to the build directory
echo -e "${YELLOW}Copying assets to build directory...${NC}"
mkdir -p build/assets
cp -r assets/* build/assets/
echo -e "${GREEN}Assets copied successfully${NC}"

# Run the game if requested
if [ "$1" == "run" ] || [ "$2" == "run" ]; then
    echo -e "${YELLOW}Running Test...${NC}"
    cd build && ./test
fi

exit 0 