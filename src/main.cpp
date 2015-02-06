#include <iostream>
#include <ctime>
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include "/usr/local/Cellar/sdl2/2.0.3/include/SDL2/SDL.h"
#include "bml.h"

////////////////////////////////////////////////////////////////////////////////
using namespace std;

typedef struct _Args {
    bool debug;
} Args;

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

typedef struct _RenderState {
  struct _SdlRenderState {
    struct _Viewport {
      int x;
      int y;
    } viewport;
  } sdl;
} RenderState;


// Ugly nasty globals
SDL_Window* win;
Args args;
RenderState rs;

int parse_args(int argc, char** argv, Args* outArgs)
{
    for (int i = 0; i < argc; ++i)
    {
        char* arg = argv[i];
        char first = arg[0];
        if (first == '-')
        {
            if (arg[1] == 'd')
                outArgs->debug = true;
        }
    }

    return 0;
}

Input handle_input()
{
    Input ret;

    // Poll events
    SDL_Event event;
    while (SDL_PollEvent(&event) )
    {
        if (event.type == SDL_QUIT) ret.quit = true;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ret.quit = true;
        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                int x = event.window.data1;
                int y = event.window.data2;
                rs.sdl.viewport.x = x;
                rs.sdl.viewport.y = x;
                int max = x > y ? x : y;
                glViewport(0, 0, max, max);
            }
        }
    }

    // Poll mouse
    struct _Mouse {
      Uint8 buttons;
      int x;
      int y;
    } mouse;
    mouse.buttons = SDL_GetMouseState(&mouse.x, &mouse.y);

    ret.shoot = (mouse.buttons & SDL_BUTTON(1));
    ret.axes.x2 = mouse.x * 2.0 / rs.sdl.viewport.x - 1.0;
    ret.axes.y2 = mouse.y * 2.0 / rs.sdl.viewport.y - 1.0;
    ret.axes.y2 *= -1;


    // Poll keyboard
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    ret.axes.y1  = 1.0 * (keystate[SDL_SCANCODE_UP]);
    ret.axes.y1 -= 1.0 * (keystate[SDL_SCANCODE_DOWN]);
    ret.axes.x1  = 1.0 * (keystate[SDL_SCANCODE_RIGHT]);
    ret.axes.x1 -= 1.0 * (keystate[SDL_SCANCODE_LEFT]);
    return ret;
}


void loop()
{

    gfx::init();

    GameState state = {0};

    while (true)
    {
        // Input
        Input input = handle_input();

        // Process gameplay
        if (input.quit) break;
        float movespeed = 0.2;
        state.player.pos.x += movespeed * input.axes.x1;
        state.player.pos.y += movespeed * input.axes.y1;

        state.player.reticle.x = input.axes.x2;
        state.player.reticle.y = input.axes.y2;

        // Render graphics
        gfx::render(state);

        // Commit
        SDL_GL_SwapWindow(win);

        // Finish frame
        SDL_Delay(20);
    }
}

void print_info()
{
    SDL_version version;
    SDL_VERSION(&version);
    cout << "SDL version: " << (int)version.major << "." << (int)version.minor << (int)version.patch << endl;
    SDL_GetVersion(&version);
    cout << "runtime version: " << (int)version.major << "." << (int)version.minor << (int)version.patch << endl;
    printf("OpenGL vendor: '%s'\n" , glGetString(GL_VENDOR));
    printf("OpenGL renderer: '%s'\n" , glGetString(GL_RENDERER));
    printf("OpenGL version: '%s'\n" , glGetString(GL_VERSION));
    printf("GLSL version: '%s'\n" , glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GLEW version: %s\n", glewGetString(GLEW_VERSION));
}

int scratch() 
{
  return 0;
}
int main ( int argc, char** argv )
{
    int ret = parse_args(argc, argv, &args);
    if (ret) return ret;

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cerr << "Couldn't init SDL";
        return 1;
    }

    win = SDL_CreateWindow("SDL2/GL4.3", 0, 0, 200, 200, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (win == NULL)
    {
        cerr << "Couldn't set video mode";
        return 2;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GLContext context = SDL_GL_CreateContext(win);
    if (context == NULL)
    {
        cerr << "Couldn't get a gl context: " << SDL_GetError();
        return 3;
    }

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 4;
    }

    print_info();

    loop();

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
