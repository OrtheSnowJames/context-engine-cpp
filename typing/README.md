# Typing Speed Test

A simple typing speed test game built with SDL2 to measure your typing speed and accuracy.

## Features

- Real-time WPM (Words Per Minute) calculation
- Accuracy tracking
- Visual feedback with color-coded correct/incorrect characters
- Support for capital letters and special characters
- Screen shake effect on typing errors
- Text wrapping for long sentences
- Monospaced font for precise character alignment

## Requirements

- SDL2 and SDL2_ttf libraries
- C++17 compatible compiler (g++ recommended)

## Building and Running

1. Make sure SDL2 and SDL2_ttf are installed on your system.

   On Ubuntu/Debian:
   ```
   sudo apt-get install libsdl2-dev libsdl2-ttf-dev
   ```

   On Arch Linux:
   ```
   sudo pacman -S sdl2 sdl2_ttf
   ```

   On macOS (using Homebrew):
   ```
   brew install sdl2 sdl2_ttf
   ```

2. Build the typing test:
   ```
   chmod +x build.sh
   ./build.sh
   ```

3. Run the typing test:
   ```
   ./build.sh run
   ```

4. Clean and rebuild:
   ```
   ./build.sh clean && ./build.sh run
   ```

## How to Play

1. Press ENTER to start the typing test
2. Type the displayed sentence as quickly and accurately as possible
3. The test will end automatically when you complete the sentence
4. Press ENTER to try again with a new random sentence

## Shortcuts

- Press ESC at any time to quit the application

## Files

- `typing_test.cpp` - Main typing test game code
- `context-engine.hpp` and `context-engine.cpp` - Graphics engine
- `build.sh` - Build script
- `assets/` - Directory containing fonts and other resources

## Notes

The game keeps track of:
- WPM (Words Per Minute): Standard calculation based on 5 characters = 1 word
- Accuracy: Percentage of correctly typed characters 