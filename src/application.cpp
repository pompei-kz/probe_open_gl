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
    if (keys::Quit(event))
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

    if (keys::KeyDown_FreeW(event))
    {
      render.setMoveVert(MoveVert::UP);
      return;
    }
    if (keys::KeyDown_FreeS(event))
    {
      render.setMoveVert(MoveVert::DOWN);
      return;
    }
    if (keys::KeyDown_FreeA(event))
    {
      render.setMoveHoriz(MoveHoriz::LEFT);
      return;
    }
    if (keys::KeyDown_FreeD(event))
    {
      render.setMoveHoriz(MoveHoriz::RIGHT);
      return;
    }
    if (keys::KeyDown_FreeQ(event))
    {
      render.setRotateForward(RotateForward::RIGHT);
      return;
    }
    if (keys::KeyDown_FreeE(event))
    {
      render.setRotateForward(RotateForward::LEFT);
      return;
    }
    if (keys::KeyUp_FreeW(event))
    {
      render.setMoveVert(MoveVert::NONE);
      return;
    }
    if (keys::KeyUp_FreeS(event))
    {
      render.setMoveVert(MoveVert::NONE);
      return;
    }
    if (keys::KeyUp_FreeA(event))
    {
      render.setMoveHoriz(MoveHoriz::NONE);
      return;
    }
    if (keys::KeyUp_FreeD(event))
    {
      render.setMoveHoriz(MoveHoriz::NONE);
      return;
    }
    if (keys::KeyUp_FreeQ(event))
    {
      render.setRotateForward(RotateForward::NONE);
      return;
    }
    if (keys::KeyUp_FreeE(event))
    {
      render.setRotateForward(RotateForward::NONE);
      return;
    }
    if (keys::MouseMotion_Free(event) && mouseCaptured_)
    {
      render.rotateCamera(event.motion.xrel, event.motion.yrel);
      return;
    }
    if (keys::MouseWheel_Free(event))
    {
      render.scrollCamera(event.wheel.y);
      return;
    }
    if (keys::Window_SizeChanged(event))
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
