#include <cmath>
#include "bml.h"

namespace game {

struct _GameParams {
    float movespeed, rotspeed, drag, modecount, scalesize;
} params = {   0.01,      0.1,  0.9,         5, 12 };

void init()
{
}

void update(GameState& state, const Input& input)
{
    // Movement with arrow keys/fake axis
    float thrust = params.movespeed * input.axes.y1;
    state.player.rotation -= params.rotspeed * input.axes.x1;
    state.player.vel.x += thrust * cos(state.player.rotation);
    state.player.vel.y += thrust * sin(state.player.rotation);

    state.player.pos += state.player.vel;
    state.player.vel.x *= params.drag;
    state.player.vel.y *= params.drag;

    // Aim with mouse
    state.reticle.pos.x = input.axes.x2;
    state.reticle.pos.y = input.axes.y2;

    // Warp/drag with mouse
    if (input.held.prime)
        state.player.pos = state.reticle.pos;

    // Scale with gamepad sticks
    state.player.scale = input.axes.y3 + 1.0;
    float y4 = input.axes.y4;
    if (y4 > 0) y4 *= params.scalesize;
    state.reticle.scale = y4 + 1.0;

    // Cycle greenness with gamepad A or spacebar
    if (input.action.prime)
        (++state.player.mode) %= (int)params.modecount;

}

}
