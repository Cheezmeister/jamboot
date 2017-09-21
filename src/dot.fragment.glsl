#include "common.glsl"
STRINGIFY(

    varying vec4 glPos;

    varying vec2 adjusted_offset;
    uniform float ticks;

void main() {
    float d = distance(vec2(glPos), adjusted_offset);
    float t = 1.0 - clamp(d * 10, 0, 1);
    float i = beat(ticks) * 4 * pow(t,  5);
    float b = t > 0 && t > 0.85 ? 1.0 : 0;
    b *= beat(ticks * 2);
    float b2 = t > 0 && t < 0.05 ? 1 : 0;
    gl_FragColor = vec4(0, i, b , i);
}

) // STRINGIFY
