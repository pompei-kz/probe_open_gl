module;

#include <epoxy/gl.h>

#include "resources.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

export module utils;

export template <typename T> float select1m1(T value, const T one, const T minusOne)
{
  if (value == one) return 1.0F;
  if (value == minusOne) return -1.0F;
  return 0.0F;
}

export template <typename T> void checkPositive(T value, const std::string_view errMessage)
{
  if (value < 0) throw std::runtime_error(errMessage.data());
}

export GLuint compileShader(const GLenum type, const resources::TxtResource &source)
{
  // Создаем объект шейдера указанного типа.
  const GLuint shader       = glCreateShader(type);
  const char  *sourceData   = source.text.data();
  const auto   sourceLength = static_cast<GLint>(source.text.size());
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
    throw std::runtime_error("RiT19tX2PN :: Shader compilation failed in " + std::string(source.path) + ":\n" + log);
  }

  return shader;
}
