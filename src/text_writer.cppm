module;

#include <glm/glm.hpp>

#include <memory>
#include <string_view>

export module text_writer;

export class TextWriter
{
public:
  TextWriter();

  TextWriter(const TextWriter &) = delete;

  TextWriter &operator=(const TextWriter &) = delete;

  TextWriter(TextWriter &&) = delete;

  TextWriter &operator=(TextWriter &&) = delete;

  ~TextWriter();

  void drawFps(int viewportWidth, int viewportHeight, float deltaSeconds) const;

  void drawText(int viewportWidth, int viewportHeight, std::string_view text, float left, float top, float scale, const glm::vec3 &color) const;

private:
  struct Impl;

  // Закрытая реализация отрисовки текста. Изменения внутри Impl не меняют BMI модуля text_writer.
  std::unique_ptr<Impl> impl_;
};
