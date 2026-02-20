#!bin/bash

set -e

arch=$(uname -m)

build_dir="$(pwd)/build_${arch}"

if [ ! -d "$build_dir" ]; then
    cmake -S . -B "$build_dir" -DENABLE_TENSORRT=ON
fi
cmake --build "$build_dir" 

cmake --install "$build_dir" --prefix "$build_dir/install"