#include <SDL2/SDL.h>
#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.hpp"

import arguments;
import main_window;
import scene;

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
  struct ShapeGlBufferIds
  {
    // Идентификатор Vertex Array Object, который хранит раскладку атрибутов формы.
    GLuint vertexArray = 0;

    // Идентификатор Vertex Buffer Object с вершинами формы.
    GLuint vertexBuffer = 0;

    // Идентификатор Element Buffer Object с индексами треугольников формы.
    GLuint indexBuffer = 0;
  };

  glm::vec3 normalize(const glm::vec3 &value, const std::string_view name)
  {
    if (const float length = glm::length(value); length <= 0.0F)
    {
      throw std::runtime_error("gt6x0oIKhs :: Cannot normalize zero vector '" + std::string(name) + "'");
    }
    return glm::normalize(value);
  }

  glm::mat4 projectionMatrix(const float fovDegrees, const float aspect, const float nearPlane, const float farPlane)
  {
    if (aspect <= 0.0F || nearPlane <= 0.0F || farPlane <= nearPlane || fovDegrees <= 0.0F || fovDegrees >= 180.0F)
    {
      throw std::runtime_error("bE0zx1qKe1 :: Invalid projection camera values");
    }

    return glm::perspective(glm::radians(fovDegrees), aspect, nearPlane, farPlane);
  }

  glm::mat4 viewMatrix(const glm::vec3 &position, const glm::vec3 &cameraForward, const glm::vec3 &cameraUp)
  {
    const glm::vec3 forward = normalize(cameraForward, "camera.forward");
    const glm::vec3 left    = normalize(glm::cross(cameraUp, forward), "camera.left");
    const glm::vec3 up      = normalize(glm::cross(forward, left), "camera.up2");

    return glm::lookAt(position, position + forward, up);
  }

  void rotateForward(glm::vec3 &forward, const glm::vec3 &cameraUp, const int mouseDeltaX, const int mouseDeltaY, const float sensitivity)
  {
    if (mouseDeltaX == 0 && mouseDeltaY == 0)
    {
      return;
    }

    const glm::vec3 up      = normalize(cameraUp, "camera.up");
    const float yaw         = glm::radians(-static_cast<float>(mouseDeltaX) * sensitivity);
    const float pitch       = glm::radians(-static_cast<float>(mouseDeltaY) * sensitivity);
    const glm::vec3 vector1 = glm::rotate(glm::mat4{1.0F}, yaw, up) * glm::vec4(forward, 0.0F);
    forward                 = normalize(vector1, "camera.forward");

    const glm::vec3 right          = normalize(glm::cross(forward, up), "camera.right");
    const glm::vec3 vector         = glm::rotate(glm::mat4{1.0F}, pitch, right) * glm::vec4(forward, 0.0F);
    const glm::vec3 pitchedForward = normalize(vector, "camera.forward");

    if (glm::length(glm::cross(pitchedForward, up)) > 0.001F)
    {
      forward = pitchedForward;
    }
  }

  GLuint compileShader(const GLenum type, const std::string_view source)
  {
    // Создаем объект шейдера указанного типа.
    const GLuint shader     = glCreateShader(type);
    const char *sourceData  = source.data();
    const auto sourceLength = static_cast<GLint>(source.size());
    // Передаем исходный код шейдера в OpenGL.
    glShaderSource(shader, 1, &sourceData, &sourceLength);
    // Компилируем загруженный исходный код шейдера.
    glCompileShader(shader);

    GLint success = GL_FALSE;
    // Проверяем, успешно ли завершилась компиляция шейдера.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE)
    {
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

  GLuint createShaderProgram()
  {
    const GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, resources::triangle_vert);
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
    if (success != GL_TRUE)
    {
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

int main(int argvCount, char **argv)
{
  Arguments args(argvCount, argv);

  const std::filesystem::path &executableDirectory = args.executableDirectory();

  const std::filesystem::path intoPath = args.executableDirectory() / "intro.txt";
  if (std::ifstream introFile(intoPath); introFile)
  {
    std::cout << introFile.rdbuf() << '\n';
  }
  else
  {
    std::cerr << "VpO9pfn9wt :: Failed to open " << intoPath << '\n';
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
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
  if (context == nullptr)
  {
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
  std::vector<ShapeGlBufferIds> shapeBuffers;
  GLuint shapeInstanceGroup   = 0;
  bool mouseCaptured          = false;
  const auto setMouseCaptured = [&mouseCaptured](const bool captured)
  {
    if (mouseCaptured == captured)
    {
      return;
    }
    if (SDL_SetRelativeMouseMode(captured ? SDL_TRUE : SDL_FALSE) != 0)
    {
      throw std::runtime_error(std::string("Rk4dDWAkY5 :: SDL_SetRelativeMouseMode failed: ") + SDL_GetError());
    }
    mouseCaptured = captured;
  };

  try
  {
    shaderProgram                        = createShaderProgram();
    // Находим uniform-переменную матрицы проекции в шейдерной программе.
    const GLint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    // Находим uniform-переменную матрицы вида в шейдерной программе.
    const GLint viewMatrixLocation       = glGetUniformLocation(shaderProgram, "viewMatrix");
    // Находим uniform-переменную матрицы модели в шейдерной программе.
    const GLint modelMatrixLocation      = glGetUniformLocation(shaderProgram, "modelMatrix");
    if (projectionMatrixLocation < 0 || viewMatrixLocation < 0 || modelMatrixLocation < 0)
    {
      throw std::runtime_error("zJ9NCwdGPQ :: Failed to locate matrix uniforms");
    }

    scene::Scene scene;
    scene.load(executableDirectory / "scene.yaml");
    glm::vec3 cameraPosition = scene.camera.position;
    glm::vec3 cameraForward  = normalize(scene.camera.forward, "camera.forward");
    const glm::vec3 cameraUp = scene.camera.up;

    // Создаем буфер для данных инстансов.
    glGenBuffers(1, &shapeInstanceGroup);
    // Делаем буфер инстансов текущим.
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup);
    // Выделяем память GPU под данные инстансов, которые будут обновляться каждый кадр.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(scene.instances.size() * 4U * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

    shapeBuffers.resize(scene.shapes.size());
    const GLsizei stride                   = static_cast<GLsizei>(scene.vertexFloatCount * sizeof(float));
    const GLsizei shapeInstanceGroupStride = static_cast<GLsizei>(4U * sizeof(float));
    for (std::size_t shapeIndex = 0; shapeIndex < scene.shapes.size(); ++shapeIndex)
    {
      const scene::Shape &shape = scene.shapes[shapeIndex];
      ShapeGlBufferIds &buffers = shapeBuffers[shapeIndex];

      // Создаем объект Vertex Array Object для описания раскладки вершин.
      glGenVertexArrays(1, &buffers.vertexArray);
      // Создаем буфер для вершинных данных.
      glGenBuffers(1, &buffers.vertexBuffer);
      // Создаем буфер для индексов.
      glGenBuffers(1, &buffers.indexBuffer);

      // Делаем VAO текущим, чтобы следующие настройки атрибутов записались в него.
      glBindVertexArray(buffers.vertexArray);
      // Делаем вершинный буфер текущим.
      glBindBuffer(GL_ARRAY_BUFFER, buffers.vertexBuffer);
      // Загружаем массив вершин формы в память GPU.
      glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(shape.vertices.size() * sizeof(float)), shape.vertices.data(), GL_STATIC_DRAW);
      // Делаем индексный буфер текущим для выбранного VAO.
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.indexBuffer);
      // Загружаем индексы треугольников формы в память GPU.
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(shape.indexes.size() * sizeof(GLuint)), shape.indexes.data(), GL_STATIC_DRAW);

      // Описываем атрибут позиции вершины.
      glVertexAttribPointer(0, scene.positionFloatCount, GL_FLOAT, GL_FALSE, stride, nullptr);
      // Включаем атрибут позиции вершины.
      glEnableVertexAttribArray(0);
      // Описываем атрибут цвета вершины.
      glVertexAttribPointer(1, scene.colorFloatCount, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(scene.positionFloatCount * sizeof(float)));
      // Включаем атрибут цвета вершины.
      glEnableVertexAttribArray(1);
      // Делаем буфер инстансов текущим для настройки атрибута смещения.
      glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup);
      // Описываем атрибут смещения и индекса формы для одного инстанса.
      glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, shapeInstanceGroupStride,
                            reinterpret_cast<void *>(shape.firstInstance * static_cast<std::size_t>(shapeInstanceGroupStride)));
      // Включаем атрибут инстанса.
      glEnableVertexAttribArray(2);
      // Указываем, что атрибут инстанса меняется один раз на инстанс, а не на вершину.
      glVertexAttribDivisor(2, 1);
    }
    // Включаем проверку глубины для 3D-сцены.
    glEnable(GL_DEPTH_TEST);

    bool running      = true;
    bool moveForward  = false;
    bool moveBackward = false;
    bool moveLeft     = false;
    bool moveRight    = false;
    std::vector<float> instanceData(scene.instances.size() * 4U);
    Uint64 previousCounter = SDL_GetPerformanceCounter();
    while (running)
    {
      SDL_Event event{};

      while (SDL_PollEvent(&event) != 0)
      {
        if (event.type == SDL_QUIT)
        {
          running = false;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
          running = false;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && event.key.repeat == 0)
        {
          setMouseCaptured(!mouseCaptured);
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && mouseCaptured)
        {
          setMouseCaptured(false);
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_w)
        {
          moveForward = true;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s)
        {
          moveBackward = true;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a)
        {
          moveLeft = true;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_d)
        {
          moveRight = true;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_w)
        {
          moveForward = false;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s)
        {
          moveBackward = false;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_a)
        {
          moveLeft = false;
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_d)
        {
          moveRight = false;
        }
        else if (event.type == SDL_MOUSEMOTION && mouseCaptured)
        {
          rotateForward(cameraForward, cameraUp, event.motion.xrel, event.motion.yrel, scene.camera.forwardMouseSensitivity);
        }
        else if (event.type == SDL_MOUSEWHEEL)
        {
          cameraPosition += cameraForward * scene.camera.forwardScrollStep * static_cast<float>(event.wheel.y);
        }
        else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
          // Подгоняем область вывода OpenGL под новый размер окна.
          glViewport(0, 0, event.window.data1, event.window.data2);
        }
        if (event.type == SDL_WINDOWEVENT)
        {
          window.syncWindowEvent(event.window);
        }
      }

      window.idle();

      const Uint64 currentCounter = SDL_GetPerformanceCounter();
      const float deltaSeconds    = static_cast<float>(currentCounter - previousCounter) / static_cast<float>(SDL_GetPerformanceFrequency());
      previousCounter             = currentCounter;
      const int movementDirection = (moveForward ? 1 : 0) - (moveBackward ? 1 : 0);
      cameraPosition += cameraForward * scene.camera.forwardVelocity * static_cast<float>(movementDirection) * deltaSeconds;
      const glm::vec3 cameraLeft      = normalize(glm::cross(cameraForward, cameraUp), "camera.left");
      const int sideMovementDirection = (moveRight ? 1 : 0) - (moveLeft ? 1 : 0);
      cameraPosition += cameraLeft * scene.camera.sideVelocity * static_cast<float>(sideMovementDirection) * deltaSeconds;

      // Задаем цвет очистки кадрового буфера.
      glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
      // Очищаем цветовой буфер перед отрисовкой нового кадра.
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Активируем шейдерную программу для текущей отрисовки.
      glUseProgram(shaderProgram);
      for (std::size_t instanceIndex = 0; instanceIndex < scene.instances.size(); ++instanceIndex)
      {
        const scene::ShapeInstance &instance = scene.instances[instanceIndex];
        const std::size_t writeIndex         = instanceIndex * 4U;
        instanceData[writeIndex]             = instance.offset[0];
        instanceData[writeIndex + 1U]        = instance.offset[1];
        instanceData[writeIndex + 2U]        = instance.offset[2];
        instanceData[writeIndex + 3U]        = static_cast<float>(instance.shapeIndex);
      }
      // Делаем буфер инстансов текущим перед обновлением его содержимого.
      glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup);
      // Загружаем актуальные смещения и индексы форм для текущего кадра.
      glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(instanceData.size() * sizeof(float)), instanceData.data());

      const int viewportWidth    = std::max(window.width(), 1);
      const int viewportHeight   = std::max(window.height(), 1);
      const float aspect         = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
      const glm::mat4 projection = projectionMatrix(scene.camera.fovDegrees, aspect, scene.camera.nearPlane, scene.camera.farPlane);
      const glm::mat4 view       = viewMatrix(cameraPosition, cameraForward, cameraUp);
      const glm::mat4 model{1.0F};
      // Передаем матрицу проекции в текущую шейдерную программу.
      glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
      // Передаем матрицу вида в текущую шейдерную программу.
      glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
      // Передаем матрицу модели в текущую шейдерную программу.
      glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(model));
      for (std::size_t shapeIndex = 0; shapeIndex < scene.shapes.size(); ++shapeIndex)
      {
        const scene::Shape &shape = scene.shapes[shapeIndex];
        if (shape.instanceCount == 0)
        {
          continue;
        }
        // Выбираем VAO формы с раскладкой вершин, индексным буфером и диапазоном инстансов.
        glBindVertexArray(shapeBuffers[shapeIndex].vertexArray);
        // Рисуем все инстансы формы по индексам из текущего индексного буфера.
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(shape.indexes.size()), GL_UNSIGNED_INT, nullptr,
                                static_cast<GLsizei>(shape.instanceCount));
      }

      // Показываем отрисованный кадр, меняя back/front буферы окна.
      SDL_GL_SwapWindow(window.nativeHandle());
    }
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << '\n';
  }

  if (mouseCaptured)
  {
    SDL_SetRelativeMouseMode(SDL_FALSE);
  }
  if (shapeInstanceGroup != 0)
  {
    // Освобождаем буфер инстансов в OpenGL.
    glDeleteBuffers(1, &shapeInstanceGroup);
  }
  for (const ShapeGlBufferIds &buffers : shapeBuffers)
  {
    if (buffers.vertexBuffer != 0)
    {
      // Освобождаем вершинный буфер в OpenGL.
      glDeleteBuffers(1, &buffers.vertexBuffer);
    }
    if (buffers.indexBuffer != 0)
    {
      // Освобождаем индексный буфер в OpenGL.
      glDeleteBuffers(1, &buffers.indexBuffer);
    }
    if (buffers.vertexArray != 0)
    {
      // Освобождаем VAO в OpenGL.
      glDeleteVertexArrays(1, &buffers.vertexArray);
    }
  }
  if (shaderProgram != 0)
  {
    // Освобождаем шейдерную программу в OpenGL.
    glDeleteProgram(shaderProgram);
  }

  // Удаляем OpenGL-контекст SDL.
  SDL_GL_DeleteContext(context);
  window.close();
  SDL_Quit();
  return 0;
}
