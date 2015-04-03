#if _WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>
#include "crossgl.h"
#include "bml.h"

// http://stackoverflow.com/a/13874526
#define MAKE_SHADER(version, shader)  "#version " #version "\n" #shader

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

typedef struct _RenderState {
  struct _Viewport {
    int width;
    int height;
    int max;
  } viewport;
} RenderState;

namespace gfx
{

    // Nasty globals
    GLuint vbo;
    GLuint reticle_vbo;
    GLuint shader;
    GLuint reticle_shader;

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
    void set_uniform(GLuint shader, const string& name, float f)
    {
        GLuint loc = glGetUniformLocation(shader, name.c_str());
        glUniform1f(loc, f);
    }
    void set_uniform(GLuint shader, const string& name, float x, float y)
    {
        GLuint loc = glGetUniformLocation(shader, name.c_str());
        glUniform2f(loc, x, y);
    }
    void set_uniform(GLuint shader, const string& name, const Vec& v)
    {
        set_uniform(shader, name, v.x, v.y);
    }

    // Rejigger viewport
    void resize(int x, int y)
    {
        int max = x > y ? x : y;
        renderstate.viewport.width = x;
        renderstate.viewport.height = y;
        renderstate.viewport.max = max;
        glViewport(0, 0, x, y);
    }

    float get_aspect()
    {
        float width = renderstate.viewport.width;
        float height = renderstate.viewport.height;
        return width / height;
    }

    // Player shader
    GLuint make_shader()
    {
        GLuint vertex = arcsynthesis::CreateShader(GL_VERTEX_SHADER,
                        "#version 120  \n"
                        "attribute vec4 inPos; \n"
                        "uniform vec2 offset; \n"
                        "uniform float rotation; \n"
                        "uniform float ticks; \n"
                        "uniform float scale; \n"
                        "varying vec4 glPos; \n"
                        "uniform float aspect = 1; \n"
                        "void main() { \n"
                        "  vec2 rotated;\n"
                        "  rotated.x = inPos.x * cos(rotation) - inPos.y * sin(rotation);\n"
                        "  rotated.y = inPos.x * sin(rotation) + inPos.y * cos(rotation);\n"
                        "  vec2 pos = rotated * scale;\n"
                        "  pos += offset; \n"
                        "  pos.y *= aspect; \n"
                        "  if (aspect > 1) pos /= aspect; \n"
                        "  gl_Position = glPos = vec4(pos, 0, 1); \n"
                        "} \n"
        );
        GLuint fragment = arcsynthesis::CreateShader(GL_FRAGMENT_SHADER,
                          "#version 120 \n"
                          /* "out vec3 color; \n" */
                          "varying vec4 glPos; \n"
                          "uniform float green; \n"
                          "void main() { \n"
                          "  vec3 c = cross(vec3(1, 0, 0), vec3(glPos.x, glPos.y, 0)); \n"
                          "  float r = length(c); \n"
                          "  gl_FragColor = vec4(r,green,glPos.y,0); \n"
                          "} \n"
        );
        GLuint program = arcsynthesis::CreateProgram(vertex, fragment);
        return program;
    }
    void init()
    {
        // Set up VBO
        VertexBuffer<3> vertexPositions = {
             0.75f, 0.0f, 0.0f, 1.0f,
            -0.75f, 0.75f, 0.0f, 1.0f,
            -0.75f, -0.75f, 0.0f, 1.0f,
        };
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.flat, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        VertexBuffer<4> reticleVertices = {
            0.1f,  0.1f, 0.0f, 1.0f,
            0.1f, -0.1f, 0.0f, 1.0f,
            -0.1f, -0.1f, 0.0f, 1.0f,
            -0.1f,  0.1f, 0.0f, 1.0f,
        };
        glGenBuffers(1, &reticle_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, reticle_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(reticleVertices), reticleVertices.flat, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Init shaders
        shader = make_shader();
        reticle_shader = make_shader();

        // Misc setup
        glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
        check_error("clearcolor");
    }

    // Render a frame
    void render(GameState& state, u32 ticks)
    {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT);
        check_error("clearing to blue");

        // Render "player"
        glUseProgram(shader);                                     check_error("binding shader");
        set_uniform(shader, "aspect", get_aspect());      check_error("getting param");
        GLuint loc = glGetUniformLocation(shader, "offset");      check_error("getting param 'offset'");
        glUniform2f(loc, state.player.pos.x, state.player.pos.y); check_error("setting uniform");
        set_uniform(shader, "rotation", state.player.rotation);   check_error("setting uniform");
        set_uniform(shader, "ticks", ticks / 100.0f);             check_error("setting ticks");
        set_uniform(shader, "green", state.player.mode * 0.25);   check_error("setting green");
        set_uniform(shader, "scale", 0.2 * state.player.scale);   check_error("setting scale");
        glBindBuffer(GL_ARRAY_BUFFER, vbo);                       check_error("binding buf");
        glEnableVertexAttribArray(0);                             check_error("enabling vaa");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);    check_error("calling vap");
        glDrawArrays(GL_TRIANGLES, 0, 3);                         check_error("drawing arrays");
        glDisableVertexAttribArray(0);                            check_error("disabling vaa");

        // Render reticle
        glUseProgram(reticle_shader);                             check_error("binding shader");
        set_uniform(reticle_shader, "aspect", get_aspect());      check_error("getting param");
        DEBUGVAR(state.reticle.scale);
        set_uniform(reticle_shader, "offset", state.reticle.pos); check_error("getting param");
        set_uniform(reticle_shader, "scale", state.reticle.scale); check_error("setting scale");

        glBindBuffer(GL_ARRAY_BUFFER, reticle_vbo);               check_error("binding buf");
        glEnableVertexAttribArray(0);                             check_error("enabling vaa");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);    check_error("calling vap");
        glDrawArrays(GL_QUADS, 0, 4);                             check_error("drawing arrays");
        glDisableVertexAttribArray(0);                            check_error("disabling vaa");

        loc = glGetUniformLocation(reticle_shader, "offset");     check_error("getting param");
        glUniform2f(loc, 0, 0); check_error("setting uniform");
        loc = glGetUniformLocation(shader, "scale");              check_error("getting param 'scale'");
        glUniform1f(loc, 5);                                      check_error("setting scale");
        glBindBuffer(GL_ARRAY_BUFFER, reticle_vbo);               check_error("binding buf");
        glEnableVertexAttribArray(0);                             check_error("enabling vaa");
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);    check_error("calling vap");
        glDrawArrays(GL_QUADS, 0, 4);                             check_error("drawing arrays");
        glDisableVertexAttribArray(0);                            check_error("disabling vaa");
    }
}

