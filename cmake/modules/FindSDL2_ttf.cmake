# FindSDL2_ttf.cmake
# Finds the SDL2_ttf library
#
# This will define the following variables
#
#   SDL2_TTF_FOUND        - True if the system has SDL2_ttf
#   SDL2_TTF_INCLUDE_DIRS - SDL2_ttf include directory
#   SDL2_TTF_LIBRARIES    - SDL2_ttf libraries
#   SDL2_TTF_VERSION      - SDL2_ttf version

include(FindPackageHandleStandardArgs)

# Find SDL2_ttf headers
find_path(SDL2_TTF_INCLUDE_DIR
    NAMES SDL_ttf.h
    PATH_SUFFIXES SDL2
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
)

# Find SDL2_ttf library
find_library(SDL2_TTF_LIBRARY
    NAMES SDL2_ttf
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
)

set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})
set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})

# Get SDL2_ttf version
if(SDL2_TTF_INCLUDE_DIR AND EXISTS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h")
    file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_LINE
        REGEX "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1"
        SDL2_TTF_VERSION_MAJOR "${SDL2_TTF_VERSION_LINE}")
    file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_LINE
        REGEX "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+([0-9]+)$" "\\1"
        SDL2_TTF_VERSION_MINOR "${SDL2_TTF_VERSION_LINE}")
    file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_LINE
        REGEX "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+([0-9]+)$" "\\1"
        SDL2_TTF_VERSION_PATCH "${SDL2_TTF_VERSION_LINE}")
    set(SDL2_TTF_VERSION "${SDL2_TTF_VERSION_MAJOR}.${SDL2_TTF_VERSION_MINOR}.${SDL2_TTF_VERSION_PATCH}")
    unset(SDL2_TTF_VERSION_LINE)
    unset(SDL2_TTF_VERSION_MAJOR)
    unset(SDL2_TTF_VERSION_MINOR)
    unset(SDL2_TTF_VERSION_PATCH)
endif()

find_package_handle_standard_args(SDL2_ttf
    REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR
    VERSION_VAR SDL2_TTF_VERSION
)

mark_as_advanced(SDL2_TTF_INCLUDE_DIR SDL2_TTF_LIBRARY) 