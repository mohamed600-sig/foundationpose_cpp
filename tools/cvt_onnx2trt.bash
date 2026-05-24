#!/bin/bash
set -e 

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

readonly __root_path="$(cd "$SCRIPT_DIR/.." && pwd)"

TRTEXEC="trtexec"

if [ -x "/usr/src/tensorrt/bin/trtexec" ]; then
    TRTEXEC="/usr/src/tensorrt/bin/trtexec"
elif ! command -v trtexec > /dev/null; then
    echo "[ERROR] can't find  'trtexec' command " 
    exit 1
fi

"${TRTEXEC}" --onnx=$__root_path/models/nvidia_score_model.onnx \
                              --minShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --optShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --maxShapes=input1:252x160x160x6,input2:252x160x160x6 \
                              --fp16 \
                              --saveEngine=$__root_path/models/nvidia_scorer.engine

"${TRTEXEC}" --onnx=$__root_path/models/nvidia_refine_model.onnx \
                              --minShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --optShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --maxShapes=input1:252x160x160x6,input2:252x160x160x6 \
                              --fp16 \
                              --saveEngine=$__root_path/models/nvidia_refiner.engine
