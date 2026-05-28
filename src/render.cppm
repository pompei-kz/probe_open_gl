module;

#include <filesystem>
#include <memory>

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
  explicit Render(const std::filesystem::path &scenePath);

  Render(const Render &) = delete;

  Render &operator=(const Render &) = delete;

  Render(Render &&) = delete;

  Render &operator=(Render &&) = delete;

  ~Render();

  void setMoveVert(MoveVert moveVert);

  void setMoveHoriz(MoveHoriz moveHoriz);

  void setRotateForward(RotateForward rotateForward);

  void rotateCamera(int mouseDeltaX, int mouseDeltaY);

  void scrollCamera(int wheelY);

  void drawFrame(int viewportWidth, int viewportHeight, float deltaSeconds);

private:
  struct Impl;

  std::unique_ptr<Impl> impl_;
};
