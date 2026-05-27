module;

#include <SDL2/SDL.h>
#include <epoxy/gl.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

export module application;

import main_window;
import render;

export class Application
{
public:
  Application(MainWindow &window)
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
    std::cout << "MXL4NrIm8M :: OpenGL: " << glGetString(GL_VERSION) << std::endl;
    // Читаем строку версии GLSL у текущего OpenGL-контекста.
    std::cout << "DZ2EDsUp4f :: GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  }

  Application(const Application &) = delete;

  Application &operator=(const Application &) = delete;

  ~Application()
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

  void run(Render &render)
  {
    Uint64 previousCounter = SDL_GetPerformanceCounter();

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
  void processEvent(const SDL_Event &event, Render &render)
  {
    if (event.type == SDL_QUIT)
    {
      running_ = false;
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & KMOD_CTRL) != 0)
    {
      running_ = false;
      return;
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && event.key.repeat == 0)
    {
      setMouseCaptured(!mouseCaptured_);
      return;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN && mouseCaptured_)
    {
      setMouseCaptured(false);
      return;
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
      render.setMoveVert(MoveVert::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s)
    {
      render.setMoveVert(MoveVert::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_a)
    {
      render.setMoveHoriz(MoveHoriz::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_d)
    {
      render.setMoveHoriz(MoveHoriz::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_q)
    {
      render.setRotateForward(RotateForward::NONE);
      return;
    }
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_e)
    {
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

  void processEvents(Render &render)
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
