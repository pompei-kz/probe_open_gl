module;

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.hpp"

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

export module render;

import scene;

export enum class MoveVert {
  NONE,
  UP,
  DOWN,
};

export enum class MoveHoriz {
  NONE,
  LEFT,
  RIGHT,
};

namespace
{
  struct ShapeGlBufferIds
  {
    GLuint vertexArray  = 0;
    GLuint vertexBuffer = 0;
    GLuint indexBuffer  = 0;
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
    const float     yaw     = glm::radians(-static_cast<float>(mouseDeltaX) * sensitivity);
    const float     pitch   = glm::radians(-static_cast<float>(mouseDeltaY) * sensitivity);
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
    const GLuint shader       = glCreateShader(type);
    const char  *sourceData   = source.data();
    const auto   sourceLength = static_cast<GLint>(source.size());
    // Передаем исходный код шейдера в OpenGL.
    glShaderSource(shader, 1, &sourceData, &sourceLength);
    // Компилируем исходный код шейдера.
    glCompileShader(shader);

    GLint success = GL_FALSE;
    // Проверяем статус компиляции шейдера.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE)
    {
      GLint logLength = 0;
      // Узнаем размер диагностического лога компиляции.
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      // Забираем текст диагностического лога компиляции.
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

    // Удаляем объект вершинного шейдера после линковки.
    glDeleteShader(vertexShader);
    // Удаляем объект фрагментного шейдера после линковки.
    glDeleteShader(fragmentShader);

    GLint success = GL_FALSE;
    // Проверяем статус линковки шейдерной программы.
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE)
    {
      GLint logLength = 0;
      // Узнаем размер диагностического лога линковки.
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      // Забираем текст диагностического лога линковки.
      glGetProgramInfoLog(program, logLength, nullptr, log.data());
      // Удаляем неудачно слинкованную программу.
      glDeleteProgram(program);
      throw std::runtime_error("9UI2AR1q8U :: Shader link failed: " + log);
    }

    return program;
  }
} // namespace

export class Render
{
public:
  explicit Render(const std::filesystem::path &scenePath)
  {
    shaderProgram_            = createShaderProgram();
    // Находим uniform-переменную матрицы проекции.
    projectionMatrixLocation_ = glGetUniformLocation(shaderProgram_, "projectionMatrix");
    // Находим uniform-переменную матрицы вида.
    viewMatrixLocation_       = glGetUniformLocation(shaderProgram_, "viewMatrix");
    // Находим uniform-переменную матрицы модели.
    modelMatrixLocation_      = glGetUniformLocation(shaderProgram_, "modelMatrix");
    // Находим uniform-переменную силы солнечного света.
    sunForceLocation_         = glGetUniformLocation(shaderProgram_, "sunForce");
    // Находим uniform-переменную направления солнечного света.
    sunDirectionLocation_     = glGetUniformLocation(shaderProgram_, "sunDirection");
    // Находим uniform-переменную цвета солнечного света.
    sunColorLocation_         = glGetUniformLocation(shaderProgram_, "sunColor");
    if (projectionMatrixLocation_ < 0 || viewMatrixLocation_ < 0 || modelMatrixLocation_ < 0 || sunForceLocation_ < 0 || sunDirectionLocation_ < 0 ||
        sunColorLocation_ < 0)
    {
      throw std::runtime_error("zJ9NCwdGPQ :: Failed to locate shader uniforms");
    }

    scene_.load(scenePath);
    cameraPosition_ = scene_.camera.position;
    cameraForward_  = normalize(scene_.camera.forward, "camera.forward");
    cameraUp_       = scene_.camera.up;

    // Создаем буфер для данных инстансов.
    glGenBuffers(1, &shapeInstanceGroup_);
    // Делаем буфер инстансов текущим.
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
    // Выделяем память GPU под данные инстансов.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(scene_.instances.size() * 4U * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

    shapeBuffers_.resize(scene_.shapes.size());
    const GLsizei stride                   = static_cast<GLsizei>(scene_.vertexFloatCount * sizeof(float));
    const GLsizei shapeInstanceGroupStride = 4U * sizeof(float);
    for (std::size_t shapeIndex = 0; shapeIndex < scene_.shapes.size(); ++shapeIndex)
    {
      const scene::Shape &shape   = scene_.shapes[shapeIndex];
      ShapeGlBufferIds   &buffers = shapeBuffers_[shapeIndex];

      // Создаем VAO для раскладки атрибутов формы.
      glGenVertexArrays(1, &buffers.vertexArray);
      // Создаем вершинный буфер формы.
      glGenBuffers(1, &buffers.vertexBuffer);
      // Создаем индексный буфер формы.
      glGenBuffers(1, &buffers.indexBuffer);

      // Делаем VAO формы текущим.
      glBindVertexArray(buffers.vertexArray);
      // Делаем вершинный буфер формы текущим.
      glBindBuffer(GL_ARRAY_BUFFER, buffers.vertexBuffer);
      // Загружаем вершины формы в GPU.
      glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(shape.vertices.size() * sizeof(float)), shape.vertices.data(), GL_STATIC_DRAW);
      // Делаем индексный буфер формы текущим.
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.indexBuffer);
      // Загружаем индексы формы в GPU.
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(shape.indexes.size() * sizeof(GLuint)), shape.indexes.data(), GL_STATIC_DRAW);

      // Описываем атрибут позиции вершины.
      glVertexAttribPointer(0, scene_.positionFloatCount, GL_FLOAT, GL_FALSE, stride, nullptr);
      // Включаем атрибут позиции вершины.
      glEnableVertexAttribArray(0);
      // Описываем атрибут цвета вершины.
      glVertexAttribPointer(1, scene_.colorFloatCount, GL_FLOAT, GL_FALSE, stride,
                            reinterpret_cast<void *>(scene_.positionFloatCount * sizeof(float)));
      // Включаем атрибут цвета вершины.
      glEnableVertexAttribArray(1);
      // Делаем буфер инстансов текущим.
      glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
      // Описываем атрибут данных инстанса.
      glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, shapeInstanceGroupStride,
                            reinterpret_cast<void *>(shape.firstInstance * static_cast<std::size_t>(shapeInstanceGroupStride)));
      // Включаем атрибут данных инстанса.
      glEnableVertexAttribArray(2);
      // Указываем, что атрибут меняется один раз на инстанс.
      glVertexAttribDivisor(2, 1);
    }

    // Включаем проверку глубины для 3D-отрисовки.
    glEnable(GL_DEPTH_TEST);
    instanceData_.resize(scene_.instances.size() * 4U);
  }

  Render(const Render &) = delete;

  Render &operator=(const Render &) = delete;

  Render(Render &&) = delete;

  Render &operator=(Render &&) = delete;

  ~Render() { release(); }

private:
  void release()
  {
    if (shapeInstanceGroup_ != 0)
    {
      // Освобождаем буфер инстансов.
      glDeleteBuffers(1, &shapeInstanceGroup_);
      shapeInstanceGroup_ = 0;
    }
    for (ShapeGlBufferIds &buffers : shapeBuffers_)
    {
      if (buffers.vertexBuffer != 0)
      {
        // Освобождаем вершинный буфер формы.
        glDeleteBuffers(1, &buffers.vertexBuffer);
        buffers.vertexBuffer = 0;
      }
      if (buffers.indexBuffer != 0)
      {
        // Освобождаем индексный буфер формы.
        glDeleteBuffers(1, &buffers.indexBuffer);
        buffers.indexBuffer = 0;
      }
      if (buffers.vertexArray != 0)
      {
        // Освобождаем VAO формы.
        glDeleteVertexArrays(1, &buffers.vertexArray);
        buffers.vertexArray = 0;
      }
    }
    if (shaderProgram_ != 0)
    {
      // Освобождаем шейдерную программу.
      glDeleteProgram(shaderProgram_);
      shaderProgram_ = 0;
    }
  }

public:
  void setMoveVert(const MoveVert moveVert) { moveVert_ = moveVert; }

  void setMoveHoriz(const MoveHoriz moveHoriz) { moveHoriz_ = moveHoriz; }

  void rotateCamera(const int mouseDeltaX, const int mouseDeltaY)
  {
    rotateForward(cameraForward_, cameraUp_, mouseDeltaX, mouseDeltaY, scene_.camera.forwardMouseSensitivity);
  }

  void scrollCamera(const int wheelY) { cameraPosition_ += cameraForward_ * scene_.camera.forwardScrollStep * static_cast<float>(wheelY); }

  void drawFrame(const int viewportWidth, const int viewportHeight, const float deltaSeconds)
  {
    const glm::vec3 cameraLeft            = normalize(glm::cross(cameraForward_, cameraUp_), "camera.left");
    const int       sideMovementDirection = (moveHoriz_ == MoveHoriz::RIGHT ? 1 : 0) - (moveHoriz_ == MoveHoriz::LEFT ? 1 : 0);
    cameraPosition_ += cameraLeft * scene_.camera.sideVelocity * static_cast<float>(sideMovementDirection) * deltaSeconds;
    const int verticalMovementDirection = (moveVert_ == MoveVert::UP ? 1 : 0) - (moveVert_ == MoveVert::DOWN ? 1 : 0);
    cameraPosition_ += cameraUp_ * scene_.camera.sideVelocity * static_cast<float>(verticalMovementDirection) * deltaSeconds;

    // Задаем цвет очистки кадрового буфера.
    glClearColor(scene_.params.backgroundColor[0], scene_.params.backgroundColor[1], scene_.params.backgroundColor[2], 1.0F);
    // Очищаем цветовой буфер и буфер глубины.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Активируем шейдерную программу для текущего кадра.
    glUseProgram(shaderProgram_);
    for (std::size_t instanceIndex = 0; instanceIndex < scene_.instances.size(); ++instanceIndex)
    {
      const scene::ShapeInstance &instance   = scene_.instances[instanceIndex];
      const std::size_t           writeIndex = instanceIndex * 4U;
      instanceData_[writeIndex]              = instance.offset[0];
      instanceData_[writeIndex + 1U]         = instance.offset[1];
      instanceData_[writeIndex + 2U]         = instance.offset[2];
      instanceData_[writeIndex + 3U]         = static_cast<float>(instance.shapeIndex);
    }
    // Делаем буфер инстансов текущим перед обновлением.
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
    // Загружаем актуальные данные инстансов в GPU.
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(instanceData_.size() * sizeof(float)), instanceData_.data());

    const int       width      = std::max(viewportWidth, 1);
    const int       height     = std::max(viewportHeight, 1);
    const float     aspect     = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 projection = projectionMatrix(scene_.camera.fovDegrees, aspect, scene_.camera.nearPlane, scene_.camera.farPlane);
    const glm::mat4 view       = viewMatrix(cameraPosition_, cameraForward_, cameraUp_);
    const glm::mat4 model{1.0F};
    // Передаем матрицу проекции в шейдер.
    glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE, glm::value_ptr(projection));
    // Передаем матрицу вида в шейдер.
    glUniformMatrix4fv(viewMatrixLocation_, 1, GL_FALSE, glm::value_ptr(view));
    // Передаем матрицу модели в шейдер.
    glUniformMatrix4fv(modelMatrixLocation_, 1, GL_FALSE, glm::value_ptr(model));
    // Передаем силу солнечного света в шейдер.
    glUniform1f(sunForceLocation_, scene_.sun.force);
    // Передаем направление солнечного света в шейдер.
    glUniform3fv(sunDirectionLocation_, 1, glm::value_ptr(scene_.sun.direction));
    // Передаем цвет солнечного света в шейдер.
    glUniform3fv(sunColorLocation_, 1, glm::value_ptr(scene_.sun.color));

    for (std::size_t shapeIndex = 0; shapeIndex < scene_.shapes.size(); ++shapeIndex)
    {
      const scene::Shape &shape = scene_.shapes[shapeIndex];
      if (shape.instanceCount == 0)
      {
        continue;
      }
      // Выбираем VAO формы перед отрисовкой.
      glBindVertexArray(shapeBuffers_[shapeIndex].vertexArray);
      // Рисуем все инстансы формы по индексам.
      glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(shape.indexes.size()), GL_UNSIGNED_INT, nullptr,
                              static_cast<GLsizei>(shape.instanceCount));
    }
  }

private:
  scene::Scene                  scene_;
  GLuint                        shaderProgram_            = 0;
  GLuint                        shapeInstanceGroup_       = 0;
  GLint                         projectionMatrixLocation_ = -1;
  GLint                         viewMatrixLocation_       = -1;
  GLint                         modelMatrixLocation_      = -1;
  GLint                         sunForceLocation_         = -1;
  GLint                         sunDirectionLocation_     = -1;
  GLint                         sunColorLocation_         = -1;
  std::vector<ShapeGlBufferIds> shapeBuffers_;
  std::vector<float>            instanceData_;
  glm::vec3                     cameraPosition_{};
  glm::vec3                     cameraForward_{};
  glm::vec3                     cameraUp_{};
  MoveVert                      moveVert_  = MoveVert::NONE;
  MoveHoriz                     moveHoriz_ = MoveHoriz::NONE;
};
