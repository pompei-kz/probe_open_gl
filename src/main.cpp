#include <epoxy/gl.h>
#include <SDL2/SDL.h>

#include "resources.hpp"

import arguments;
import main_window;
import tri_data;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
  GLuint compileShader(GLenum type, std::string_view source) {
    // Создаем объект шейдера указанного типа.
    const GLuint shader = glCreateShader(type);
    const char *sourceData = source.data();
    const auto sourceLength = static_cast<GLint>(source.size());
    // Передаем исходный код шейдера в OpenGL.
    glShaderSource(shader, 1, &sourceData, &sourceLength);
    // Компилируем загруженный исходный код шейдера.
    glCompileShader(shader);

    GLint success = GL_FALSE;
    // Проверяем, успешно ли завершилась компиляция шейдера.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
      GLint logLength = 0;
      // Узнаем размер диагностического лога компиляции.
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      // Забираем текст ошибки компиляции из OpenGL.
      glGetShaderInfoLog(shader, logLength, nullptr, log.data());
      // Удаляем неудачно скомпилированный шейдер.
      glDeleteShader(shader);
      throw std::runtime_error("qWcxNTYsV8 :: Shader compilation failed: " + log);
    }

    return shader;
  }

  GLuint createShaderProgram() {
    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, resources::triangle_vert);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, resources::triangle_frag);

    // Создаем шейдерную программу.
    const GLuint program = glCreateProgram();
    // Подключаем вершинный шейдер к программе.
    glAttachShader(program, vertexShader);
    // Подключаем фрагментный шейдер к программе.
    glAttachShader(program, fragmentShader);
    // Линкуем программу из подключенных шейдеров.
    glLinkProgram(program);

    // После линковки отдельные объекты шейдеров больше не нужны.
    glDeleteShader(vertexShader);
    // После линковки отдельные объекты шейдеров больше не нужны.
    glDeleteShader(fragmentShader);

    GLint success = GL_FALSE;
    // Проверяем, успешно ли слинковалась шейдерная программа.
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
      GLint logLength = 0;
      // Узнаем размер диагностического лога линковки.
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      // Забираем текст ошибки линковки из OpenGL.
      glGetProgramInfoLog(program, logLength, nullptr, log.data());
      // Удаляем неудачно слинкованную программу.
      glDeleteProgram(program);
      throw std::runtime_error("9UI2AR1q8U :: Shader link failed: " + log);
    }

    return program;
  }
} // namespace

int main(int argvCount, char **argv) {
  Arguments args(argvCount, argv);

  const std::filesystem::path &executableDirectory = args.executableDirectory();

  const std::filesystem::path intoPath = args.executableDirectory() / "intro.txt";
  if (std::ifstream introFile(intoPath); introFile) {
    std::cout << introFile.rdbuf() << '\n';
  } else {
    std::cerr << "VpO9pfn9wt :: Failed to open " << intoPath << '\n';
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "mVCKXbMftG :: SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  // Выбираем мажорную версию OpenGL-контекста.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  // Выбираем минорную версию OpenGL-контекста.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  // Запрашиваем core profile без устаревших функций OpenGL.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // Включаем двойную буферизацию для плавного вывода кадра.
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  MainWindow window(args.cacheDirectory(), "OpenGL 3.3 Core Triangle");

  // Создаем OpenGL-контекст для окна SDL.
  SDL_GLContext context = SDL_GL_CreateContext(window.nativeHandle());
  if (context == nullptr) {
    std::cerr << "Ivn1fta1oB :: SDL_GL_CreateContext failed: " << SDL_GetError() << '\n';
    window.close();
    SDL_Quit();
    return 1;
  }

  // Синхронизируем обмен буферов с вертикальной разверткой.
  SDL_GL_SetSwapInterval(1);
  // Задаем начальную область вывода OpenGL в размере окна.
  glViewport(0, 0, window.width(), window.height());

  // Читаем строку версии OpenGL у текущего контекста.
  std::cout << "MXL4NrIm8M :: OpenGL: " << glGetString(GL_VERSION) << '\n';
  // Читаем строку версии GLSL у текущего контекста.
  std::cout << "DZ2EDsUp4f :: GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

  GLuint shaderProgram = 0;
  GLuint vertexArray = 0;
  GLuint vertexBuffer = 0;
  GLuint indexBuffer = 0;

  try {
    shaderProgram = createShaderProgram();

    const tri_data::TriData triData = tri_data::loadTriData(executableDirectory / "tri-data.yaml");

    // Создаем объект Vertex Array Object для описания раскладки вершин.
    glGenVertexArrays(1, &vertexArray);
    // Создаем буфер для вершинных данных.
    glGenBuffers(1, &vertexBuffer);
    // Создаем буфер для индексов.
    glGenBuffers(1, &indexBuffer);

    // Делаем VAO текущим, чтобы следующие настройки атрибутов записались в него.
    glBindVertexArray(vertexArray);
    // Делаем вершинный буфер текущим.
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    // Загружаем массив вершин в память GPU.
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(triData.vertices.size() * sizeof(float)),
                 triData.vertices.data(),
                 GL_STATIC_DRAW);
    // Делаем индексный буфер текущим для выбранного VAO.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    // Загружаем индексы треугольников в память GPU.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(triData.indexes.size() * sizeof(GLuint)),
                 triData.indexes.data(),
                 GL_STATIC_DRAW);

    const auto stride = static_cast<GLsizei>(triData.vertexFloatCount * sizeof(float));
    // Описываем атрибут позиции вершины.
    glVertexAttribPointer(0, triData.positionFloatCount, GL_FLOAT, GL_FALSE, stride, nullptr);
    // Включаем атрибут позиции вершины.
    glEnableVertexAttribArray(0);
    // Описываем атрибут цвета вершины.
    glVertexAttribPointer(1,
                          triData.colorFloatCount,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          reinterpret_cast<void *>(triData.positionFloatCount * sizeof(float)));
    // Включаем атрибут цвета вершины.
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
          // Подгоняем область вывода OpenGL под новый размер окна.
          glViewport(0, 0, event.window.data1, event.window.data2);
        }
        if (event.type == SDL_WINDOWEVENT) {
          window.syncWindowEvent(event.window);
        }
      }

      // Задаем цвет очистки кадрового буфера.
      glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
      // Очищаем цветовой буфер перед отрисовкой нового кадра.
      glClear(GL_COLOR_BUFFER_BIT);

      // Активируем шейдерную программу для текущей отрисовки.
      glUseProgram(shaderProgram);
      // Выбираем VAO с раскладкой вершин и индексным буфером.
      glBindVertexArray(vertexArray);
      // Рисуем треугольники по индексам из текущего индексного буфера.
      glDrawElements(GL_TRIANGLES,
                     static_cast<GLsizei>(triData.indexes.size()),
                     GL_UNSIGNED_INT,
                     nullptr);

      // Показываем отрисованный кадр, меняя back/front буферы окна.
      SDL_GL_SwapWindow(window.nativeHandle());
    }
  } catch (const std::exception &exception) {
    std::cerr << exception.what() << '\n';
  }

  if (vertexBuffer != 0) {
    // Освобождаем вершинный буфер в OpenGL.
    glDeleteBuffers(1, &vertexBuffer);
  }
  if (indexBuffer != 0) {
    // Освобождаем индексный буфер в OpenGL.
    glDeleteBuffers(1, &indexBuffer);
  }
  if (vertexArray != 0) {
    // Освобождаем VAO в OpenGL.
    glDeleteVertexArrays(1, &vertexArray);
  }
  if (shaderProgram != 0) {
    // Освобождаем шейдерную программу в OpenGL.
    glDeleteProgram(shaderProgram);
  }

  // Удаляем OpenGL-контекст SDL.
  SDL_GL_DeleteContext(context);
  window.close();
  SDL_Quit();
  return 0;
}
