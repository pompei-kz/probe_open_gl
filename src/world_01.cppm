module;

#include <iostream>
#include <ostream>
#include <vector>

export module world_01;

import world;
import scene;

export namespace world
{
  class World_01 : public World
  {
  public:
    ~World_01() override;

    void initShapes(std::vector<scene::Shape> &shapes) override;

    void writeToShapesBeforeRender(std::vector<scene::Shape> &shapes) override;

  private:
    float sum_ = 0.0F;
  };

  World_01::~World_01() {}

  void World_01::initShapes(std::vector<scene::Shape> &shapes)
  {
    std::cout << "sJjI3FaEOk :: World_01::initShapes" << std::endl;
  }

  void World_01::writeToShapesBeforeRender(std::vector<scene::Shape> &shapes)
  {
    float sum = 0;

    for (auto shape : shapes)
    {
      const float x = shape.offset[0];
      const float y = shape.offset[1];
      const float z = shape.offset[2];

      sum += x;
      sum += y;
      sum += z;
    }

    sum_ = sum;
  }
} // namespace world
