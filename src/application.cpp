module;

#include <SDL2/SDL.h>
#include <epoxy/gl.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

module application;

import main_window;
import render;
import keys;

namespace
{
  enum class KeyboardMode
  {
    Base,
    Shift,
    Ctrl,
    Alt,
  };

  KeyboardMode keyboardMode(const Uint16 mod)
  {
    if ((mod & KMOD_CTRL) != 0)
    {
      return KeyboardMode::Ctrl;
    }
    if ((mod & KMOD_ALT) != 0)
    {
      return KeyboardMode::Alt;
    }
    if ((mod & KMOD_SHIFT) != 0)
    {
      return KeyboardMode::Shift;
    }
    return KeyboardMode::Base;
  }

  bool hasModifier(const Uint16 mod)
  {
    return keyboardMode(mod) != KeyboardMode::Base;
  }
} // namespace

struct Application::Impl
{
  explicit Impl(MainWindow &window)
      : window_(window)
  {
    context_ = SDL_GL_CreateContext(window_.nativeHandle());
    if (context_ == nullptr)
    {
      throw std::runtime_error(std::string("Ivn1fta1oB :: SDL_GL_CreateContext failed: ") + SDL_GetError());
    }

    // Включаем синхронизацию смены буферов с вертикальной разверткой.
    SDL_GL_SetSwapInterval(1);
    // Задаем начальный viewport под текущий размер окна.
    glViewport(0, 0, window_.width(), window_.height());

    // Читаем строку версии текущего OpenGL-контекста.
    // Запрашиваем у OpenGL строку версии текущего контекста.
    std::cout << "MXL4NrIm8M :: OpenGL: " << glGetString(GL_VERSION) << std::endl;
    // Читаем строку версии GLSL у текущего OpenGL-контекста.
    // Запрашиваем у OpenGL строку версии языка шейдеров.
    std::cout << "DZ2EDsUp4f :: GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  }

  ~Impl()
  {
    if (mouseCaptured_)
    {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    if (context_ != nullptr)
    {
      SDL_GL_DeleteContext(context_);
    }
  }

  void run(const Render &render)
  {
    Uint64 previousCounter = SDL_GetPerformanceCounter();

    render.init();

    running_ = true;
    while (running_)
    {
      processEvents(render);

      window_.idle();

      const Uint64 currentCounter = SDL_GetPerformanceCounter();
      const float  deltaSeconds   = static_cast<float>(currentCounter - previousCounter) / static_cast<float>(SDL_GetPerformanceFrequency());
      previousCounter             = currentCounter;
      render.drawFrame(window_.width(), window_.height(), deltaSeconds);

      SDL_GL_SwapWindow(window_.nativeHandle());
    }
  }

private:
  void processEvent(const SDL_Event &event, const Render &render)
  {
    if (event.type == SDL_QUIT)
    {
      running_ = false;
      return;
    }
    if (keys::KeyDown_CtrlQ(event))
    {
      running_ = false;
      return;
    }
    if (keys::KeyDown_FreeSpace(event))
    {
      setMouseCaptured(!mouseCaptured_);
      return;
    }
    if (keys::MouseDown_FreePrimary(event) && mouseCaptured_)
    {
      setMouseCaptured(false);
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_w && hasModifier(event.key.keysym.mod))
    {
      switch (keyboardMode(event.key.keysym.mod))
      {
      case KeyboardMode::Shift:
      case KeyboardMode::Ctrl:
        render.rotateCamera(0, -8);
        return;
      case KeyboardMode::Alt:
        render.scrollCamera(1);
        return;
      case KeyboardMode::Base:
        break;
      }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s && hasModifier(event.key.keysym.mod))
    {
      switch (keyboardMode(event.key.keysym.mod))
      {
      case KeyboardMode::Shift:
      case KeyboardMode::Ctrl:
        render.rotateCamera(0, 8);
        return;
      case KeyboardMode::Alt:
        render.scrollCamera(-1);
        return;
      case KeyboardMode::Base:
        break;
      }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a && hasModifier(event.key.keysym.mod))
    {
      switch (keyboardMode(event.key.keysym.mod))
      {
      case KeyboardMode::Shift:
        render.rotateCamera(-8, 0);
        return;
      case KeyboardMode::Ctrl:
        render.scrollCamera(1);
        return;
      case KeyboardMode::Alt:
        render.rotateCamera(0, -8);
        return;
      case KeyboardMode::Base:
        break;
      }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_d && hasModifier(event.key.keysym.mod))
    {
      switch (keyboardMode(event.key.keysym.mod))
      {
      case KeyboardMode::Shift:
        render.rotateCamera(8, 0);
        return;
      case KeyboardMode::Ctrl:
        render.scrollCamera(-1);
        return;
      case KeyboardMode::Alt:
        render.rotateCamera(0, 8);
        return;
      case KeyboardMode::Base:
        break;
      }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q && hasModifier(event.key.keysym.mod))
    {
      switch (keyboardMode(event.key.keysym.mod))
      {
      case KeyboardMode::Shift:
        render.scrollCamera(1);
        return;
      case KeyboardMode::Ctrl:
        render.rotateCamera(-8, 0);
        return;
      case KeyboardMode::Alt:
        render.rotateCamera(8, 0);
        return;
      case KeyboardMode::Base:
        break;
      }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_e && hasModifier(event.key.keysym.mod))
    {
      switch (keyboardMode(event.key.keysym.mod))
      {
      case KeyboardMode::Shift:
        render.scrollCamera(-1);
        return;
      case KeyboardMode::Ctrl:
        render.rotateCamera(8, 0);
        return;
      case KeyboardMode::Alt:
        render.rotateCamera(-8, 0);
        return;
      case KeyboardMode::Base:
        break;
      }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_w)
    {
      render.setMoveVert(MoveVert::UP);
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s)
    {
      render.setMoveVert(MoveVert::DOWN);
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a)
    {
      render.setMoveHoriz(MoveHoriz::LEFT);
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_d)
    {
      render.setMoveHoriz(MoveHoriz::RIGHT);
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q)
    {
      render.setRotateForward(RotateForward::RIGHT);
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_e)
    {
      render.setRotateForward(RotateForward::LEFT);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_w)
    {
      if (hasModifier(event.key.keysym.mod))
      {
        return;
      }
      render.setMoveVert(MoveVert::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s)
    {
      if (hasModifier(event.key.keysym.mod))
      {
        return;
      }
      render.setMoveVert(MoveVert::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_a)
    {
      if (hasModifier(event.key.keysym.mod))
      {
        return;
      }
      render.setMoveHoriz(MoveHoriz::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_d)
    {
      if (hasModifier(event.key.keysym.mod))
      {
        return;
      }
      render.setMoveHoriz(MoveHoriz::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_q)
    {
      if (hasModifier(event.key.keysym.mod))
      {
        return;
      }
      render.setRotateForward(RotateForward::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_e)
    {
      if (hasModifier(event.key.keysym.mod))
      {
        return;
      }
      render.setRotateForward(RotateForward::NONE);
      return;
    }
    if (event.type == SDL_MOUSEMOTION && mouseCaptured_)
    {
      render.rotateCamera(event.motion.xrel, event.motion.yrel);
      return;
    }
    if (event.type == SDL_MOUSEWHEEL)
    {
      render.scrollCamera(event.wheel.y);
      return;
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    {
      // Обновляем viewport под новый размер окна.
      glViewport(0, 0, event.window.data1, event.window.data2);
      return;
    }
  }

  void processEvents(const Render &render)
  {
    SDL_Event event{};

    while (SDL_PollEvent(&event) != 0)
    {
      processEvent(event, render);

      if (event.type == SDL_WINDOWEVENT)
      {
        window_.syncWindowEvent(event.window);
      }
    }
  }

  void setMouseCaptured(const bool captured)
  {
    if (mouseCaptured_ == captured)
    {
      return;
    }
    if (SDL_SetRelativeMouseMode(captured ? SDL_TRUE : SDL_FALSE) != 0)
    {
      throw std::runtime_error(std::string("Rk4dDWAkY5 :: SDL_SetRelativeMouseMode failed: ") + SDL_GetError());
    }
    mouseCaptured_ = captured;
  }

  MainWindow   &window_;
  SDL_GLContext context_       = nullptr;
  bool          mouseCaptured_ = false;
  bool          running_       = false;
};

Application::Application(MainWindow &window)
    : impl_(std::make_unique<Impl>(window))
{
}

Application::~Application() = default;

void Application::run(const Render &render) const
{
  impl_->run(render);
}
