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

    virtual void initShapes(std::vector<scene::Shape> &shapes) = 0;

    virtual void writeToShapesBeforeRender(std::vector<scene::Shape> &shapes) = 0;
  };

  World::~World() {}
} // namespace world
