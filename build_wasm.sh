#!/bin/bash

# Define colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo -e "${RED}Error: Emscripten (emcc) not found.${NC}"
    echo -e "Please install Emscripten SDK (emsdk) and make sure it's activated in your environment."
    echo -e "Visit https://emscripten.org/docs/getting_started/downloads.html for installation instructions."
    exit 1
fi

# Parse command line arguments
CLEAN=false
RUN=false

for arg in "$@"
do
    case $arg in
        clean)
        CLEAN=true
        shift
        ;;
        run)
        RUN=true
        shift
        ;;
        *)
        # Unknown option
        ;;
    esac
done

# Create web directory if it doesn't exist
WEB_DIR="web"
mkdir -p $WEB_DIR

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning WebAssembly output directory...${NC}"
    rm -rf $WEB_DIR/*
fi

# Configure and build with direct emcc command
echo -e "${YELLOW}Building WebAssembly file...${NC}"

# Use our custom config that disables FROZEN_CACHE
export EM_CONFIG=$(pwd)/.em-config

# Create ~/.emscripten_cache if it doesn't exist
mkdir -p ~/.emscripten_cache

# Set FROZEN_CACHE explicitly in the environment as well
export FROZEN_CACHE=0

# Build to JavaScript instead of HTML to avoid minification issues
emcc -std=c++17 -O2 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s USE_SDL=2 \
    -s USE_SDL_TTF=2 \
    -s USE_SDL_NET=2 \
    -s USE_SDL_IMAGE=2 \
    -s SDL2_IMAGE_FORMATS='["png"]' \
    -s USE_SDL_TTF=2 \
    -s ASSERTIONS=1 \
    -s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
    -s ASYNCIFY \
    -s EXIT_RUNTIME=0 \
    -s GL_POOL_TEMP_BUFFERS=0 \
    --preload-file assets \
    -I. \
    -o $WEB_DIR/index.js \
    context-engine.cpp test.cpp

# Check if build was successful
if [ $? -eq 0 ]; then
    echo -e "${GREEN}WebAssembly build completed successfully.${NC}"

    # Create an HTML file to load the generated JavaScript
    cat > $WEB_DIR/index.html << EOF
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Context Engine - WebAssembly Demo</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #121212;
            color: #ffffff;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
        }
        
        canvas {
            border: 2px solid #333;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);
            background-color: #000;
            margin: 20px;
            max-width: 100%;
            max-height: 80vh;
        }
        
        .controls {
            margin-top: 20px;
            padding: 15px;
            background-color: #1e1e1e;
            border-radius: 5px;
        }
        
        .controls h2 {
            margin-top: 0;
            font-size: 18px;
        }
        
        .controls p {
            margin: 5px 0;
        }
        
        .key {
            display: inline-block;
            padding: 5px 10px;
            background-color: #333;
            border-radius: 3px;
            margin-right: 5px;
        }
        
        #status {
            margin: 10px 0;
            padding: 10px;
            background-color: #333;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <h1>Context Engine - WebAssembly Demo</h1>
    
    <div id="status">Loading...</div>
    
    <canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>
    
    <div class="controls">
        <h2>Controls:</h2>
        <p><span class="key">←</span><span class="key">↑</span><span class="key">↓</span><span class="key">→</span> Move the player</p>
        <p><span class="key">ESC</span> Exit the game</p>
    </div>

    <script>
        var statusElement = document.getElementById('status');
        var canvas = document.getElementById('canvas');
        
        canvas.width = 800;
        canvas.height = 600;
        
        var Module = {
            print: function(text) {
                console.log(text);
            },
            printErr: function(text) {
                console.error(text);
            },
            canvas: canvas,
            setStatus: function(text) {
                statusElement.innerHTML = text;
            },
            totalDependencies: 0,
            monitorRunDependencies: function(left) {
                this.totalDependencies = Math.max(this.totalDependencies, left);
                Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
            },
            onRuntimeInitialized: function() {
                console.log('WebAssembly runtime initialized!');
                statusElement.style.display = 'none';
            }
        };
        
        window.onerror = function(message, source, lineno, colno, error) {
            console.error('Error:', message, source, lineno, colno, error);
            statusElement.innerHTML = 'Error: ' + message;
            statusElement.style.display = 'block';
            statusElement.style.backgroundColor = '#aa0000';
        };
    </script>
    <script async src="index.js"></script>
</body>
</html>
EOF

    echo -e "${YELLOW}Created custom HTML file to load the WebAssembly module.${NC}"
else
    echo -e "${RED}WebAssembly build failed.${NC}"
    exit 1
fi

# Run the program if requested
if [ "$RUN" = true ]; then
    # Change to web directory and start Python HTTP server
    echo -e "${YELLOW}Starting Python HTTP server in the web directory...${NC}"
    echo -e "${BLUE}Open your browser at http://localhost:8001/${NC}"
    echo -e "${RED}Press Ctrl+C to stop the server${NC}"
    cd $WEB_DIR
    python3 -m http.server 8001
fi

if [ "$RUN" != true ]; then
    echo -e "${GREEN}Done building WebAssembly.${NC}"
    echo -e "WebAssembly files are located in the ${YELLOW}$WEB_DIR/${NC} directory."
    echo -e "To run the WebAssembly version: ${YELLOW}./build_wasm.sh run${NC}"
    echo -e "To clean and rebuild: ${YELLOW}./build_wasm.sh clean${NC}"
fi 