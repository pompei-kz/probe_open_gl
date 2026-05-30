module;

#include <memory>

export module application;

import main_window;
import render;

export class Application
{
  struct Impl;
  std::unique_ptr<Impl> impl_;

public:
  explicit Application(MainWindow &window);

  Application(const Application &) = delete;

  Application &operator=(const Application &) = delete;

  ~Application();

  void run(const Render &render) const;
};
