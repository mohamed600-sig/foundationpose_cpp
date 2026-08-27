#!/bin/bash

set -e

arch=$(uname -m)

build_dir="$(pwd)/build_${arch}"
TENSORRT_PATH_DIR="${TENSORRT_PATH:-}"

cmake_args=(
    "-DENABLE_TENSORRT=ON"
)

if [ -n "$TENSORRT_PATH_DIR" ]; then
    cmake_args+=("-DTENSORRT_DIR=$TENSORRT_PATH_DIR")
fi

if [ ! -d "$build_dir" ]; then
    cmake -S . -B "$build_dir" "${cmake_args[@]}"
fi

cmake --build "$build_dir"

cmake --install "$build_dir" --prefix "$build_dir/install"

lib_path="$build_dir/install/lib"

case ":${LD_LIBRARY_PATH:-}:" in
    *:"$lib_path":*)
        echo "Already in LD_LIBRARY_PATH: $lib_path"
        ;;
    *)
        export LD_LIBRARY_PATH="$lib_path${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
        echo "Added to LD_LIBRARY_PATH: $lib_path"
        ;;
esac