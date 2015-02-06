#include <iostream>

#ifndef NULL
#define NULL 0
#endif

#define DEBUGVAR(x) bml::debug << #x " is " << x << endl;
const char ESCAPE = '\e';
namespace bml 
{
  static std::ostream& log = std::cout;
}



typedef struct _GameState {
    struct _Player {
      struct _Pos {
        float x;
        float y;
      } pos;
      struct _Pos reticle;
    } player;
} GameState;

namespace gfx
{
  void init();
  void render(GameState& state);
}
