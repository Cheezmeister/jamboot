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
} RenderState;

namespace gfx
{

    // Nasty globals
    GLuint vbo;
    VBO reticle_vbo;
    GLuint shader;
    GLuint reticle_shader;

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
				glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); check_error("buffering");
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
        // Set up triangle VBO
        VertexBuffer<3> vertexPositions = {
             0.75f, 0.0f, 0.0f, 1.0f,
            -0.75f, 0.75f, 0.0f, 1.0f,
            -0.75f, -0.75f, 0.0f, 1.0f,
        };
        vbo = make_vbo(sizeof(vertexPositions), vertexPositions.flat);

        // Set up square VBO
        VertexBuffer<4> reticleVertices = {
            0.1f,  0.1f, 0.0f, 1.0f,
            0.1f, -0.1f, 0.0f, 1.0f,
            -0.1f, -0.1f, 0.0f, 1.0f,
            -0.1f,  0.1f, 0.0f, 1.0f,
        };
        reticle_vbo = make_vbo(reticleVertices);

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
        glClear(GL_COLOR_BUFFER_BIT); check_error("clearing screen");

        // Render "player", drawing array from raw handle
        glUseProgram(shader); check_error("binding shader");
        set_uniform(shader, "aspect", renderstate.viewport.aspect);
        set_uniform(shader, "offset", state.player.pos);
        set_uniform(shader, "rotation", state.player.rotation);
        set_uniform(shader, "ticks", ticks);
        set_uniform(shader, "green", state.player.mode * 0.25);
        set_uniform(shader, "scale", 0.2 * state.player.scale);
				draw_array(vbo, 3, GL_TRIANGLES);

        // Render reticle, drawing from VBO struct
        glUseProgram(reticle_shader); check_error("binding shader");
        set_uniform(reticle_shader, "aspect", renderstate.viewport.aspect);
        set_uniform(reticle_shader, "offset", state.reticle.pos);
        set_uniform(reticle_shader, "rotation", ticks / 1000.0f);
        set_uniform(reticle_shader, "scale", state.reticle.scale);
        draw_array(reticle_vbo, GL_QUADS);

        // Draw centered 1-unit square, reusing reticle vertices
        set_uniform(reticle_shader, "offset", 0, 0);
        set_uniform(reticle_shader, "scale", 5);
        set_uniform(reticle_shader, "rotation", 0);
        draw_array(reticle_vbo, GL_QUADS);
    }
}

