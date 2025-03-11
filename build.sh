#!/usr/bin/env bash

# Use: './build.sh <path to C compiler> <path to CXX compiler> [Debug|Release]'
# C:\Another\MSYS2\mingw64\bin\gcc.exe C:\Another\MSYS2\mingw64\bin\g++.exe 
# Default: Debug


## Process input arguments
COMPILER_C="$1"
COMPILER_CXX="$2"
BUILD_TYPE="Debug"
if (( $# > 3 )); then
    BUILD_TYPE= $3
fi

if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo "Build type must be one of [ Debug, Release ], got $BUILD_TYPE" 1>&2
    exit 1
fi

## Dir variables
BUILD_ROOT="$(cd "./" && pwd)"
echo $BUILD_ROOT

BUILD_DIR=""
if [[ "$BUILD_TYPE" == "Debug" ]]; then
    BUILD_DIR="build_debug"
elif [[ "$BUILD_TYPE" == "Release" ]]; then
    BUILD_DIR="build_release"
fi


## CMake arguments
CMAKE_FLAGS_CONFIG=""
if [[ "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" ]]; then
    BUILD_DIR="$BUILD_DIR/win"
    CMAKE_FLAGS_CONFIG="${CMAKE_FLAGS_CONFIG} -DGLFW_BUILD_WAYLAND=Off -DGLFW_BUILD_X11=Off -G 'MinGW Makefiles'"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    BUILD_DIR="$BUILD_DIR/linux"
    CMAKE_FLAGS_CONFIG="${CMAKE_FLAGS_CONFIG} -DGLFW_BUILD_WAYLAND=Off -DGLFW_BUILD_X11=Off"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    BUILD_DIR="$BUILD_DIR/mac"
else
    BUILD_DIR="$BUILD_DIR/def"
fi


CMAKE_FLAGS_CONFIG="${CMAKE_FLAGS_CONFIG} -DCMAKE_C_COMPILER='$COMPILER_C' -DCMAKE_CXX_COMPILER='$COMPILER_CXX' -DCMAKE_BUILD_TYPE='$BUILD_TYPE'"
# Note: override overriding build location
CMAKE_FLAGS_CONFIG="${CMAKE_FLAGS_CONFIG} -DSVKFW_OVERRIDE_RES_LOCATION=Off -DCMAKE_RUNTIME_OUTPUT_DIRECTORY='$BUILD_ROOT/$BUILD_DIR' -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY='$BUILD_ROOT/$BUILD_DIR/lib' -DCMAKE_LIBRARY_OUTPUT_DIRECTORY='$BUILD_ROOT/$BUILD_DIR/lib'"
CMAKE_FLAGS_BUILD="--build $BUILD_DIR/build --target svkfw_test -j8"

## CMake config + build
cmake -S"$BUILD_ROOT" -B"$BUILD_ROOT/$BUILD_DIR/build" ${CMAKE_FLAGS_CONFIG}
cmake $CMAKE_FLAGS_BUILD