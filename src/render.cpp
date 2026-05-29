module;

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

module render;

import scene;
import text_writer;
import utils;

namespace
{
  struct ShapeGlBufferIds
  {
    GLuint vertexArrayID  = 0;
    GLuint vertexBufferID = 0;
    GLuint indexBufferID  = 0;
  };

  struct ShaderProgram
  {
    GLuint id                       = 0;
    GLint  projectionMatrixLocation = -1;
    GLint  viewMatrixLocation       = -1;
    GLint  modelMatrixLocation      = -1;
    GLint  sunForceLocation         = -1;
    GLint  sunDirectionLocation     = -1;
    GLint  sunColorLocation         = -1;
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

  void rotateCameraAxes(glm::vec3 &forward, glm::vec3 &cameraUp, const int mouseDeltaX, const int mouseDeltaY, const float sensitivity)
  {
    if (mouseDeltaX == 0 && mouseDeltaY == 0)
    {
      return;
    }

    glm::vec3       up        = normalize(cameraUp, "camera.up");
    const float     yaw       = glm::radians(-static_cast<float>(mouseDeltaX) * sensitivity);
    const float     pitch     = glm::radians(-static_cast<float>(mouseDeltaY) * sensitivity);
    const glm::mat4 yawMatrix = glm::rotate(glm::mat4{1.0F}, yaw, up);
    forward                   = normalize(yawMatrix * glm::vec4(forward, 0.0F), "camera.forward");

    const glm::vec3 right          = normalize(glm::cross(forward, up), "camera.right");
    const glm::mat4 pitchMatrix    = glm::rotate(glm::mat4{1.0F}, pitch, right);
    const glm::vec3 pitchedForward = normalize(pitchMatrix * glm::vec4(forward, 0.0F), "camera.forward");
    const glm::vec3 pitchedUp      = normalize(pitchMatrix * glm::vec4(up, 0.0F), "camera.up");

    if (glm::length(glm::cross(pitchedForward, pitchedUp)) > 0.001F)
    {
      forward  = pitchedForward;
      cameraUp = pitchedUp;
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

  std::string_view shaderSource(const std::string_view shaderName, const GLenum type)
  {
    if (shaderName == "triangle")
    {
      if (type == GL_VERTEX_SHADER)
      {
        return resources::triangle_vert;
      }
      if (type == GL_FRAGMENT_SHADER)
      {
        return resources::triangle_frag;
      }
    }

    throw std::runtime_error("v8gvUqpTFA :: Unknown shader program name: " + std::string(shaderName));
  }

  GLuint createShaderProgram(const std::string_view shaderName)
  {
    const GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, shaderSource(shaderName, GL_VERTEX_SHADER));
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, shaderSource(shaderName, GL_FRAGMENT_SHADER));

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
    // Удаляем объект фрагментарного шейдера после линковки.
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

  ShaderProgram createShapeShaderProgram(const std::string_view shaderName)
  {
    ShaderProgram result;
    result.id = createShaderProgram(shaderName);

    // Находим uniform-переменную матрицы проекции.
    result.projectionMatrixLocation = glGetUniformLocation(result.id, "projectionMatrix");
    // Находим uniform-переменную матрицы вида.
    result.viewMatrixLocation       = glGetUniformLocation(result.id, "viewMatrix");
    // Находим uniform-переменную матрицы модели.
    result.modelMatrixLocation      = glGetUniformLocation(result.id, "modelMatrix");
    // Находим uniform-переменную силы солнечного света.
    result.sunForceLocation         = glGetUniformLocation(result.id, "sunForce");
    // Находим uniform-переменную направления солнечного света.
    result.sunDirectionLocation     = glGetUniformLocation(result.id, "sunDirection");
    // Находим uniform-переменную цвета солнечного света.
    result.sunColorLocation         = glGetUniformLocation(result.id, "sunColor");

    checkPositive(result.projectionMatrixLocation, "zJ9NCwdGPQ :: Failed to locate shader uniform: projectionMatrix");
    checkPositive(result.viewMatrixLocation, "V1E8JgGFEo :: Failed to locate shader uniform: viewMatrix");
    checkPositive(result.modelMatrixLocation, "TWodBEarQO :: Failed to locate shader uniform: modelMatrix");
    checkPositive(result.sunForceLocation, "bwbBRPuG4e :: Failed to locate shader uniform: sunForce");
    checkPositive(result.sunDirectionLocation, "snQlp7ciWr :: Failed to locate shader uniform: sunDirection");
    checkPositive(result.sunColorLocation, "OiRdj622GQ :: Failed to locate shader uniform: sunColor");

    return result;
  }
} // namespace

struct Render::Impl
{
  explicit Impl(const std::filesystem::path &scenePath)
  {
    scene_.load(scenePath);

    for (const scene::ShapeGroup &shapeGroup : scene_.shapeGroups)
    {
      if (!shaderPrograms_.contains(shapeGroup.shaderName))
      {
        shaderPrograms_.emplace(shapeGroup.shaderName, createShapeShaderProgram(shapeGroup.shaderName));
      }
    }

    cameraPosition_ = scene_.camera.position;
    cameraForward_  = normalize(scene_.camera.forward, "camera.forward");
    cameraUp_       = scene_.camera.up;

    // Создаем буфер для данных инстансов.
    glGenBuffers(1, &shapeInstanceGroup_);
    // Делаем буфер инстансов текущим.
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
    // Выделяем память GPU под данные инстансов.
    glBufferData(GL_ARRAY_BUFFER, scene_.instancesSizeBytes(), nullptr, GL_DYNAMIC_DRAW);

    shapeBufferIds_.resize(scene_.shapes.size());

    for (std::size_t shapeIndex = 0; shapeIndex < scene_.shapes.size(); ++shapeIndex)
    {
      const scene::Shape &shape          = scene_.shapes[shapeIndex];
      ShapeGlBufferIds   &shapeBufferIds = shapeBufferIds_[shapeIndex];

      // Создаем VAO для раскладки атрибутов фигуры.
      glGenVertexArrays(1, &shapeBufferIds.vertexArrayID);
      // Создаем вершинный буфер фигуры.
      glGenBuffers(1, &shapeBufferIds.vertexBufferID);
      // Создаем индексный буфер фигуры.
      glGenBuffers(1, &shapeBufferIds.indexBufferID);

      // Делаем VAO фигуры текущим.
      glBindVertexArray(shapeBufferIds.vertexArrayID);

      // Делаем вершинный буфер фигуры текущим.
      glBindBuffer(GL_ARRAY_BUFFER, shapeBufferIds.vertexBufferID);

      // Загружаем вершины фигуры в GPU.
      glBufferData(GL_ARRAY_BUFFER, shape.verticesSizeBytes(), shape.vertices.data(), GL_STATIC_DRAW);

      // Делаем индексный буфер фигуры текущим.
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapeBufferIds.indexBufferID);

      // Загружаем индексы фигуры в GPU.
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.indexesSizeBytes(), shape.indexes.data(), GL_STATIC_DRAW);

      //
      // Attribute index: 0
      //

      // Описываем атрибут позиции вершины.
      glVertexAttribPointer(0,
                            scene::Shape::VertexPosFloatCount,
                            GL_FLOAT,
                            GL_FALSE,
                            scene::Shape::VertexStride,
                            reinterpret_cast<void *>(scene::Shape::VertexPosOffset));

      // Включаем атрибут позиции вершины.
      glEnableVertexAttribArray(0);

      //
      // Attribute index: 1
      //

      // Описываем атрибут цвета вершины.
      glVertexAttribPointer(1,
                            scene::Shape::VertexColorFloatCount,
                            GL_FLOAT,
                            GL_FALSE,
                            scene::Shape::VertexStride,
                            reinterpret_cast<void *>(scene::Shape::VertexColorOffset));

      // Включаем атрибут цвета вершины.
      glEnableVertexAttribArray(1);

      //
      // Attribute index: 2
      //

      // Делаем буфер инстансов текущим.
      glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
      // Описываем атрибут данных инстанса.
      glVertexAttribPointer(2, // index of attribute
                            scene::ShapeInstance::ComponentCount,
                            GL_FLOAT,
                            GL_FALSE,
                            scene::ShapeInstance::Stride,
                            nullptr);
      // Включаем атрибут данных инстанса.
      glEnableVertexAttribArray(2);
      // Указываем, что атрибут меняется один раз на инстанс.
      glVertexAttribDivisor(2, 1);
    }

    // Включаем проверку глубины для 3D-отрисовки.
    glEnable(GL_DEPTH_TEST);
    instanceData_.resize(scene_.instances.size() * scene::ShapeInstance::ComponentCount);
  }

  ~Impl() { release(); }

private:
  void release()
  {
    if (shapeInstanceGroup_ != 0)
    {
      // Освобождаем буфер инстансов.
      glDeleteBuffers(1, &shapeInstanceGroup_);
      shapeInstanceGroup_ = 0;
    }
    for (ShapeGlBufferIds &buffers : shapeBufferIds_)
    {
      if (buffers.vertexBufferID != 0)
      {
        // Освобождаем вершинный буфер фигуры.
        glDeleteBuffers(1, &buffers.vertexBufferID);
        buffers.vertexBufferID = 0;
      }
      if (buffers.indexBufferID != 0)
      {
        // Освобождаем индексный буфер фигуры.
        glDeleteBuffers(1, &buffers.indexBufferID);
        buffers.indexBufferID = 0;
      }
      if (buffers.vertexArrayID != 0)
      {
        // Освобождаем VAO фигуры.
        glDeleteVertexArrays(1, &buffers.vertexArrayID);
        buffers.vertexArrayID = 0;
      }
    }
    for (auto &[shaderName, shaderProgram] : shaderPrograms_)
    {
      // Освобождаем шейдерную программу.
      if (shaderProgram.id != 0)
      {
        glDeleteProgram(shaderProgram.id);
        shaderProgram.id = 0;
      }
    }
    shaderPrograms_.clear();
  }

public:
  void setMoveVert(const MoveVert moveVert) { moveVert_ = moveVert; }

  void setMoveHoriz(const MoveHoriz moveHoriz) { moveHoriz_ = moveHoriz; }

  void setRotateForward(const RotateForward rotateForward) { rotateForward_ = rotateForward; }

  void rotateCamera(const int mouseDeltaX, const int mouseDeltaY)
  {
    rotateCameraAxes(cameraForward_, cameraUp_, mouseDeltaX, mouseDeltaY, scene_.camera.forwardMouseSensitivity);
  }

  void scrollCamera(const int wheelY) { cameraPosition_ += cameraForward_ * scene_.camera.forwardScrollStep * static_cast<float>(wheelY); }

  void drawFrame(const int viewportWidth, const int viewportHeight, const float deltaSeconds)
  {
    const glm::vec3 cameraLeft             = normalize(glm::cross(cameraForward_, cameraUp_), "camera.left");
    const float     sideMoveDirection      = select1m1(moveHoriz_, MoveHoriz::RIGHT, MoveHoriz::LEFT);
    const float     vertMoveDirection      = select1m1(moveVert_, MoveVert::UP, MoveVert::DOWN);
    const float     forwardRotateDirection = select1m1(rotateForward_, RotateForward::RIGHT, RotateForward::LEFT);

    cameraPosition_ += cameraLeft * scene_.camera.sideVelocity * sideMoveDirection * deltaSeconds;
    cameraPosition_ += cameraUp_ * scene_.camera.sideVelocity * vertMoveDirection * deltaSeconds;

    if (forwardRotateDirection != 0)
    {
      const float     angle      = glm::radians(scene_.camera.forwardRotateDegPSec * static_cast<float>(forwardRotateDirection) * deltaSeconds);
      const glm::mat4 rollMatrix = glm::rotate(glm::mat4{1.0F}, angle, normalize(cameraForward_, "camera.forward"));
      cameraUp_                  = normalize(rollMatrix * glm::vec4(cameraUp_, 0.0F), "camera.up");
    }

    const glm::vec3 background_color = scene_.params.backgroundColor;

    // Задаем цвет очистки кадрового буфера.
    glClearColor(background_color[0], background_color[1], background_color[2], 1.0F);
    // Очищаем цветовой буфер и буфер глубины.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (std::size_t instanceIndex = 0; instanceIndex < scene_.instances.size(); ++instanceIndex)
    {
      const scene::ShapeInstance &instance   = scene_.instances[instanceIndex];
      const std::size_t           writeIndex = instanceIndex * scene::ShapeInstance::ComponentCount;

      instanceData_[writeIndex + 0U] = instance.offset[0];
      instanceData_[writeIndex + 1U] = instance.offset[1];
      instanceData_[writeIndex + 2U] = instance.offset[2];
    }

    // Делаем буфер инстансов текущим перед обновлением.
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
    // Загружаем актуальные данные инстансов в GPU.
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceSizeBytes(), instanceData_.data());

    const int       width      = std::max(viewportWidth, 1);
    const int       height     = std::max(viewportHeight, 1);
    const float     aspect     = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 projection = projectionMatrix(scene_.camera.fovDegrees, aspect, scene_.camera.nearPlane, scene_.camera.farPlane);
    const glm::mat4 view       = viewMatrix(cameraPosition_, cameraForward_, cameraUp_);
    const glm::mat4 model{1.0F};

    for (const scene::ShapeGroup &shapeGroup : scene_.shapeGroups)
    {
      const scene::Shape &shape = scene_.shapes[shapeGroup.shapeIndex];
      if (shapeGroup.instanceCount == 0)
      {
        continue;
      }

      const auto shaderProgram = shaderPrograms_.find(shapeGroup.shaderName);
      if (shaderProgram == shaderPrograms_.end())
      {
        throw std::runtime_error("YgoNQK3CtV :: Missing shader program: " + shapeGroup.shaderName);
      }

      // Активируем шейдерную программу группы перед отрисовкой этой группы.
      glUseProgram(shaderProgram->second.id);
      // Передаем матрицу проекции в шейдер.
      glUniformMatrix4fv(shaderProgram->second.projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
      // Передаем матрицу вида в шейдер.
      glUniformMatrix4fv(shaderProgram->second.viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
      // Передаем матрицу модели в шейдер.
      glUniformMatrix4fv(shaderProgram->second.modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(model));
      // Передаем силу солнечного света в шейдер.
      glUniform1f(shaderProgram->second.sunForceLocation, scene_.sun.force);
      // Передаем направление солнечного света в шейдер.
      glUniform3fv(shaderProgram->second.sunDirectionLocation, 1, glm::value_ptr(scene_.sun.direction));
      // Передаем цвет солнечного света в шейдер.
      glUniform3fv(shaderProgram->second.sunColorLocation, 1, glm::value_ptr(scene_.sun.color));

      // Выбираем VAO фигуры перед отрисовкой.
      glBindVertexArray(shapeBufferIds_[shapeGroup.shapeIndex].vertexArrayID);
      // Для каждой группы сдвигаем instanced-атрибут на первый инстанс группы.
      glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
      glVertexAttribPointer(2,
                            scene::ShapeInstance::ComponentCount,
                            GL_FLOAT,
                            GL_FALSE,
                            scene::ShapeInstance::Stride,
                            reinterpret_cast<void *>(shapeGroup.firstInstanceOffset()));
      // Рисуем все инстансы фигуры по индексам.
      glDrawElementsInstanced(GL_TRIANGLES,
                              static_cast<GLsizei>(shape.indexes.size()),
                              GL_UNSIGNED_INT,
                              nullptr,
                              static_cast<GLsizei>(shapeGroup.instanceCount));
    }

    textWriter_.drawFps(viewportWidth, viewportHeight, deltaSeconds);
  }

private:
  scene::Scene                                   scene_;
  GLuint                                         shapeInstanceGroup_ = 0;
  std::vector<ShapeGlBufferIds>                  shapeBufferIds_;
  std::unordered_map<std::string, ShaderProgram> shaderPrograms_;
  std::vector<float>                             instanceData_;
  TextWriter                                     textWriter_;
  glm::vec3                                      cameraPosition_{};
  glm::vec3                                      cameraForward_{};
  glm::vec3                                      cameraUp_{};
  MoveVert                                       moveVert_      = MoveVert::NONE;
  MoveHoriz                                      moveHoriz_     = MoveHoriz::NONE;
  RotateForward                                  rotateForward_ = RotateForward::NONE;

  GLsizeiptr instanceSizeBytes() const { return static_cast<GLsizeiptr>(instanceData_.size() * sizeof(float)); }
};

Render::Render(const std::filesystem::path &scenePath)
    : impl_(std::make_unique<Impl>(scenePath))
{
}

Render::~Render() = default;

void Render::setMoveVert(const MoveVert moveVert) const
{
  impl_->setMoveVert(moveVert);
}

void Render::setMoveHoriz(const MoveHoriz moveHoriz) const
{
  impl_->setMoveHoriz(moveHoriz);
}

void Render::setRotateForward(const RotateForward rotateForward) const
{
  impl_->setRotateForward(rotateForward);
}

void Render::rotateCamera(const int mouseDeltaX, const int mouseDeltaY) const
{
  impl_->rotateCamera(mouseDeltaX, mouseDeltaY);
}

void Render::scrollCamera(const int wheelY) const
{
  impl_->scrollCamera(wheelY);
}

void Render::drawFrame(const int viewportWidth, const int viewportHeight, const float deltaSeconds) const
{
  impl_->drawFrame(viewportWidth, viewportHeight, deltaSeconds);
}
