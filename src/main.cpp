#if _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <ctime>
#include "SDL.h"
#include <GL/glew.h>
#include "crossgl.h"
#include "bml.h"

////////////////////////////////////////////////////////////////////////////////
using namespace std;

typedef struct _Args {
    bool debug;
} Args;

typedef struct _Dimension2 {
  float x;
  float y;
} Dimension2;


// Ugly nasty globals
SDL_Window* win;
SDL_GameController* controller = NULL;
Args args;
Dimension2 viewport;

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
    Input ret = {0};

    // Process events first
    SDL_Event event;
    while (SDL_PollEvent(&event) )
    {
        if (event.type == SDL_QUIT) ret.quit = true;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ret.quit = true;

        // Window resize
        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                int x = event.window.data1;
                int y = event.window.data2;
                viewport.x = x;
                viewport.y = y;
                gfx::resize(x, y);
                ret.sys.resized = true;
                ret.sys.resize.w = x;
                ret.sys.resize.h = y;
            }
        }

        // Gamepad buttons
        if (event.type == SDL_CONTROLLERBUTTONDOWN)
        {
            if (event.cbutton.button & SDL_CONTROLLER_BUTTON_A)
            {
                ret.action.prime = true;
            }
        }
        // Keyboard presses
        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                ret.action.prime = true;
            }
        }
    }

    // Poll the current state of the mouse
    struct _Mouse {
        Uint8 buttons;
        int x;
        int y;
    } mouse;
    mouse.buttons = SDL_GetMouseState(&mouse.x, &mouse.y);
    ret.held.prime |= (mouse.buttons & SDL_BUTTON(1));
    ret.axes.x2 = mouse.x * 2.0 / viewport.x - 1.0;
    ret.axes.y2 = mouse.y * 2.0 / viewport.y - 1.0;
    ret.axes.y2 *= -1;

    // Poll the current state of the keyboard
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    ret.axes.y1  = 1.0 * (keystate[SDL_SCANCODE_UP]);
    ret.axes.y1 -= 1.0 * (keystate[SDL_SCANCODE_DOWN]);
    ret.axes.x1  = 1.0 * (keystate[SDL_SCANCODE_RIGHT]);
    ret.axes.x1 -= 1.0 * (keystate[SDL_SCANCODE_LEFT]);

    // Poll the gamepad sticks
    ret.axes.x3 = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / (float)32767;
    ret.axes.x4 = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / (float)32767;
    ret.axes.y3 = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / (float)32767;
    ret.axes.y4 = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / (float)32767;

    // Poll gamepad buttons
    ret.held.aux = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

    return ret;
}


void enter_main_loop()
{

    gfx::init();
    game::init();

    GameState state = {0};

    while (true)
    {
        // Timing
        u32 ticks = SDL_GetTicks();

        // Input
        Input input = handle_input();
        if (input.quit) break;

        // Process gameplay
        game::update(state, input);

        // Render graphics
        gfx::render(state, ticks);

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

extern "C"
int main(int argc, char** argv )
{
    int ret = parse_args(argc, argv, &args);
    if (ret) return ret;

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        cerr << "Couldn't init SDL";
        return 1;
    }

    viewport.x = 200;
    viewport.y = 200;
    win = SDL_CreateWindow("SDL2/GL2.1", 0, 0, 200, 200, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

    SDL_ShowCursor(SDL_DISABLE);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 4;
    }

    print_info();

    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            cout << "Detected controller in slot " << i << endl;
            controller = SDL_GameControllerOpen(i);
        }
    }

    SDL_ShowCursor(SDL_DISABLE);

    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            controller = SDL_GameControllerOpen(i);
        }
    }

    enter_main_loop();

    if (controller) SDL_GameControllerClose(controller);
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
