#include <epoxy/gl.h>
#include <SDL2/SDL.h>

#include <array>
#include <iostream>
#include <string>

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;

constexpr char kVertexShaderSource[] = R"(
#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

out vec3 vertexColor;

void main() {
  vertexColor = color;
  gl_Position = vec4(position, 0.0, 1.0);
}
)";

constexpr char kFragmentShaderSource[] = R"(
#version 330 core

in vec3 vertexColor;
out vec4 fragmentColor;

void main() {
  fragmentColor = vec4(vertexColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* source) {
  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint success = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(logLength), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error("Shader compilation failed: " + log);
  }

  return shader;
}

GLuint createShaderProgram() {
  const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, kVertexShaderSource);
  const GLuint fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLint success = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(logLength), '\0');
    glGetProgramInfoLog(program, logLength, nullptr, log.data());
    glDeleteProgram(program);
    throw std::runtime_error("Shader link failed: " + log);
  }

  return program;
}

}  // namespace

int main(int, char**) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Window* window = SDL_CreateWindow(
      "OpenGL 3.3 Core Triangle",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      kWindowWidth,
      kWindowHeight,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  if (window == nullptr) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
    SDL_Quit();
    return 1;
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (context == nullptr) {
    std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << '\n';
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_GL_SetSwapInterval(1);
  glViewport(0, 0, kWindowWidth, kWindowHeight);

  std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
  std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

  GLuint shaderProgram = 0;
  GLuint vertexArray = 0;
  GLuint vertexBuffer = 0;

  try {
    shaderProgram = createShaderProgram();

    constexpr std::array<float, 15> vertices{
        -0.65F, -0.55F, 1.0F, 0.20F, 0.25F,
        0.65F, -0.55F, 0.15F, 0.85F, 0.35F,
        0.0F, 0.65F, 0.25F, 0.45F, 1.0F,
    };

    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    constexpr GLsizei kStride = 5 * sizeof(float);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, kStride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, kStride, reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    bool running = true;
    while (running) {
      SDL_Event event{};
      while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
          running = false;
        } else if (event.type == SDL_KEYDOWN &&
                   event.key.keysym.sym == SDLK_ESCAPE) {
          running = false;
        } else if (event.type == SDL_WINDOWEVENT &&
                   event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          glViewport(0, 0, event.window.data1, event.window.data2);
        }
      }

      glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
      glClear(GL_COLOR_BUFFER_BIT);

      glUseProgram(shaderProgram);
      glBindVertexArray(vertexArray);
      glDrawArrays(GL_TRIANGLES, 0, 3);

      SDL_GL_SwapWindow(window);
    }
  } catch (const std::exception& exception) {
    std::cerr << exception.what() << '\n';
  }

  if (vertexBuffer != 0) {
    glDeleteBuffers(1, &vertexBuffer);
  }
  if (vertexArray != 0) {
    glDeleteVertexArrays(1, &vertexArray);
  }
  if (shaderProgram != 0) {
    glDeleteProgram(shaderProgram);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
