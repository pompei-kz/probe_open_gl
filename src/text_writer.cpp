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
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

module text_writer;

import utils;

namespace
{
  constexpr char32_t FallbackCodePoint = U'?';

  struct Glyph
  {
    GLuint     textureID = 0;
    glm::ivec2 size{};
    glm::ivec2 bearing{};
    GLuint     advance = 0;
  };

  GLuint createTextShaderProgram()
  {
    const GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, resources::write_text_vert);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, resources::write_text_frag);
    // Создаем шейдерную программу для отрисовки текста.
    const GLuint program        = glCreateProgram();

    // Подключаем вершинный шейдер текста к программе.
    glAttachShader(program, vertexShader);
    // Подключаем фрагментный шейдер текста к программе.
    glAttachShader(program, fragmentShader);
    // Линкуем шейдерную программу текста.
    glLinkProgram(program);
    // Удаляем объект вершинного шейдера после линковки.
    glDeleteShader(vertexShader);
    // Удаляем объект фрагментарного шейдера после линковки.
    glDeleteShader(fragmentShader);

    GLint success = GL_FALSE;

    // Проверяем статус линковки шейдерной программы текста.
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success != GL_TRUE)
    {
      GLint logLength = 0;
      // Узнаем размер диагностического лога линковки.
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
      std::string log(static_cast<std::size_t>(logLength), '\0');
      // Забираем текст диагностического лога линковки.
      glGetProgramInfoLog(program, logLength, nullptr, log.data());
      // Удаляем неудачно слинкованную программу текста.
      glDeleteProgram(program);
      throw std::runtime_error("wh8JVw0PZs :: Text shader link failed: " + log);
    }

    return program;
  }

  char32_t nextCodePoint(const std::string_view text, std::size_t &index)
  {
    const auto first = static_cast<unsigned char>(text[index++]);
    if (first < 0x80U)
    {
      return first;
    }

    unsigned int byteCount = 0;
    char32_t     result    = 0;
    if ((first & 0xE0U) == 0xC0U)
    {
      byteCount = 2;
      result    = static_cast<char32_t>(first & 0x1FU);
    }
    else if ((first & 0xF0U) == 0xE0U)
    {
      byteCount = 3;
      result    = static_cast<char32_t>(first & 0x0FU);
    }
    else if ((first & 0xF8U) == 0xF0U)
    {
      byteCount = 4;
      result    = static_cast<char32_t>(first & 0x07U);
    }
    else
    {
      return FallbackCodePoint;
    }

    if (text.size() - index < byteCount - 1)
    {
      index = text.size();
      return FallbackCodePoint;
    }

    for (unsigned int i = 1; i < byteCount; ++i)
    {
      const auto value = static_cast<unsigned char>(text[index++]);
      if ((value & 0xC0U) != 0x80U)
      {
        return FallbackCodePoint;
      }
      // ReSharper disable once CppRedundantParentheses
      result = static_cast<char32_t>((result << 6U) | static_cast<char32_t>(value & 0x3FU));
    }

    return result;
  }
} // namespace

struct TextWriter::Impl
{
  Impl()
  {
    shaderProgram_ = createTextShaderProgram();

    // Находим uniform-переменную матрицы проекции текста.
    projectionMatrixLocation_ = glGetUniformLocation(shaderProgram_, "projectionMatrix");

    // Находим uniform-переменную цвета текста.
    colorLocation_ = glGetUniformLocation(shaderProgram_, "textColor");

    // Находим uniform-переменную текстуры глифа.
    textureLocation_ = glGetUniformLocation(shaderProgram_, "textTexture");

    checkPositive(projectionMatrixLocation_, "TtKr7eUlWG :: Failed to locate text shader uniform: projectionMatrix");
    checkPositive(colorLocation_, "amS6l2n0dJ :: Failed to locate text shader uniform: textColor");
    checkPositive(textureLocation_, "As0Bh7Jt52 :: Failed to locate text shader uniform: textTexture");

    loadFont();
    createBuffers();
  }

  ~Impl() { release(); }

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

    // Активируем шейдерную программу для отрисовки текста.
    glUseProgram(shaderProgram_);
    const glm::mat4 projection = glm::ortho(0.0F, static_cast<float>(width), static_cast<float>(height), 0.0F);
    // Передаем орто графическую матрицу проекции в текстовый шейдер.
    glUniformMatrix4fv(projectionMatrixLocation_, 1, GL_FALSE, glm::value_ptr(projection));
    // Передаем цвет текста в текстовый шейдер.
    glUniform3fv(colorLocation_, 1, glm::value_ptr(color));
    // Привязываем sampler текста к нулевому текстурному слоту.
    glUniform1i(textureLocation_, 0);

    // Делаем нулевой текстурный слот активным для текстур глифов.
    glActiveTexture(GL_TEXTURE0);
    // Выбираем VAO с раскладкой вершин текстового квадрата.
    glBindVertexArray(vertexArrayID_);
    // Включаем смешивание для альфа-канала глифов.
    glEnable(GL_BLEND);
    // Настраиваем стандартное альфа-смешивание текста поверх сцены.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Отключаем тест глубины, чтобы текст рисовался поверх 3D-сцены.
    glDisable(GL_DEPTH_TEST);

    std::size_t textIndex = 0;
    while (textIndex < text.size())
    {
      const char32_t codePoint = nextCodePoint(text, textIndex);
      auto           glyphIt   = glyphs_.find(codePoint);
      if (glyphIt == glyphs_.end())
      {
        glyphIt = glyphs_.find(FallbackCodePoint);
        if (glyphIt == glyphs_.end())
        {
          continue;
        }
      }

      const Glyph &glyph = glyphIt->second;
      const float  x_pos = x + static_cast<float>(glyph.bearing.x) * scale;
      const float  y_pos = baselineY - static_cast<float>(glyph.bearing.y) * scale;
      const float  w     = static_cast<float>(glyph.size.x) * scale;
      const float  h     = static_cast<float>(glyph.size.y) * scale;

      // ReSharper disable once CppTemplateArgumentsCanBeDeduced
      const std::array<float, TextVertexBufferFloatCount> vertices{
          x_pos, y_pos + h, 0.0F, 1.0F, x_pos,     y_pos, 0.0F, 0.0F, x_pos + w, y_pos,     1.0F, 0.0F,
          x_pos, y_pos + h, 0.0F, 1.0F, x_pos + w, y_pos, 1.0F, 0.0F, x_pos + w, y_pos + h, 1.0F, 1.0F,
      };

      // Привязываем текстуру текущего глифа.
      glBindTexture(GL_TEXTURE_2D, glyph.textureID);
      // Делаем динамический буфер вершин текста текущим.
      glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID_);
      // Загружаем координаты квадрата текущего глифа в GPU.
      glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data());
      // Рисуем два треугольника, образующие глиф.
      glDrawArrays(GL_TRIANGLES, 0, 6);

      x += static_cast<float>(glyph.advance >> 6) * scale;
    }

    // Отвязываем буфер вершин текста.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Отвязываем VAO текста.
    glBindVertexArray(0);
    // Отвязываем текстуру глифа.
    glBindTexture(GL_TEXTURE_2D, 0);
    // Отключаем смешивание после отрисовки текста.
    glDisable(GL_BLEND);
    // Возвращаем тест глубины для дальнейшей 3D-отрисовки.
    glEnable(GL_DEPTH_TEST);
  }

private:
  void release()
  {
    if (vertexBufferID_ != 0)
    {
      // Освобождаем буфер вершин текста.
      glDeleteBuffers(1, &vertexBufferID_);
      vertexBufferID_ = 0;
    }
    if (vertexArrayID_ != 0)
    {
      // Освобождаем VAO текста.
      glDeleteVertexArrays(1, &vertexArrayID_);
      vertexArrayID_ = 0;
    }
    // ReSharper disable once CppUseElementsView
    for (auto &[codePoint, glyph] : glyphs_)
    {
      if (glyph.textureID != 0)
      {
        // Освобождаем текстуру глифа.
        glDeleteTextures(1, &glyph.textureID);
        glyph.textureID = 0;
      }
    }
    if (shaderProgram_ != 0)
    {
      // Освобождаем шейдерную программу текста.
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

    const std::span<const unsigned char> font = resources::fonts_Roboto_Regular_ttf.binary;

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

    // Выравниваем строки пикселей по одному байту для одноканальных битмапов глифов.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (char32_t codePoint = FirstAsciiGlyph; codePoint <= LastAsciiGlyph; ++codePoint)
    {
      loadGlyph(face, codePoint);
    }

    for (char32_t codePoint = FirstCyrillicGlyph; codePoint <= LastCyrillicGlyph; ++codePoint)
    {
      loadGlyph(face, codePoint);
    }
    loadGlyph(face, CyrillicYoUpperGlyph);
    loadGlyph(face, CyrillicYoLowerGlyph);
    loadGlyph(face, NumeroGlyph);

    // Отвязываем текстуру глифа после загрузки шрифта.
    glBindTexture(GL_TEXTURE_2D, 0);
    // Возвращаем стандартное четырех байтовое выравнивание строк пикселей.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
  }

  void loadGlyph(const FT_Face face, const char32_t codePoint)
  {
    // ReSharper disable once CppRedundantCastExpression
    if (FT_Load_Char(face, static_cast<FT_ULong>(codePoint), FT_LOAD_RENDER) != 0)
    {
      return;
    }

    GLuint textureID = 0;

    // Создаем текстуру для битмапа глифа.
    glGenTextures(1, &textureID);

    // Делаем текстуру текущего глифа активной для настройки и загрузки.
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Загружаем одноканальный битмап глифа в текстуру.
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 static_cast<GLsizei>(face->glyph->bitmap.width),
                 static_cast<GLsizei>(face->glyph->bitmap.rows),
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);

    // Запрещаем повторение текстуры глифа по горизонтали.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // Запрещаем повторение текстуры глифа по вертикали.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Включаем линейную фильтрацию при уменьшении текста.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Включаем линейную фильтрацию при увеличении текста.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glyphs_[codePoint] = Glyph{
        textureID,
        glm::ivec2{static_cast<int>(face->glyph->bitmap.width), static_cast<int>(face->glyph->bitmap.rows)},
        glm::ivec2{face->glyph->bitmap_left, face->glyph->bitmap_top},
        static_cast<GLuint>(face->glyph->advance.x),
    };
  }

  void createBuffers()
  {
    // Создаем VAO для раскладки вершин текста.
    glGenVertexArrays(1, &vertexArrayID_);
    // Создаем динамический буфер вершин текста.
    glGenBuffers(1, &vertexBufferID_);

    // Делаем VAO текста текущим для настройки атрибутов.
    glBindVertexArray(vertexArrayID_);
    // Делаем буфер вершин текста текущим.
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID_);
    // Выделяем память под один прямоугольник глифа.
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(TextVertexBufferFloatCount * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);
    // Описываем атрибут вершин текста: позиция на экране и координаты текстуры.
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    // Включаем атрибут вершин текста.
    glEnableVertexAttribArray(0);
    // Отвязываем буфер вершин текста после настройки.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Отвязываем VAO текста после настройки.
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
      fpsText_           = std::format("FPS: {}", std::lround(fps * 10.0F) / 10.0F);
      fpsElapsedSeconds_ = 0.0F;
      fpsFrameCount_     = 0;
    }
  }

  // Первый загружаемый ASCII-символ: пробел. Всё ниже не нужно для строки FPS и обычного текста.
  static constexpr char32_t FirstAsciiGlyph = 32;

  // Последний загружаемый ASCII-символ: тильда. Диапазон 32..126 покрывает печатные ASCII-глифы.
  static constexpr char32_t LastAsciiGlyph = 126;

  // Первый загружаемый символ основного русского диапазона Unicode: заглавная буква А.
  static constexpr char32_t FirstCyrillicGlyph = U'А';

  // Последний загружаемый символ основного русского диапазона Unicode: строчная буква я.
  static constexpr char32_t LastCyrillicGlyph = U'я';

  // Заглавная буква Ё, которая находится вне непрерывного диапазона А..я.
  static constexpr char32_t CyrillicYoUpperGlyph = U'Ё';

  // Строчная буква ё, которая находится вне непрерывного диапазона А..я.
  static constexpr char32_t CyrillicYoLowerGlyph = U'ё';

  // Символ номера №, часто используемый в русском интерфейсном тексте.
  static constexpr char32_t NumeroGlyph = U'№';

  // Высота глифа в пикселях, которую FreeType использует при растеризации встроенного шрифта.
  static constexpr unsigned int FontPixelSize = 20;

  // Размер временного буфера вершин для одного глифа: шесть вершин по четыре float-компонента.
  static constexpr std::size_t TextVertexBufferFloatCount = 6 * 4;

  // OpenGL-программа, которая рисует текстовые квадраты и берет прозрачность из текстуры глифа.
  GLuint shaderProgram_ = 0;

  // VAO с описанием одного атрибута вершины: экранные координаты xy и текстурные координаты uv.
  GLuint vertexArrayID_ = 0;

  // Динамический VBO, в который перед каждым глифом загружается шесть вершин его прямоугольника.
  GLuint vertexBufferID_ = 0;

  // Позиция uniform-переменной орто графической матрицы проекции в текстовой программе.
  GLint projectionMatrixLocation_ = -1;

  // Позиция uniform-переменной цвета текста в текстовой программе.
  GLint colorLocation_ = -1;

  // Позиция uniform-переменной sampler2D, читающей текстуру текущего глифа.
  GLint textureLocation_ = -1;

  // Таблица Unicode-глифов: текстура, размер битмапа, смещение относительно базовой линии и шаг пера.
  std::unordered_map<char32_t, Glyph> glyphs_{};

  // Готовая строка FPS, обновляемая не каждый кадр, чтобы текст не дергался слишком часто.
  std::string fpsText_ = "FPS: 0";

  // Накопленное время окна измерения FPS в секундах.
  float fpsElapsedSeconds_ = 0.0F;

  // Количество кадров, накопленных за текущее окно измерения FPS.
  int fpsFrameCount_ = 0;

  // Высота подъема шрифта над базовой линией в пикселях; нужна для позиционирования текста от верхнего края.
  float ascenderPixels_ = static_cast<float>(FontPixelSize);
};

TextWriter::TextWriter()
    : impl_(std::make_unique<Impl>())
{
}

TextWriter::~TextWriter() = default;

void TextWriter::drawFps(const int viewportWidth, const int viewportHeight, const float deltaSeconds) const
{
  impl_->drawFps(viewportWidth, viewportHeight, deltaSeconds);
}

void TextWriter::drawText(const int              viewportWidth,
                          const int              viewportHeight,
                          const std::string_view text,
                          const float            left,
                          const float            top,
                          const float            scale,
                          const glm::vec3       &color) const
{
  impl_->drawText(viewportWidth, viewportHeight, text, left, top, scale, color);
}
