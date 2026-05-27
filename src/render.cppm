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
    const GLuint shader     = glCreateShader(type);
    const char *sourceData  = source.data();
    const auto sourceLength = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &sourceData, &sourceLength);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE)
    {
      GLint logLength = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      glGetShaderInfoLog(shader, logLength, nullptr, log.data());
      glDeleteShader(shader);
      throw std::runtime_error("qWcxNTYsV8 :: Shader compilation failed: " + log);
    }

    return shader;
  }

  GLuint createShaderProgram()
  {
    const GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, resources::triangle_vert);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, resources::triangle_frag);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE)
    {
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

export class Render
{
public:
  explicit Render(const std::filesystem::path &scenePath)
  {
    shaderProgram_            = createShaderProgram();
    projectionMatrixLocation_ = glGetUniformLocation(shaderProgram_, "projectionMatrix");
    viewMatrixLocation_       = glGetUniformLocation(shaderProgram_, "viewMatrix");
    modelMatrixLocation_      = glGetUniformLocation(shaderProgram_, "modelMatrix");
    sunForceLocation_         = glGetUniformLocation(shaderProgram_, "sunForce");
    sunDirectionLocation_     = glGetUniformLocation(shaderProgram_, "sunDirection");
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

    glGenBuffers(1, &shapeInstanceGroup_);
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(scene_.instances.size() * 4U * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

    shapeBuffers_.resize(scene_.shapes.size());
    const GLsizei stride                   = static_cast<GLsizei>(scene_.vertexFloatCount * sizeof(float));
    const GLsizei shapeInstanceGroupStride = static_cast<GLsizei>(4U * sizeof(float));
    for (std::size_t shapeIndex = 0; shapeIndex < scene_.shapes.size(); ++shapeIndex)
    {
      const scene::Shape &shape = scene_.shapes[shapeIndex];
      ShapeGlBufferIds &buffers = shapeBuffers_[shapeIndex];

      glGenVertexArrays(1, &buffers.vertexArray);
      glGenBuffers(1, &buffers.vertexBuffer);
      glGenBuffers(1, &buffers.indexBuffer);

      glBindVertexArray(buffers.vertexArray);
      glBindBuffer(GL_ARRAY_BUFFER, buffers.vertexBuffer);
      glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(shape.vertices.size() * sizeof(float)), shape.vertices.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.indexBuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(shape.indexes.size() * sizeof(GLuint)), shape.indexes.data(), GL_STATIC_DRAW);

      glVertexAttribPointer(0, scene_.positionFloatCount, GL_FLOAT, GL_FALSE, stride, nullptr);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, scene_.colorFloatCount, GL_FLOAT, GL_FALSE, stride,
                            reinterpret_cast<void *>(scene_.positionFloatCount * sizeof(float)));
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
      glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, shapeInstanceGroupStride,
                            reinterpret_cast<void *>(shape.firstInstance * static_cast<std::size_t>(shapeInstanceGroupStride)));
      glEnableVertexAttribArray(2);
      glVertexAttribDivisor(2, 1);
    }

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
      glDeleteBuffers(1, &shapeInstanceGroup_);
      shapeInstanceGroup_ = 0;
    }
    for (ShapeGlBufferIds &buffers : shapeBuffers_)
    {
      if (buffers.vertexBuffer != 0)
      {
        glDeleteBuffers(1, &buffers.vertexBuffer);
        buffers.vertexBuffer = 0;
      }
      if (buffers.indexBuffer != 0)
      {
        glDeleteBuffers(1, &buffers.indexBuffer);
        buffers.indexBuffer = 0;
      }
      if (buffers.vertexArray != 0)
      {
        glDeleteVertexArrays(1, &buffers.vertexArray);
        buffers.vertexArray = 0;
      }
    }
    if (shaderProgram_ != 0)
    {
      glDeleteProgram(shaderProgram_);
      shaderProgram_ = 0;
    }
  }

public:
  void setMoveUp(const bool enabled) { moveUp_ = enabled; }

  void setMoveDown(const bool enabled) { moveDown_ = enabled; }

  void setMoveLeft(const bool enabled) { moveLeft_ = enabled; }

  void setMoveRight(const bool enabled) { moveRight_ = enabled; }

  void rotateCamera(const int mouseDeltaX, const int mouseDeltaY)
  {
    rotateForward(cameraForward_, cameraUp_, mouseDeltaX, mouseDeltaY, scene_.camera.forwardMouseSensitivity);
  }

  void scrollCamera(const int wheelY) { cameraPosition_ += cameraForward_ * scene_.camera.forwardScrollStep * static_cast<float>(wheelY); }

  void drawFrame(const int viewportWidth, const int viewportHeight, const float deltaSeconds)
  {
    const glm::vec3 cameraLeft      = normalize(glm::cross(cameraForward_, cameraUp_), "camera.left");
    const int sideMovementDirection = (moveRight_ ? 1 : 0) - (moveLeft_ ? 1 : 0);
    cameraPosition_ += cameraLeft * scene_.camera.sideVelocity * static_cast<float>(sideMovementDirection) * deltaSeconds;
    const int verticalMovementDirection = (moveUp_ ? 1 : 0) - (moveDown_ ? 1 : 0);
    cameraPosition_ += cameraUp_ * scene_.camera.sideVelocity * static_cast<float>(verticalMovementDirection) * deltaSeconds;

    glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram_);
    for (std::size_t instanceIndex = 0; instanceIndex < scene_.instances.size(); ++instanceIndex)
    {
      const scene::ShapeInstance &instance = scene_.instances[instanceIndex];
      const std::size_t writeIndex         = instanceIndex * 4U;
      instanceData_[writeIndex]            = instance.offset[0];
      instanceData_[writeIndex + 1U]       = instance.offset[1];
      instanceData_[writeIndex + 2U]       = instance.offset[2];
      instanceData_[writeIndex + 3U]       = static_cast<float>(instance.shapeIndex);
    }
    glBindBuffer(GL_ARRAY_BUFFER, shapeInstanceGroup_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(instanceData_.size() * sizeof(float)), instanceData_.data());

    const int width            = std::max(viewportWidth, 1);
    const int height           = std::max(viewportHeight, 1);
    const float aspect         = static_cast<float>(width) / static_cast<float>(height);
    const glm::mat4 projection = projectionMatrix(scene_.camera.fovDegrees, aspect, scene_.camera.nearPlane, scene_.camera.farPlane);
    const glm::mat4 view       = viewMatrix(cameraPosition_, cameraForward_, cameraUp_);
    const glm::mat4 model{1.0F};
    glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewMatrixLocation_, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelMatrixLocation_, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(sunForceLocation_, scene_.sun.force);
    glUniform3fv(sunDirectionLocation_, 1, glm::value_ptr(scene_.sun.direction));
    glUniform3fv(sunColorLocation_, 1, glm::value_ptr(scene_.sun.color));

    for (std::size_t shapeIndex = 0; shapeIndex < scene_.shapes.size(); ++shapeIndex)
    {
      const scene::Shape &shape = scene_.shapes[shapeIndex];
      if (shape.instanceCount == 0)
      {
        continue;
      }
      glBindVertexArray(shapeBuffers_[shapeIndex].vertexArray);
      glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(shape.indexes.size()), GL_UNSIGNED_INT, nullptr,
                              static_cast<GLsizei>(shape.instanceCount));
    }
  }

private:
  scene::Scene scene_;
  GLuint shaderProgram_           = 0;
  GLuint shapeInstanceGroup_      = 0;
  GLint projectionMatrixLocation_ = -1;
  GLint viewMatrixLocation_       = -1;
  GLint modelMatrixLocation_      = -1;
  GLint sunForceLocation_         = -1;
  GLint sunDirectionLocation_     = -1;
  GLint sunColorLocation_         = -1;
  std::vector<ShapeGlBufferIds> shapeBuffers_;
  std::vector<float> instanceData_;
  glm::vec3 cameraPosition_{};
  glm::vec3 cameraForward_{};
  glm::vec3 cameraUp_{};
  bool moveUp_    = false;
  bool moveDown_  = false;
  bool moveLeft_  = false;
  bool moveRight_ = false;
};
