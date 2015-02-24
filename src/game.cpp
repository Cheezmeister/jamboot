#include <cmath>
#include "bml.h"

namespace game {

void init()
{
}

void update(GameState& state, const Input& input)
{
    // Movement
    float movespeed = 0.02;
    float rotspeed = 0.1;
    float thrust = movespeed * input.axes.y1;
    state.player.rotation -= rotspeed * input.axes.x1;
    state.player.vel.x += thrust * cos(state.player.rotation);
    state.player.vel.y += thrust * sin(state.player.rotation);

    state.player.pos.x += state.player.vel.x;
    state.player.pos.y += state.player.vel.y;

    /* std::cerr << "pos " << state.player.pos.x << ' ' << state.player.pos.y << "rot " << state.player.rotation << '\n'; */

    // Aiming
    state.player.reticle.x = input.axes.x2;
    state.player.reticle.y = input.axes.y2;

}

}
