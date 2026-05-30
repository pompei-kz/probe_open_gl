module;

#include <memory>
#include <vector>

export module world_02;

import world;
import scene;

export namespace world
{
  class World_02 : public World
  {
  public:
    World_02();

    ~World_02() override;

    void initShapes(scene::ShapeGroup &shapeGroup, const std::vector<scene::MaterialParams> &materials) override;

    void writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup) override;

  private:
    struct Impl;

    std::unique_ptr<Impl> impl_;
  };
} // namespace world
