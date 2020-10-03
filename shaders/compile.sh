#!/bin/bash

vert_glsl=shader.vert
vert_spv=vert.spv
frag_glsl=shader.frag
frag_spv=frag.spv

glslc $vert_glsl -o $vert_spv
glslc $frag_glsl -o $frag_spv

