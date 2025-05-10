cmake_minimum_required(VERSION 3.10)
project(ContextEngine VERSION 1.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add custom modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Check if we're building with Emscripten
if(EMSCRIPTEN)
    message(STATUS "Building with Emscripten for WebAssembly")
    set(USING_EMSCRIPTEN ON)
    set(SDL_FOR_EMSCRIPTEN ON)
    set(WEB_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/web")
    
    # Emscripten compile options
    set(EMSCRIPTEN_OPTS
        "-s USE_SDL=2"
        "-s USE_SDL_IMAGE=2"
        "-s USE_SDL_TTF=2"
        "-s SDL2_IMAGE_FORMATS='[\"png\",\"jpg\"]'"
        "-s WASM=1"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s NO_EXIT_RUNTIME=1"
        "-s ASSERTIONS=1"
        "-s EXPORTED_RUNTIME_METHODS=['cwrap']"
    )
    
    # Add Emscripten compile definitions
    add_compile_definitions(__EMSCRIPTEN__)
else()
    # Prefer g++ compiler if available
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(STATUS "Using preferred compiler: g++")
    else()
        message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()

    # Find SDL2 package using our custom module
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    # Find SDL2_ttf package
    find_package(SDL2_ttf REQUIRED)
    include_directories(${SDL2_TTF_INCLUDE_DIRS})
endif()

# Create a library for the Context Engine
if(EMSCRIPTEN)
    add_library(ContextEngine STATIC
        context-engine.cpp
    )
    
    # Set include directories for the library - Emscripten includes SDL2 automatically
    target_include_directories(ContextEngine PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Add compile options for Emscripten
    target_compile_options(ContextEngine PRIVATE -Wall -Wextra)
    
    # Add the WebAssembly executable
    add_executable(test.html test.cpp)
    
    # Set Emscripten-specific properties
    set_target_properties(test.html PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "${EMSCRIPTEN_OPTS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets -s DISABLE_DEPRECATED_FUNCTIONS=0"
        RUNTIME_OUTPUT_DIRECTORY "${WEB_OUTPUT_DIR}"
    )
    
    # Link the WebAssembly executable with our library
    target_link_libraries(test.html PRIVATE ContextEngine)
    
    # Set compiler flags for the WebAssembly executable
    target_compile_options(test.html PRIVATE -Wall -Wextra)
    
    # Create directory for web output
    file(MAKE_DIRECTORY ${WEB_OUTPUT_DIR})
    
    # Copy index.html to the output directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/web_template/index.html ${WEB_OUTPUT_DIR}/index.html COPYONLY)
    
    # Add a custom target for easy building of the web version
    add_custom_target(web DEPENDS test.html)
    
else()
    add_library(ContextEngine STATIC
        context-engine.cpp
    )
    
    # Set include directories for the library
    target_include_directories(ContextEngine PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
    )
    
    # Link the library with SDL2 and SDL2_ttf
    target_link_libraries(ContextEngine PUBLIC 
        ${SDL2_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
    )
    
    # Add the test executable
    add_executable(test test.cpp)
    
    # Link the test executable with our library
    target_link_libraries(test PRIVATE ContextEngine)
    
    # Set compiler flags
    target_compile_options(ContextEngine PRIVATE -Wall -Wextra)
    target_compile_options(test PRIVATE -Wall -Wextra)
    
    # Installation
    install(TARGETS test ContextEngine
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    
    install(FILES context-engine.hpp DESTINATION include)
endif()

# Output message after configuration
if(NOT EMSCRIPTEN)
    message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
    message(STATUS "SDL2 include directories: ${SDL2_INCLUDE_DIRS}")
    message(STATUS "SDL2_ttf libraries: ${SDL2_TTF_LIBRARIES}")
    message(STATUS "SDL2_ttf include directories: ${SDL2_TTF_INCLUDE_DIRS}")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}") 