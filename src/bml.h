#include <iostream>

#ifndef NULL
#define NULL 0
#endif

#define DEBUGVAR(x) bml::debug << #x " is " << x << endl;
const char ESCAPE = '\e';
namespace bml 
{
  static std::ostream& log = std::cout;
  typedef struct _Vec {
    float x;
    float y;
  } Vec;
}


typedef struct _Input {
    bool quit;

    // Normalized (-1.0 <-> 1.0) axes
    struct _Axes {
      float x1;
      float x2;
      float y1;
      float y2;
    } axes;

    bool pause;
    bool shoot;
    
} Input;


typedef struct _GameState {
    struct _Player {
      bml::Vec pos;
      bml::Vec vel;
      float rotation; // radians
      bml::Vec reticle;
    } player;
} GameState;

namespace gfx
{
  void init();
  void render(GameState& state);
}

namespace game
{
  void init();
  void update(GameState& state, const Input& input);
}
