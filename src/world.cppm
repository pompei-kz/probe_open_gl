module;

#include <vector>

export module world;

import scene;

export namespace world
{
  class World
  {
  public:
    virtual ~World();

    virtual void initShapes(scene::ShapeGroup &shapeGroup, const std::vector<scene::MaterialParams> &materials) = 0;

    virtual void writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup) = 0;
  };

  World::~World() {}
} // namespace world
