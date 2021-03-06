#include "bml.h"

using namespace bml;

const int VIEWPORT_WIDTH = 400;
const int VIEWPORT_HEIGHT = 300;


namespace game
{
typedef struct _GameState {
    struct _Player {
        Vec pos;
        Vec vel;
        float rotation; // radians
        float scale; // 0-2.0
        int mode;
    } player;
    struct _Reticle {
        Vec pos;
        float scale; // 0-2.0
    } reticle;
    struct _Field {
        float w;
        float h;
    } field;
} GameState;

void init();
void update(GameState& state, const Input& input);
}

namespace gfx
{
void init();
void render(game::GameState& state, u32 ticks);
void resize(int width, int height);
}
