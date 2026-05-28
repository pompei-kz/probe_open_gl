module;

#include <SDL2/SDL.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

module main_window;

import deferred_tasks;

struct MainWindow::Impl
{
  Impl(const std::filesystem::path &cacheDirectory, const char *title)
      : positionPath_(cacheDirectory / "main_window_position.txt")
      , sizePath_(cacheDirectory / "main_window_size.txt")
      , size_(loadWindowSize(sizePath_).value_or(WindowSize{}))
      , position_(loadWindowPosition(positionPath_))
  {
    window_ = SDL_CreateWindow(title,
                               position_ ? position_->left : SDL_WINDOWPOS_CENTERED,
                               position_ ? position_->top : SDL_WINDOWPOS_CENTERED,
                               size_.width,
                               size_.height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (window_ == nullptr)
    {
      throw std::runtime_error(std::string("jcxLsEPi1X :: SDL_CreateWindow failed: ") + SDL_GetError());
    }
  }

  Impl(Impl &&other) noexcept
      : window_(other.window_)
      , positionPath_(std::move(other.positionPath_))
      , sizePath_(std::move(other.sizePath_))
      , size_(other.size_)
      , position_(other.position_)
  {
    other.window_ = nullptr;
  }

  Impl &operator=(Impl &&other) noexcept
  {
    if (this != &other)
    {
      close();
      window_       = other.window_;
      positionPath_ = std::move(other.positionPath_);
      sizePath_     = std::move(other.sizePath_);
      size_         = other.size_;
      position_     = other.position_;
      other.window_ = nullptr;
    }
    return *this;
  }

  ~Impl() { close(); }

  [[nodiscard]] SDL_Window *nativeHandle() const { return window_; }

  [[nodiscard]] int width() const { return size_.width; }

  [[nodiscard]] int height() const { return size_.height; }

  void syncWindowEvent(const SDL_WindowEvent &event)
  {
    if (event.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    {
      size_ = WindowSize{event.data1, event.data2};
      saveWindowSize(size_);
      return;
    }

    if (event.event == SDL_WINDOWEVENT_MOVED)
    {
      position_ = outerWindowPosition(WindowPosition{event.data1, event.data2});
      saveWindowPosition(*position_);
      return;
    }
  }

  void close()
  {
    if (window_ != nullptr)
    {
      SDL_DestroyWindow(window_);
      window_ = nullptr;
    }
  }

  void idle() const { deferredTasks_.idle(std::chrono::system_clock::now()); }

private:
  struct WindowPosition
  {
    int left = 0;
    int top  = 0;
  };

  struct WindowSize
  {
    int width  = 800;
    int height = 600;
  };

  SDL_Window                   *window_ = nullptr;
  std::filesystem::path         positionPath_;
  std::filesystem::path         sizePath_;
  WindowSize                    size_;
  std::optional<WindowPosition> position_;
  DeferredTasks                 deferredTasks_;

  static std::string trim(const std::string_view value)
  {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos)
    {
      return {};
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(begin, end - begin + 1));
  }

  static std::optional<WindowPosition> loadWindowPosition(const std::filesystem::path &path)
  {
    std::ifstream input(path);
    if (!input)
    {
      return std::nullopt;
    }

    std::optional<int> left;
    std::optional<int> top;
    std::string        line;
    while (std::getline(input, line))
    {
      const std::string cleaned = trim(line.substr(0, line.find('#')));
      if (cleaned.empty())
      {
        continue;
      }

      const auto separator = cleaned.find('=');
      if (separator == std::string::npos)
      {
        continue;
      }

      const std::string key   = trim(std::string_view(cleaned).substr(0, separator));
      const int         value = std::stoi(trim(std::string_view(cleaned).substr(separator + 1)));
      if (key == "left")
      {
        left = value;
      }
      else if (key == "top")
      {
        top = value;
      }
    }

    if (!left || !top)
    {
      return std::nullopt;
    }
    return WindowPosition{*left, *top};
  }

  static std::optional<WindowSize> loadWindowSize(const std::filesystem::path &path)
  {
    std::ifstream input(path);
    if (!input)
    {
      return std::nullopt;
    }

    std::string line;
    while (std::getline(input, line))
    {
      const std::string cleaned = trim(line.substr(0, line.find('#')));
      if (cleaned.empty())
      {
        continue;
      }

      const auto separator = cleaned.find('x');
      if (separator == std::string::npos)
      {
        return std::nullopt;
      }

      const int width  = std::stoi(trim(std::string_view(cleaned).substr(0, separator)));
      const int height = std::stoi(trim(std::string_view(cleaned).substr(separator + 1)));
      if (width <= 0 || height <= 0)
      {
        return std::nullopt;
      }
      return WindowSize{width, height};
    }

    return std::nullopt;
  }

  [[nodiscard]] WindowPosition outerWindowPosition(const WindowPosition reportedPosition) const
  {
    int top    = 0;
    int left   = 0;
    int bottom = 0;
    int right  = 0;
    if (window_ != nullptr && SDL_GetWindowBordersSize(window_, &top, &left, &bottom, &right) == 0)
    {
      return WindowPosition{
          reportedPosition.left - left,
          reportedPosition.top - top,
      };
    }
    return reportedPosition;
  }

  void saveWindowPosition(const WindowPosition position) const
  {
    const std::filesystem::path path = positionPath_;
    deferredTasks_.add(std::chrono::milliseconds{700},
                       "SaveMainWindowPosition",
                       [path, position]
                       {
                         std::filesystem::create_directories(path.parent_path());
                         std::ofstream output(path);
                         output << "left=" << position.left << '\n' << "top=" << position.top << '\n';
                       });
  }

  void saveWindowSize(const WindowSize size) const
  {
    const std::filesystem::path path = sizePath_;
    deferredTasks_.add(std::chrono::milliseconds{700},
                       "SaveMainWindowSize",
                       [path, size]
                       {
                         std::filesystem::create_directories(path.parent_path());
                         std::ofstream output(path);
                         output << size.width << 'x' << size.height << '\n';
                       });
  }
};

MainWindow::MainWindow(const std::filesystem::path &cacheDirectory, const char *title)
    : impl_(std::make_unique<Impl>(cacheDirectory, title))
{
}

MainWindow::MainWindow(MainWindow &&) noexcept = default;

MainWindow &MainWindow::operator=(MainWindow &&) noexcept = default;

MainWindow::~MainWindow() = default;

SDL_Window *MainWindow::nativeHandle() const
{
  return impl_->nativeHandle();
}

int MainWindow::width() const
{
  return impl_->width();
}

int MainWindow::height() const
{
  return impl_->height();
}

void MainWindow::syncWindowEvent(const SDL_WindowEvent &event) const
{
  impl_->syncWindowEvent(event);
}

void MainWindow::close() const
{
  impl_->close();
}

void MainWindow::idle() const
{
  impl_->idle();
}
