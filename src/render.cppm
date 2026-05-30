module;

#include <filesystem>
#include <memory>

import world;

export module render;

export enum class MoveVert
{
  NONE,
  UP,
  DOWN,
};

export enum class MoveHoriz
{
  NONE,
  LEFT,
  RIGHT,
};

export enum class RotateForward
{
  NONE,
  LEFT,
  RIGHT,
};

export class Render
{
public:
  explicit Render(const std::filesystem::path &scenePath, world::World *world);

  Render(const Render &) = delete;

  Render &operator=(const Render &) = delete;

  Render(Render &&) = delete;

  Render &operator=(Render &&) = delete;

  ~Render();

  void setMoveVert(MoveVert moveVert) const;

  void setMoveHoriz(MoveHoriz moveHoriz) const;

  void setRotateForward(RotateForward rotateForward) const;

  void rotateCamera(int mouseDeltaX, int mouseDeltaY) const;

  void scrollCamera(int wheelY) const;

  void drawFrame(int viewportWidth, int viewportHeight, float deltaSeconds) const;

  void init() const;

private:
  struct Impl;

  std::unique_ptr<Impl> impl_;
};
