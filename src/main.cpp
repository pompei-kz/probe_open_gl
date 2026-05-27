#include <SDL2/SDL.h>
#include <epoxy/gl.h>

import arguments;
import main_window;
import render;

#include <iostream>
#include <stdexcept>
#include <string>

int mainInTry(const Arguments &args);

int main(const int argvCount, char **argv)
{
  const Arguments args(argvCount, argv);

  try
  {
    args.printIntro();

    return mainInTry(args);
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << std::endl;
    return 1;
  }
}

int mainInTry(const Arguments &args)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    std::cerr << "mVCKXbMftG :: SDL_Init failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  MainWindow window(args.cacheFolder(), "OpenGL 3.3 Core Triangle");

  // Выбираем мажорную версию OpenGL-контекста.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  // Выбираем минорную версию OpenGL-контекста.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  // Запрашиваем core profile без устаревших функций OpenGL.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // Включаем двойную буферизацию для плавного вывода кадра.
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Создаем OpenGL-контекст для окна SDL.
  SDL_GLContext context = SDL_GL_CreateContext(window.nativeHandle());
  if (context == nullptr)
  {
    std::cerr << "Ivn1fta1oB :: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
    window.close();
    SDL_Quit();
    return 1;
  }

  // Синхронизируем обмен буферов с вертикальной разверткой.
  SDL_GL_SetSwapInterval(1);
  // Задаем начальную область вывода OpenGL в размере окна.
  glViewport(0, 0, window.width(), window.height());

  // Читаем строку версии OpenGL у текущего контекста.
  std::cout << "MXL4NrIm8M :: OpenGL: " << glGetString(GL_VERSION) << std::endl;
  // Читаем строку версии GLSL у текущего контекста.
  std::cout << "DZ2EDsUp4f :: GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  bool mouseCaptured          = false;
  const auto setMouseCaptured = [&mouseCaptured](const bool captured)
  {
    if (mouseCaptured == captured)
    {
      return;
    }
    if (SDL_SetRelativeMouseMode(captured ? SDL_TRUE : SDL_FALSE) != 0)
    {
      throw std::runtime_error(std::string("Rk4dDWAkY5 :: SDL_SetRelativeMouseMode failed: ") + SDL_GetError());
    }
    mouseCaptured = captured;
  };

  {
    Render render(args.exeFolder() / "scene.yaml");

    bool running           = true;
    Uint64 previousCounter = SDL_GetPerformanceCounter();

    while (running)
    {
      SDL_Event event{};

      while (SDL_PollEvent(&event) != 0)
      {
        if (event.type == SDL_QUIT)
        {
          running = false;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & KMOD_CTRL) != 0)
        {
          running = false;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && event.key.repeat == 0)
        {
          setMouseCaptured(!mouseCaptured);
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && mouseCaptured)
        {
          setMouseCaptured(false);
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_w)
        {
          render.setMoveUp(true);
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s)
        {
          render.setMoveDown(true);
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a)
        {
          render.setMoveLeft(true);
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_d)
        {
          render.setMoveRight(true);
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_w)
        {
          render.setMoveUp(false);
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s)
        {
          render.setMoveDown(false);
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_a)
        {
          render.setMoveLeft(false);
        }
        else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_d)
        {
          render.setMoveRight(false);
        }
        else if (event.type == SDL_MOUSEMOTION && mouseCaptured)
        {
          render.rotateCamera(event.motion.xrel, event.motion.yrel);
        }
        else if (event.type == SDL_MOUSEWHEEL)
        {
          render.scrollCamera(event.wheel.y);
        }
        else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
          glViewport(0, 0, event.window.data1, event.window.data2);
        }
        if (event.type == SDL_WINDOWEVENT)
        {
          window.syncWindowEvent(event.window);
        }
      }

      window.idle();

      const Uint64 currentCounter = SDL_GetPerformanceCounter();
      const float deltaSeconds    = static_cast<float>(currentCounter - previousCounter) / static_cast<float>(SDL_GetPerformanceFrequency());
      previousCounter             = currentCounter;
      render.drawFrame(window.width(), window.height(), deltaSeconds);

      SDL_GL_SwapWindow(window.nativeHandle());
    }
  }

  if (mouseCaptured)
  {
    SDL_SetRelativeMouseMode(SDL_FALSE);
  }

  // Удаляем OpenGL-контекст SDL.
  SDL_GL_DeleteContext(context);
  window.close();
  SDL_Quit();

  std::cout << "kWT0prpHuO :: Application finished" << std::endl;

  return 0;
}
