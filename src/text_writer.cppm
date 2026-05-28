module;

#include <epoxy/gl.h>
#include <ft2build.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include FT_FREETYPE_H

#include "resources.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

export module text_writer;

import utils;

namespace
{
  struct Glyph
  {
    GLuint     textureID = 0;
    glm::ivec2 size{};
    glm::ivec2 bearing{};
    GLuint     advance = 0;
  };

  GLuint compileShader(const GLenum type, const std::string_view source)
  {
    const GLuint shader       = glCreateShader(type);
    const char  *sourceData   = source.data();
    const auto   sourceLength = static_cast<GLint>(source.size());
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

  GLuint createTextShaderProgram()
  {
    constexpr std::string_view vertexShaderSource   = resources::write_text_vert;
    constexpr std::string_view fragmentShaderSource = resources::write_text_frag;

    const GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    const GLuint program        = glCreateProgram();

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
      throw std::runtime_error("wh8JVw0PZs :: Text shader link failed: " + log);
    }

    return program;
  }
} // namespace

export class TextWriter
{
public:
  TextWriter()
  {
    shaderProgram_            = createTextShaderProgram();
    projectionMatrixLocation_ = glGetUniformLocation(shaderProgram_, "projectionMatrix");
    colorLocation_            = glGetUniformLocation(shaderProgram_, "textColor");
    textureLocation_          = glGetUniformLocation(shaderProgram_, "textTexture");
    checkPositive(projectionMatrixLocation_, "TtKr7eUlWG :: Failed to locate text shader uniform: projectionMatrix");
    checkPositive(colorLocation_, "amS6l2n0dJ :: Failed to locate text shader uniform: textColor");
    checkPositive(textureLocation_, "As0Bh7Jt52 :: Failed to locate text shader uniform: textTexture");

    loadFont();
    createBuffers();
  }

  TextWriter(const TextWriter &) = delete;

  TextWriter &operator=(const TextWriter &) = delete;

  TextWriter(TextWriter &&) = delete;

  TextWriter &operator=(TextWriter &&) = delete;

  ~TextWriter() { release(); }

  void drawFps(const int viewportWidth, const int viewportHeight, const float deltaSeconds)
  {
    updateFpsText(deltaSeconds);
    drawText(viewportWidth, viewportHeight, fpsText_, 8.0F, 8.0F, 1.0F, glm::vec3{0.96F, 0.97F, 1.0F});
  }

  void drawText(const int              viewportWidth,
                const int              viewportHeight,
                const std::string_view text,
                const float            left,
                const float            top,
                const float            scale,
                const glm::vec3       &color) const
  {
    const int   width     = std::max(viewportWidth, 1);
    const int   height    = std::max(viewportHeight, 1);
    float       x         = left;
    const float baselineY = top + ascenderPixels_ * scale;

    glUseProgram(shaderProgram_);
    const glm::mat4 projection = glm::ortho(0.0F, static_cast<float>(width), static_cast<float>(height), 0.0F);
    glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(colorLocation_, 1, glm::value_ptr(color));
    glUniform1i(textureLocation_, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vertexArrayID_);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    for (const char value : text)
    {
      const auto character = static_cast<unsigned char>(value);
      if (character >= glyphs_.size())
      {
        continue;
      }

      const Glyph &glyph = glyphs_[character];
      if (glyph.textureID == 0)
      {
        continue;
      }

      const float x_pos = x + static_cast<float>(glyph.bearing.x) * scale;
      const float y_pos = baselineY - static_cast<float>(glyph.bearing.y) * scale;
      const float w     = static_cast<float>(glyph.size.x) * scale;
      const float h     = static_cast<float>(glyph.size.y) * scale;

      // ReSharper disable once CppTemplateArgumentsCanBeDeduced
      const std::array<float, TextVertexBufferFloatCount> vertices{
          x_pos, y_pos + h, 0.0F, 1.0F, x_pos,     y_pos, 0.0F, 0.0F, x_pos + w, y_pos,     1.0F, 0.0F,
          x_pos, y_pos + h, 0.0F, 1.0F, x_pos + w, y_pos, 1.0F, 0.0F, x_pos + w, y_pos + h, 1.0F, 1.0F,
      };

      glBindTexture(GL_TEXTURE_2D, glyph.textureID);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID_);
      glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data());
      glDrawArrays(GL_TRIANGLES, 0, 6);

      x += static_cast<float>(glyph.advance >> 6) * scale;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }

private:
  void release()
  {
    if (vertexBufferID_ != 0)
    {
      glDeleteBuffers(1, &vertexBufferID_);
      vertexBufferID_ = 0;
    }
    if (vertexArrayID_ != 0)
    {
      glDeleteVertexArrays(1, &vertexArrayID_);
      vertexArrayID_ = 0;
    }
    for (Glyph &glyph : glyphs_)
    {
      if (glyph.textureID != 0)
      {
        glDeleteTextures(1, &glyph.textureID);
        glyph.textureID = 0;
      }
    }
    if (shaderProgram_ != 0)
    {
      glDeleteProgram(shaderProgram_);
      shaderProgram_ = 0;
    }
  }

  void loadFont()
  {
    FT_Library library = nullptr;
    if (FT_Init_FreeType(&library) != 0)
    {
      throw std::runtime_error("AWv2sDuFfl :: Failed to initialize FreeType");
    }

    const auto font = resources::fonts_Roboto_Regular_ttf();

    FT_Face face = nullptr;
    if (FT_New_Memory_Face(library, font.data(), static_cast<FT_Long>(font.size()), 0, &face) != 0)
    {
      FT_Done_FreeType(library);
      throw std::runtime_error("bsWec3cGVN :: Failed to load embedded Roboto-Regular.ttf");
    }

    if (FT_Set_Pixel_Sizes(face, 0, FontPixelSize) != 0)
    {
      FT_Done_Face(face);
      FT_Done_FreeType(library);
      throw std::runtime_error("DGYQhYw31S :: Failed to set font pixel size");
    }

    if (face->size != nullptr)
    {
      ascenderPixels_ = static_cast<float>(face->size->metrics.ascender >> 6);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char character = FirstGlyph; character <= LastGlyph; ++character)
    {
      if (FT_Load_Char(face, character, FT_LOAD_RENDER) != 0)
      {
        continue;
      }

      GLuint textureID = 0;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RED,
                   static_cast<GLsizei>(face->glyph->bitmap.width),
                   static_cast<GLsizei>(face->glyph->bitmap.rows),
                   0,
                   GL_RED,
                   GL_UNSIGNED_BYTE,
                   face->glyph->bitmap.buffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glyphs_[character] = Glyph{
          textureID,
          glm::ivec2{static_cast<int>(face->glyph->bitmap.width), static_cast<int>(face->glyph->bitmap.rows)},
          glm::ivec2{face->glyph->bitmap_left, face->glyph->bitmap_top},
          static_cast<GLuint>(face->glyph->advance.x),
      };
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
  }

  void createBuffers()
  {
    glGenVertexArrays(1, &vertexArrayID_);
    glGenBuffers(1, &vertexBufferID_);

    glBindVertexArray(vertexArrayID_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(TextVertexBufferFloatCount * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  void updateFpsText(const float deltaSeconds)
  {
    if (deltaSeconds <= 0.0F)
    {
      return;
    }

    fpsElapsedSeconds_ += deltaSeconds;
    ++fpsFrameCount_;

    if (fpsElapsedSeconds_ >= 0.25F)
    {
      const float fps    = static_cast<float>(fpsFrameCount_) / fpsElapsedSeconds_;
      fpsText_           = std::format("FPS: {}", static_cast<int>(std::lround(fps)));
      fpsElapsedSeconds_ = 0.0F;
      fpsFrameCount_     = 0;
    }
  }

  static constexpr unsigned char FirstGlyph                 = 32;
  static constexpr unsigned char LastGlyph                  = 126;
  static constexpr unsigned int  FontPixelSize              = 20;
  static constexpr std::size_t   TextVertexBufferFloatCount = 6 * 4;

  GLuint                 shaderProgram_            = 0;
  GLuint                 vertexArrayID_            = 0;
  GLuint                 vertexBufferID_           = 0;
  GLint                  projectionMatrixLocation_ = -1;
  GLint                  colorLocation_            = -1;
  GLint                  textureLocation_          = -1;
  std::array<Glyph, 128> glyphs_{};
  std::string            fpsText_           = "FPS: 0";
  float                  fpsElapsedSeconds_ = 0.0F;
  int                    fpsFrameCount_     = 0;
  float                  ascenderPixels_    = static_cast<float>(FontPixelSize);
};
