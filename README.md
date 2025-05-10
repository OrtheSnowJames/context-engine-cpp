# Context Engine C++

A dead simple 2D game engine built with SDL2 in C++, with WebAssembly support.

## Overview

Context Engine is a lightweight 2D game engine that provides:
- Abstraction away from SDL while keeping cross-platformness
- Window and renderer management
- Scene management system
- Input handling
- Basic rendering primitives (rectangles, lines, points)
- WebAssembly compilation support

## Requirements

### Native Build
- C++17 compatible compiler (g++ preferred)
- SDL2 development libraries
- CMake 3.10 or higher

### WebAssembly Build
- Emscripten SDK (emsdk)
- Python 3 (for hosting)

## Building the Project

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get install cmake g++ libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

#### Fedora
```bash
sudo dnf install cmake gcc-c++ SDL2-devel SDL2_image SDL2_ttf
```

#### Arch Linux
```bash
sudo pacman -S cmake gcc sdl2 sdl2_ttf sdl2_image
```

#### Emscripten for WebAssembly
```bash
# Clone the repository
git clone https://github.com/emscripten-core/emsdk.git

# Enter the directory
cd emsdk

# Download and install the latest SDK tools
./emsdk install latest

# Activate the latest SDK tools
./emsdk activate latest

# Set up the environment variables
source ./emsdk_env.sh

# You may want to add this to your .bashrc or .zshrc
# echo 'source "/path/to/emsdk/emsdk_env.sh"' >> ~/.bashrc
```

### Building with the Script (Native)

The project includes a build script that simplifies the build process:

```bash
# Regular build
./build.sh

# Clean build (removes all previous build artifacts)
./build.sh clean

# Build and run
./build.sh run

# Build release version
./build.sh release
```

### Building for WebAssembly

The project includes a separate script for WebAssembly builds:

```bash
# Regular WebAssembly build
./build_wasm.sh

# Clean build
./build_wasm.sh clean

# Build and run (starts a Python HTTP server)
./build_wasm.sh run
```

When running with `./build_wasm.sh run`, the script will start a Python HTTP server at `http://localhost:8000`, where you can access the WebAssembly version of the game in your browser.

### Manual CMake Build

If you prefer to use CMake directly:

#### Native build
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

#### WebAssembly build
```bash
mkdir -p build_wasm && cd build_wasm
emcmake cmake ..
emmake cmake --build . --target web
```

## Running the Engine

### Native Version
```bash
./build/test
```

### WebAssembly Version
```bash
cd web
python3 -m http.server
# Open your browser at http://localhost:8000
```

## Key Controls

- Arrow keys: Move the player
- ESC: Exit the game 