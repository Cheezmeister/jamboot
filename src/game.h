#include "bml.h"

using namespace bml;

namespace game {
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

}
