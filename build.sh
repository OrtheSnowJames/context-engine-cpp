#!/bin/bash

# Define colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Default build type
BUILD_TYPE="Debug"

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
        release)
        BUILD_TYPE="Release"
        shift
        ;;
        *)
        # Unknown option
        ;;
    esac
done

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf *
fi

# Configure CMake with g++ as the compiler
echo -e "${YELLOW}Configuring CMake (${BUILD_TYPE})...${NC}"
export CXX=g++
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . -- -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build completed successfully.${NC}"
else
    echo -e "${RED}Build failed.${NC}"
    cd ..
    exit 1
fi

# Return to the original directory
cd ..

# Run the program if requested
if [ "$RUN" = true ]; then
    echo -e "${YELLOW}Running the program...${NC}"
    ./build/test
fi

echo -e "${GREEN}Done.${NC}"
echo -e "To run the program: ${YELLOW}./build/test${NC}"
echo -e "To clean and rebuild: ${YELLOW}./build.sh clean${NC}"
echo -e "To build and run: ${YELLOW}./build.sh run${NC}"
echo -e "To build release version: ${YELLOW}./build.sh release${NC}" 