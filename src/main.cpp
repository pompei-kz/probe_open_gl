#include <epoxy/gl.h>
#include <SDL2/SDL.h>

#include "resources.hpp"

import arguments;
import main_window;
import tri_data;

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
  using Vec3 = std::array<float, 3>;
  using Mat4 = std::array<float, 16>;

  float dot(const Vec3 &left, const Vec3 &right) {
    return left[0] * right[0] + left[1] * right[1] + left[2] * right[2];
  }

  Vec3 cross(const Vec3 &left, const Vec3 &right) {
    return {
      left[1] * right[2] - left[2] * right[1],
      left[2] * right[0] - left[0] * right[2],
      left[0] * right[1] - left[1] * right[0],
    };
  }

  Vec3 scale(const Vec3 &value, const float factor) {
    return {value[0] * factor, value[1] * factor, value[2] * factor};
  }

  Vec3 normalize(const Vec3 &value, const std::string_view name) {
    const float length = std::sqrt(dot(value, value));
    if (length <= 0.0F) {
      throw std::runtime_error("gt6x0oIKhs :: Cannot normalize zero vector '" + std::string(name) + "'");
    }
    return scale(value, 1.0F / length);
  }

  Mat4 identityMatrix() {
    return {
      1.0F, 0.0F, 0.0F, 0.0F,
      0.0F, 1.0F, 0.0F, 0.0F,
      0.0F, 0.0F, 1.0F, 0.0F,
      0.0F, 0.0F, 0.0F, 1.0F,
    };
  }

  Mat4 projectionMatrix(const float fovDegrees,
                        const float aspect,
                        const float nearPlane,
                        const float farPlane) {
    if (aspect <= 0.0F || nearPlane <= 0.0F || farPlane <= nearPlane
        || fovDegrees <= 0.0F || fovDegrees >= 180.0F) {
      throw std::runtime_error("bE0zxdqKe1 :: Invalid projection camera values");
    }

    const float fovRadians = fovDegrees * std::numbers::pi_v<float> / 180.0F;
    const float topScale = 1.0F / std::tan(fovRadians / 2.0F);
    return {
      topScale / aspect, 0.0F, 0.0F, 0.0F,
      0.0F, topScale, 0.0F, 0.0F,
      0.0F, 0.0F, -(farPlane + nearPlane) / (farPlane - nearPlane), -1.0F,
      0.0F, 0.0F, -(2.0F * farPlane * nearPlane) / (farPlane - nearPlane), 0.0F,
    };
  }

  Mat4 viewMatrix(const Vec3 &position, const Vec3 &cameraForward, const Vec3 &cameraUp) {
    const Vec3 forward = normalize(cameraForward, "camera.forward");
    const Vec3 left = normalize(cross(cameraUp, forward), "camera.left");
    const Vec3 up = normalize(cross(forward, left), "camera.up2");
    const Vec3 right = scale(left, -1.0F);
    const Vec3 backward = scale(forward, -1.0F);

    return {
      right[0], up[0], backward[0], 0.0F,
      right[1], up[1], backward[1], 0.0F,
      right[2], up[2], backward[2], 0.0F,
      -dot(right, position), -dot(up, position), -dot(backward, position), 1.0F,
    };
  }

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
    // Находим uniform-переменную матрицы проекции в шейдерной программе.
    const GLint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    // Находим uniform-переменную матрицы вида в шейдерной программе.
    const GLint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    // Находим uniform-переменную матрицы модели в шейдерной программе.
    const GLint modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
    if (projectionMatrixLocation < 0 || viewMatrixLocation < 0 || modelMatrixLocation < 0) {
      throw std::runtime_error("zJ9NCwdGPQ :: Failed to locate matrix uniforms");
    }

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
    // Включаем проверку глубины для 3D-сцены.
    glEnable(GL_DEPTH_TEST);

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
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Активируем шейдерную программу для текущей отрисовки.
      glUseProgram(shaderProgram);
      const int viewportWidth = std::max(window.width(), 1);
      const int viewportHeight = std::max(window.height(), 1);
      const float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
      const Mat4 projection = projectionMatrix(triData.camera.fovDegrees,
                                               aspect,
                                               triData.camera.nearPlane,
                                               triData.camera.farPlane);
      const Mat4 view = viewMatrix(triData.camera.position, triData.camera.forward, triData.camera.up);
      const Mat4 model = identityMatrix();
      // Передаем матрицу проекции в текущую шейдерную программу.
      glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projection.data());
      // Передаем матрицу вида в текущую шейдерную программу.
      glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, view.data());
      // Передаем матрицу модели в текущую шейдерную программу.
      glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, model.data());
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
