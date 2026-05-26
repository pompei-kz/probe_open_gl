#include <epoxy/gl.h>
#include <SDL2/SDL.h>

#include "resources.hpp"

import tri_data;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
  constexpr int kWindowWidth = 800;
  constexpr int kWindowHeight = 600;

  GLuint compileShader(GLenum type, std::string_view source) {
    const GLuint shader = glCreateShader(type);
    const char *sourceData = source.data();
    const auto sourceLength = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &sourceData, &sourceLength);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      GLint logLength = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      glGetShaderInfoLog(shader, logLength, nullptr, log.data());
      glDeleteShader(shader);
      throw std::runtime_error("qWcxNTYsV8 :: Shader compilation failed: " + log);
    }

    return shader;
  }

  GLuint createShaderProgram() {
    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, resources::triangle_vert);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, resources::triangle_frag);

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
      throw std::runtime_error("9UI2AR1q8U :: Shader link failed: " + log);
    }

    return program;
  }
} // namespace

int main(int, char **argv) {
  std::filesystem::path executableDirectory = std::filesystem::path(argv[0]).parent_path();
  if (executableDirectory.empty()) {
    executableDirectory = std::filesystem::current_path();
  }

  const std::filesystem::path intoPath = executableDirectory / "intro.txt";
  if (std::ifstream introFile(intoPath); introFile) {
    std::cout << introFile.rdbuf() << '\n';
  } else {
    std::cerr << "VpO9pfn9wt :: Failed to open " << intoPath << '\n';
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "mVCKXbMftG :: SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_Window *window = SDL_CreateWindow(
    "OpenGL 3.3 Core Triangle",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    kWindowWidth,
    kWindowHeight,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  if (window == nullptr) {
    std::cerr << "jcxLsEPi1X :: SDL_CreateWindow failed: " << SDL_GetError() << '\n';
    SDL_Quit();
    return 1;
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (context == nullptr) {
    std::cerr << "Ivn1fta1oB :: SDL_GL_CreateContext failed: " << SDL_GetError() << '\n';
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_GL_SetSwapInterval(1);
  glViewport(0, 0, kWindowWidth, kWindowHeight);

  std::cout << "MXL4NrIm8M :: OpenGL: " << glGetString(GL_VERSION) << '\n';
  std::cout << "DZ2EDsUp4f :: GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

  GLuint shaderProgram = 0;
  GLuint vertexArray = 0;
  GLuint vertexBuffer = 0;
  GLuint indexBuffer = 0;

  try {
    shaderProgram = createShaderProgram();

    const tri_data::TriData triData = tri_data::loadTriData(executableDirectory / "tri-data.yaml");

    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(triData.vertices.size() * sizeof(float)),
                 triData.vertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(triData.indexes.size() * sizeof(GLuint)),
                 triData.indexes.data(),
                 GL_STATIC_DRAW);

    const auto stride = static_cast<GLsizei>(triData.vertexFloatCount * sizeof(float));
    glVertexAttribPointer(0, triData.positionFloatCount, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,
                          triData.colorFloatCount,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          reinterpret_cast<void *>(triData.positionFloatCount * sizeof(float)));
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
      glDrawElements(GL_TRIANGLES,
                     static_cast<GLsizei>(triData.indexes.size()),
                     GL_UNSIGNED_INT,
                     nullptr);

      SDL_GL_SwapWindow(window);
    }
  } catch (const std::exception &exception) {
    std::cerr << exception.what() << '\n';
  }

  if (vertexBuffer != 0) {
    glDeleteBuffers(1, &vertexBuffer);
  }
  if (indexBuffer != 0) {
    glDeleteBuffers(1, &indexBuffer);
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
