#!/usr/bin/env bash

#TODO: --config, -j -- choose in CMake/Automatically
BUILD_DIR="$( dirname "${BASH_SOURCE[0]}")/build/"

if [[ "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" ]]; then
    BUILD_DIR="${BUILD_DIR}win/build/"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    BUILD_DIR="${BUILD_DIR}linux/build/"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    BUILD_DIR="${BUILD_DIR}mac/build/"
else
    BUILD_DIR="${BUILD_DIR}def/build/"
fi

cmake -S"$( dirname "${BASH_SOURCE[0]}")/" -B"${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target svkfw_test --config Debug -j 10