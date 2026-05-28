module;

#include <SDL2/SDL.h>

#include <filesystem>
#include <memory>

export module main_window;

export class MainWindow
{
public:
  MainWindow(const std::filesystem::path &cacheDirectory, const char *title);

  MainWindow(const MainWindow &) = delete;

  MainWindow &operator=(const MainWindow &) = delete;

  MainWindow(MainWindow &&other) noexcept;

  MainWindow &operator=(MainWindow &&other) noexcept;

  ~MainWindow();

  [[nodiscard]] SDL_Window *nativeHandle() const;

  [[nodiscard]] int width() const;

  [[nodiscard]] int height() const;

  void syncWindowEvent(const SDL_WindowEvent &event);

  void close();

  void idle();

private:
  struct Impl;

  std::unique_ptr<Impl> impl_;
};
