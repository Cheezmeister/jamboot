/* This file is (ab)used by the C preprocessor
   to embed shaders in gfx.cpp at compile time. */
#include "common.glsl"
STRINGIFY(

    attribute vec4 inPos;
    varying vec4 glPos;
    varying vec2 adjusted_offset;

    uniform vec2 offset;
    uniform float rotation;
    uniform float scale = 1;
    uniform float aspect = 1;

void main() {
    vec2 rotated;
    rotated.x = inPos.x * cos(rotation) - inPos.y * sin(rotation);
    rotated.y = inPos.x * sin(rotation) + inPos.y * cos(rotation);
    vec2 pos = rotated * scale;
    pos += offset;
    pos = adjustforaspect(pos, aspect);

    adjusted_offset = offset;
    adjusted_offset = adjustforaspect(adjusted_offset, aspect);

    gl_Position = glPos = vec4(pos, 0, 1);
}

)
#undef STRINGIFY
