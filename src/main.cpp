#include <SDL2/SDL.h>

import application;
import arguments;
import main_window;

#include <iostream>
import render;

int main(const int argvCount, char **argv)
{
  const Arguments args(argvCount, argv);

  try
  {
    args.printIntro();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      std::cerr << "mVCKXbMftG :: SDL_Init failed: " << SDL_GetError() << std::endl;
      return 1;
    }

    // Выбираем мажорную версию OpenGL-контекста.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    // Выбираем минорную версию OpenGL-контекста.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // Запрашиваем core profile без устаревших функций OpenGL.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // Включаем двойную буферизацию для плавного вывода кадра.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    MainWindow window(args.cacheFolder(), "OpenGL 3.3 Core Triangle");

    {
      Application application(window);
      Render render(args.exeFolder() / "scene.yaml");

      application.run(render);
    }

    window.close();
    SDL_Quit();

    std::cout << "kWT0prpHuO :: Application finished" << std::endl;

    return 0;
  }
  catch (const std::exception &exception)
  {
    std::cerr << exception.what() << std::endl;
    return 1;
  }
}
