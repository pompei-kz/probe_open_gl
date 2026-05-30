module;

#include <chrono>
#include <cmath>
#include <glm/glm.hpp>
#include <memory>
#include <numbers>
#include <random>
#include <vector>

module world_02;

import scene;
import world;
import atom;

namespace
{
  constexpr float PI = std::numbers::pi_v<float>;
}

struct world::World_02::Impl
{
  using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

  const Timestamp         startedAt_ = std::chrono::steady_clock::now();

  void initShapes(const scene::ShapeGroup &atoms, const std::vector<scene::MaterialParams> &materials);

  void writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup) const;
};

world::World_02::World_02()
    : impl_(std::make_unique<Impl>())
{
}

world::World_02::~World_02() = default;

void world::World_02::initShapes(scene::ShapeGroup &atoms, const std::vector<scene::MaterialParams> &materials)
{
  impl_->initShapes(atoms, materials);
}

void world::World_02::writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup)
{
  impl_->writeToShapesBeforeRender(shapeGroup);
}

void world::World_02::Impl::initShapes(const scene::ShapeGroup &atoms, const std::vector<scene::MaterialParams> &materials)
{
}

void world::World_02::Impl::writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup) const
{
}
