#if _WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>
#include "crossgl.h"
#include "game.h"

#define GLSL_VERSION "#version 120\n"

using namespace std;
using namespace bml;

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
        cerr << "Compile failure in " << strShaderType << " shader:\n";
        cerr << strInfoLog << endl;
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
        cerr << "Linker failure: " << strInfoLog << endl;
        delete[] strInfoLog;
    }

    glDetachShader(program, vertex);
    glDetachShader(program, fragment);

    return program;
}

}

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

typedef struct _VBO {
    GLuint handle;
    GLsizei size;
} VBO;

typedef struct _RenderState {
    struct _Viewport {
        int width;
        int height;
        float aspect;
    } viewport;
    struct _Shaders {
        GLuint player;
        GLuint reticle;
    } shaders;
} RenderState;

namespace gfx
{

// Nasty globals
VBO vbo_tri;
VBO vbo_quad;

// We make the assumption that the screen starts off square.
// Actual pixel counts don't matter, only the aspect ratio
// Here, a fake 1x1 resolution has an aspect of 1.
RenderState renderstate = {1, 1, 1};

// Update a VBO
void update_vbo(VertexBuffer<3> vertexPositions, GLuint which)
{
    glBindBuffer(GL_ARRAY_BUFFER, which);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions.flat), vertexPositions.flat, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Check for GL errors
void check_error(const string& message)
{
    GLenum error = glGetError();
    if (error)
    {
        logger << "Error " << message.c_str() << ": [" << error << "] ";
        switch (error)
        {
        case 1282:
            logger << "invalid operation\n";
            return;
        default:
            logger << "unknown error\n";
            return;
        }
    }
}

// Set a uniform shader param
void set_uniform(GLuint shader, const char* name, float f)
{
    GLuint loc = glGetUniformLocation(shader, name);
    glUniform1f(loc, f);
    check_error(string("Setting " )+ name);
}
void set_uniform(GLuint shader, const char* name, float x, float y)
{
    GLuint loc = glGetUniformLocation(shader, name);
    glUniform2f(loc, x, y);
    check_error(string("Setting " )+ name);
}
void set_uniform(GLuint shader, const char* name, float x, float y, float z)
{
    GLuint loc = glGetUniformLocation(shader, name);
    glUniform3f(loc, x, y, z);
    check_error(string("Setting " )+ name);
}
void set_uniform(GLuint shader, const char* name, const Vec& v)
{
    set_uniform(shader, name, v.x, v.y);
}

// Draw a vertext array from a VBO
void draw_array(GLuint handle, GLsizei size, GLenum type)
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(type, 0, size);
    glDisableVertexAttribArray(0);
}
void draw_array(VBO vbo, GLenum type = GL_TRIANGLES)
{
    draw_array(vbo.handle, vbo.size, type);
}

// Rejigger viewport
void resize(int x, int y)
{
    renderstate.viewport.width = x;
    renderstate.viewport.height = y;
    renderstate.viewport.aspect = (float)x / (float)y;
    glViewport(0, 0, x, y);
}

GLuint make_vbo(size_t size, float* vertices)
{
    GLuint handle;
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    check_error("buffering");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return handle;
}

template<int VERTEXCOUNT>
VBO make_vbo(VertexBuffer<VERTEXCOUNT> buffer)
{
    VBO ret;
    ret.handle = make_vbo(sizeof(buffer), buffer.flat);
    ret.size = VERTEXCOUNT;
    return ret;
}

// Player shader
GLuint make_shader(GLuint vertex, GLuint fragment)
{
    GLuint program = arcsynthesis::CreateProgram(vertex, fragment);
    return program;
}

void init()
{
    // Set up triangle VBO
    VertexBuffer<3> vertexPositions = {
        0.75f, 0.0f, 0.0f, 1.0f,
        -0.75f, 0.75f, 0.0f, 1.0f,
        -0.75f, -0.75f, 0.0f, 1.0f,
    };
    vbo_tri = make_vbo(vertexPositions);

    // Set up square VBO
    VertexBuffer<4> reticleVertices = {
        0.1f,  0.1f, 0.0f, 1.0f,
        0.1f, -0.1f, 0.0f, 1.0f,
        -0.1f, -0.1f, 0.0f, 1.0f,
        -0.1f,  0.1f, 0.0f, 1.0f,
    };
    vbo_quad = make_vbo(reticleVertices);

    GLuint vs_noop = arcsynthesis::CreateShader(GL_VERTEX_SHADER, GLSL_VERSION
#include "noop.vertex.glsl"
                     );
    check_error("Compiling noop.vertex.glsl");


    GLuint vs_affine = arcsynthesis::CreateShader(GL_VERTEX_SHADER, GLSL_VERSION
#include "affine.vertex.glsl"
                       );
    check_error("Compiling affine.vertex.glsl");

    GLuint fs_pulse = arcsynthesis::CreateShader(GL_FRAGMENT_SHADER, GLSL_VERSION
#include "pulse.fragment.glsl"
                      );
    check_error("Compiling pulse.fragment.glsl");

    GLuint fs_dot = arcsynthesis::CreateShader(GL_FRAGMENT_SHADER, GLSL_VERSION
#include "dot.fragment.glsl"
                    );
    check_error("Compiling dot.fragment.glsl");

    // Init shaders
    renderstate.shaders.player = make_shader(vs_affine, fs_dot);
    renderstate.shaders.reticle = make_shader(vs_affine, fs_pulse);

    // Misc setup
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    check_error("clearcolor");
}

// Render a frame
void render(game::GameState& state, u32 ticks)
{
    // Clear
    glClear(GL_COLOR_BUFFER_BIT);
    check_error("clearing screen");

    // Render "player", drawing array from raw handle
    glUseProgram(renderstate.shaders.player);
    check_error("binding renderstate.shaders.player");
    set_uniform(renderstate.shaders.player, "aspect", renderstate.viewport.aspect);
    set_uniform(renderstate.shaders.player, "offset", state.player.pos);
    set_uniform(renderstate.shaders.player, "rotation", state.player.rotation);
    set_uniform(renderstate.shaders.player, "ticks", ticks);
    set_uniform(renderstate.shaders.player, "green", state.player.mode * 0.25);
    set_uniform(renderstate.shaders.player, "scale", 0.2 * state.player.scale);
    draw_array(vbo_tri, GL_TRIANGLES);

    glUseProgram(renderstate.shaders.reticle);
    check_error("binding renderstate.shaders.reticle");

    set_uniform(renderstate.shaders.reticle, "aspect", renderstate.viewport.aspect);
    set_uniform(renderstate.shaders.reticle, "offset", state.reticle.pos);
    set_uniform(renderstate.shaders.reticle, "rotation", ticks / 1000.0f);
    set_uniform(renderstate.shaders.reticle, "scale", state.reticle.scale);

    draw_array(vbo_quad, GL_QUADS);


}
}

