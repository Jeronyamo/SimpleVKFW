#!/usr/bin/env bash

# Use: './build.sh [<path to C compiler> <path to CXX compiler>] [Debug|Release]'
# C:\Another\MSYS2\mingw64\bin\gcc.exe C:\Another\MSYS2\mingw64\bin\g++.exe 
# Default: Debug, compilers from the preset (Windows: gcc/g++ from PATH, Linux/Mac: CMake default)
# Note: the presets are in CMakePresets.json, build files go to build_<BuildType>/<os>/build


## Process input arguments
COMPILER_C=""
COMPILER_CXX=""
BUILD_TYPE="Debug"
if (( $# >= 2 )); then
    COMPILER_C="$1"
    COMPILER_CXX="$2"
    shift 2
fi
if (( $# >= 1 )); then
    BUILD_TYPE="$1"
fi

if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo "Build type must be one of [ Debug, Release ], got $BUILD_TYPE" 1>&2
    exit 1
fi

## Preset name: <os>-<build type>
BUILD_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo $BUILD_ROOT

if [[ "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" ]]; then
    PRESET_OS="win"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PRESET_OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PRESET_OS="mac"
else
    echo "Unknown OS type: $OSTYPE" 1>&2
    exit 1
fi
PRESET="$PRESET_OS-${BUILD_TYPE,,}"


## CMake arguments
# Note: arrays keep arguments with spaces (compiler paths) in one piece
CMAKE_FLAGS_CONFIG=(--preset "$PRESET")
if [[ -n "$COMPILER_C" ]]; then
    CMAKE_FLAGS_CONFIG+=("-DCMAKE_C_COMPILER=$COMPILER_C" "-DCMAKE_CXX_COMPILER=$COMPILER_CXX")
fi
# Note: GLFW builds both X11 and Wayland by default and picks one at runtime.
#       Set SVKFW_GLFW_WAYLAND=Off to build without the Wayland dev packages.
if [[ "$PRESET_OS" == "linux" ]]; then
    CMAKE_FLAGS_CONFIG+=("-DGLFW_BUILD_WAYLAND=${SVKFW_GLFW_WAYLAND:-On}")
fi
CMAKE_FLAGS_BUILD=(--build --preset "$PRESET" --target svkfw svkfw_test)

## CMake config + build
cd "$BUILD_ROOT" || exit 1
cmake "${CMAKE_FLAGS_CONFIG[@]}" || exit 1
cmake "${CMAKE_FLAGS_BUILD[@]}"
