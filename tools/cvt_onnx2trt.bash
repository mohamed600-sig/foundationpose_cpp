#!/bin/bash
__root_path=/workspace
trtexec_path=/usr/src/tensorrt
${trtexec_path}/bin/trtexec --onnx=$__root_path/models/nvidia_score_model.onnx \
                              --minShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --optShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --maxShapes=input1:252x160x160x6,input2:252x160x160x6 \
                              --fp16 \
                              --saveEngine=$__root_path/models/nvidia_scorer.engine

${trtexec_path}/bin/trtexec --onnx=$__root_path/models/nvidia_refine_model.onnx \
                              --minShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --optShapes=input1:1x160x160x6,input2:1x160x160x6 \
                              --maxShapes=input1:252x160x160x6,input2:252x160x160x6 \
                              --fp16 \
                              --saveEngine=$__root_path/models/nvidia_refiner.engine
