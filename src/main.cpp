#include <SDL2/SDL.h>

import application;
import arguments;
import main_window;

#include <iostream>
import render;
import world_01;
import world;

int main(const int argvCount, char **argv)
{
  try
  {
    const Arguments args(argvCount, argv);

    args.printIntro();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      std::cerr << "mVCKXbMftG :: SDL_Init failed: " << SDL_GetError() << std::endl;
      return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // Запрашиваем core profile без устаревших функций OpenGL.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    MainWindow window(args.cacheFolder(), "Atom Simulation");

    {
      world::World_01 world;

      const Application application(window);
      const Render      render(args.exeFolder() / "scene.yaml", &world);

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
