#include <iostream>
#include <cstdint>

#ifndef NULL
#define NULL 0
#endif

#define DEBUGVAR(x) bml::debug << #x " is " << x << endl;
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

  static Vec operator +(const Vec& lhs, const Vec& rhs)
  {
    Vec ret = {lhs.x + rhs.x, lhs.y + rhs.y};
    return ret;
  }
  static void operator +=(Vec& lhs, const Vec& rhs)
  {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
  }
}


typedef struct _Input {
    bool quit;

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
    
    bool pause;

} Input;


typedef struct _GameState {
    struct _Player {
        bml::Vec pos;
        bml::Vec vel;
        float rotation; // radians
        float scale; // 0-2.0
        int mode;
    } player;
    struct _Reticle {
        bml::Vec pos;
        float scale; // 0-2.0
    } reticle;
} GameState;

namespace gfx
{
  void init();
  void render(GameState& state, u32 ticks);
}

namespace game
{
  void init();
  void update(GameState& state, const Input& input);
}
