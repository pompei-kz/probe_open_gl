module;

#include <memory>

export module application;

import main_window;
import render;

export class Application
{
public:
  explicit Application(MainWindow &window);

  Application(const Application &) = delete;

  Application &operator=(const Application &) = delete;

  ~Application();

  void run(Render &render);

private:
  struct Impl;

  std::unique_ptr<Impl> impl_;
};
