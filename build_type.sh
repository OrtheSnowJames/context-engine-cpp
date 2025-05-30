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

# Compile the typing test game
compile "Typing Test Game" "g++ -c typing_test.cpp -o build/typing_test.o $SDL_CFLAGS -std=c++17 -Wall -Wextra"
if [ $? -ne 0 ]; then exit 1; fi

# Link the final executable
compile "Typing Test Executable" "g++ build/context-engine.o build/typing_test.o -o build/typing_test $SDL_LIBS $SDL_TTF_LIBS -std=c++17"
if [ $? -ne 0 ]; then exit 1; fi

# Copy assets to the build directory
echo -e "${YELLOW}Copying assets to build directory...${NC}"
mkdir -p build/assets
mkdir -p assets

# Copy any existing assets
cp -r assets/* build/assets/ 2>/dev/null || true

# Copy JetBrains Mono font
FONT_PATH="/usr/share/fonts/TTF/JetBrainsMono-Regular.ttf"
if [ -f "$FONT_PATH" ]; then
    echo -e "${GREEN}Copying JetBrains Mono font...${NC}"
    cp "$FONT_PATH" build/assets/JetBrainsMono-Regular.ttf
    cp "$FONT_PATH" assets/font.ttf
else
    echo -e "${RED}JetBrains Mono font not found at $FONT_PATH!${NC}"
    echo -e "${YELLOW}Looking for alternative JetBrains Mono fonts...${NC}"
    ALT_FONT=$(find /usr/share/fonts/TTF -name "JetBrainsMono-Regular.ttf" -o -name "JetBrainsMono*.ttf" | head -n 1)
    if [ -n "$ALT_FONT" ]; then
        echo -e "${GREEN}Found alternative JetBrains Mono font: $ALT_FONT${NC}"
        cp "$ALT_FONT" build/assets/JetBrainsMono-Regular.ttf
        cp "$ALT_FONT" assets/font.ttf
    else
        echo -e "${RED}No JetBrains Mono font found. Using system default font.${NC}"
    fi
fi

echo -e "${GREEN}Assets copied successfully${NC}"

# Run the game if requested
if [ "$1" == "run" ] || [ "$2" == "run" ]; then
    echo -e "${YELLOW}Running Typing Test...${NC}"
    cd build && ./typing_test
fi

exit 0