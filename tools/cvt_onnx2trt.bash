#!/bin/bash
__root_path=/workspace
trtexec_path=
#/usr/src/tensorrt
${trtexec_path}/bin/trtexec --onnx=$__root_path/models/scorer_hwc.onnx \
                              --minShapes=render_input:1x160x160x6,transf_input:1x160x160x6 \
                              --optShapes=render_input:252x160x160x6,transf_input:252x160x160x6 \
                              --maxShapes=render_input:252x160x160x6,transf_input:252x160x160x6 \
                              --fp16 \
                              --saveEngine=$__root_path/models/scorer_hwc_dynamic_fp16.engine

${trtexec_path}/bin/trtexec --onnx=$__root_path/models/refiner_hwc.onnx \
                              --minShapes=render_input:1x160x160x6,transf_input:1x160x160x6 \
                              --optShapes=render_input:252x160x160x6,transf_input:252x160x160x6 \
                              --maxShapes=render_input:252x160x160x6,transf_input:252x160x160x6 \
                              --fp16 \
                              --saveEngine=$__root_path/models/refiner_hwc_dynamic_fp16.engine
