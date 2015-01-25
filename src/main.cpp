#include <iostream>

#include <ctime>
#include <GL/glew.h>
#include "/usr/local/Cellar/sdl2/2.0.3/include/SDL2/SDL.h"
#include <OpenGL/gl.h>

#define DEBUGVAR(x) cout << #x " is " << x << endl;
#define log std::cout

// Adapted from arsynthesis.org/gltut
namespace arcsynthesis {

    GLuint CreateShader(GLenum eShaderType, const char* strFileData)
    {
        GLuint shader = glCreateShader(eShaderType);
        glShaderSource(shader, 1, &strFileData, NULL);

        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

            const char *strShaderType = eShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment";
            fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
            delete[] strInfoLog;
        }

        return shader;
    }


    GLuint CreateProgram(GLuint vertex, GLuint fragment)
    {
        GLuint program = glCreateProgram();

        glAttachShader(program, vertex);
        glAttachShader(program, fragment);

        glLinkProgram(program);

        GLint status;
        glGetProgramiv (program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
        {
            GLint infoLogLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
            fprintf(stderr, "Linker failure: %s\n", strInfoLog);
            delete[] strInfoLog;
        }

        glDetachShader(program, vertex);
        glDetachShader(program, fragment);

        return program;
    }
}

////////////////////////////////////////////////////////////////////////////////
using namespace std;

typedef struct _Args {
    bool debug;
} Args;

typedef struct _Input {
    bool quit;
    float left;
    float right;
    float up;
    float down;
} Input;

typedef struct _GameState {
    struct _Player {
      struct _Pos {
        float x;
        float y;
      } pos;
    } player;
} GameState;

typedef union Vertex {
    float a[4];
    struct {
      float x;
      float y;
      float z;
      float w;
    };
} Vertex;

template<int N>
union VertexBuffer {
  float flat[4 * N];
  Vertex v[N];
};



SDL_Window* win;
Args args;

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
    SDL_Event event;
    Input ret;
    while (SDL_PollEvent(&event) )
    {
        if (event.type == SDL_QUIT) ret.quit = true;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ret.quit = true;
    }
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    ret.up = (keystate[SDL_SCANCODE_UP]);
    ret.down = (keystate[SDL_SCANCODE_DOWN]);
    ret.left = (keystate[SDL_SCANCODE_LEFT]);
    ret.right = (keystate[SDL_SCANCODE_RIGHT]);
    return ret;
}

void check_error(const string& message)
{
    GLenum error = glGetError();
    if (error || args.debug)
    {
        log << message << " reported error: " << error << endl;
    }
}

GLuint make_shader()
{
    GLuint vertex = arcsynthesis::CreateShader(GL_VERTEX_SHADER,
                    "#version 120  \n"
                    "attribute vec4 inPos; \n"
                    "varying vec4 glPos; \n"
                    "void main() { \n"
                    "  gl_Position = glPos = (inPos); \n"
                    "} \n"
    );
    GLuint fragment = arcsynthesis::CreateShader(GL_FRAGMENT_SHADER,
                      "#version 120 \n"
                      /* "out vec3 color; \n" */
                      "varying vec4 glPos; \n"
                      "void main() { \n"
                      "  vec3 c = cross(vec3(1, 0, 0), vec3(glPos.x, glPos.y, 0)); \n"
                      "  float green = length(c); \n"
                      "  gl_FragColor = vec4(glPos.x,green,glPos.y,0); \n"
                      "} \n"
    );
    GLuint program = arcsynthesis::CreateProgram(vertex, fragment);
    return program;
}

GLuint vbo;
GLuint shader;
void render(GameState state)
{
    // Clear
    glClear(GL_COLOR_BUFFER_BIT);
    check_error("clearing to blue");

    // Render
    glUseProgram(shader);                                   check_error("binding shader");
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                     check_error("binding buf");
    glEnableVertexAttribArray(0);                           check_error("enabling vaa");
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);  check_error("calling vap");
    glDrawArrays(GL_TRIANGLES, 0, 3);                       check_error("drawing arrays");
    glDisableVertexAttribArray(0);                          check_error("disabling vaa");

    // Commit
    SDL_GL_SwapWindow(win);

}

void update_vbo(VertexBuffer<3> vertexPositions, GLuint which)
{
    glBindBuffer(GL_ARRAY_BUFFER, which);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions.flat), vertexPositions.flat, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void loop()
{
    // Set up VBO
    VertexBuffer<3> vertexPositions = {
        0.75f, 0.75f, 0.0f, 1.0f,
        0.75f, -0.75f, 0.0f, 1.0f,
        -0.75f, -0.75f, 0.0f, 1.0f,
    };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.flat, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Init shaders
    shader = make_shader();

    // Misc setup
    glClearColor(0.0f, 1.0f, 1.0f, 0.0f);
    check_error("clearcolor");

    GameState state;

    while (true)
    {
        // Input
        Input input = handle_input();

        // Process input
        if (input.quit) break;
        if (input.right)
        {
            state.player.pos.x += 0.05;
        }

        if (input.left)
        {
            state.player.pos.x -= 0.05;
        }

        render(state);

        SDL_Delay(50);
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
