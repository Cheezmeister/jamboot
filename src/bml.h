#include <iostream>
#include <cmath>
#include <cstdlib>
#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef M_PI
#define M_PI 3.1415926536
#endif

#define RETURN_IF_NONZERO(do_something) \
  { int _code = do_something; if (code) return code; }

const float ROOT_2   = 1.4142135623731;
const float ROOT_2_2 = 0.707106781186548;
#define DEBUGVAR(x) bml::logger << #x " is " << x << endl;

const char ESCAPE = '\e';

typedef uint32_t u32;
typedef int32_t i32;

namespace bml
{
static std::ostream& logger = std::cout;
typedef struct _Vec {
    float x;
    float y;
} Vec;


const Vec UNIT_X = { 1, 0 };
const Vec UNIT_Y = { 0, 1 };

// http://stackoverflow.com/q/563198
static float cross(const Vec& lhs, const Vec& rhs)
{
    return lhs.x * rhs.y - rhs.x * lhs.y;
}

static float dot(const Vec& lhs, const Vec& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

static float length(const Vec& vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y);
}

static void negate(Vec& vec)
{
    vec.x = -vec.x;
    vec.y = -vec.y;
}

static Vec operator -(const Vec& unary)
{
    Vec ret = {-unary.x, -unary.y};
    return ret;
}
static Vec operator -(const Vec& lhs, const Vec& rhs)
{
    Vec ret = {lhs.x - rhs.x, lhs.y - rhs.y};
    return ret;
}
static Vec operator +(const Vec& lhs, const Vec& rhs)
{
    Vec ret = {lhs.x + rhs.x, lhs.y + rhs.y};
    return ret;
}
static void operator *=(Vec& lhs, float rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
}
static void operator -=(Vec& lhs, const Vec& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
}
static void operator +=(Vec& lhs, const Vec& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
}
static Vec operator *(float rhs, const Vec& lhs)
{
    Vec ret = {lhs.x * rhs, lhs.y * rhs};
    return ret;
}
static Vec operator *(const Vec& lhs, float rhs)
{
    Vec ret = {lhs.x * rhs, lhs.y * rhs};
    return ret;
}

static Vec project(const Vec& lhs, const Vec& rhs)
{
    return (dot(lhs, rhs) / dot(rhs, rhs)) * rhs;
}

static std::ostream& operator<<(std::ostream& lhs, const Vec& rhs)
{
    return lhs << '[' << rhs.x << ',' << rhs.y << ']';
}

static void warn(const char* message)
{
    logger << "WARNING: " << message << std::endl;
}

// random float between -1 and 1
static float normrand()
{
    return (float)(rand() % 32767) * 2.0 / (float)32767 - 1.0;
}

}


typedef struct _Input {
    bool pause;
    bool quit;

    struct _System {
        bool resized;
        struct _ResizeEvent {
            int w;
            int h;
        } resize;
    } sys;

    // Normalized (-1.0 <-> 1.0) axes
    struct _Axes {
        float x1;
        float x2;
        float x3;
        float x4;
        float y1;
        float y2;
        float y3;
        float y4;
    } axes;

    struct _Held {
        bool prime;
        bool aux;
    } held;

    struct _Action {
        bool prime;
        bool aux;
    } action;

} Input;

namespace gfx
{
void init();
void render(GameState& state, u32 ticks);
void resize(int width, int height);
}

namespace game
{
struct GameState;
GameState init();
void update(GameState& state, const Input& input);
}
