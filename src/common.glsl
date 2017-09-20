/* This file is (ab)used by the C preprocessor
   to embed shaders in gfx.cpp at compile time. */
#define STRINGIFY(glsl) #glsl

STRINGIFY(
    const float mPI = 3.14159;

float ncos(float a) {
    return 0.5 + cos(a)/2;
}
float nsin(float a) {
    return 0.5 + sin(a)/2;
}
/* Via http://stackoverflow.com/a/17897228 */
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec2 adjustforaspect(vec2 point, float aspect)
{
    point.y *= aspect;
    if (aspect > 1) point /= aspect;
    return point;
}

float beat(float ticks)
{
  return nsin(radians(360.) * 97/2.0 * ticks / 60000.0);
}

)
