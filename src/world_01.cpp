module;

#include <chrono>
#include <cmath>
#include <glm/glm.hpp>
#include <memory>
#include <numbers>
#include <random>
#include <vector>

module world_01;

import scene;
import world;
import atom;

namespace
{
  constexpr float PI = std::numbers::pi_v<float>;
}

struct world::World_01::Impl
{
  using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

  const Timestamp         startedAt_ = std::chrono::steady_clock::now();
  std::vector<glm::vec3>  startPositions_;
  std::vector<glm::vec3>  positions_;
  std::vector<glm::vec3>  moveX_;
  std::vector<glm::vec3>  moveY_;
  std::vector<float>      angle_;
  std::vector<float>      velocity_;
  std::vector<atom::Atom> atoms_;
  const float             R = 0.5F;

  void initShapes(const scene::ShapeGroup &atoms, const std::vector<scene::MaterialParams> &materials);

  void writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup);

  void calculateIdle();
};

world::World_01::World_01()
    : impl_(std::make_unique<Impl>())
{
}

world::World_01::~World_01() = default;

void world::World_01::initShapes(scene::ShapeGroup &atoms, const std::vector<scene::MaterialParams> &materials)
{
  impl_->initShapes(atoms, materials);
}

void world::World_01::writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup)
{
  impl_->writeToShapesBeforeRender(shapeGroup);
}

void world::World_01::Impl::initShapes(const scene::ShapeGroup &atoms, const std::vector<scene::MaterialParams> &materials)
{
  std::random_device             rd;
  std::mt19937                   gen(rd());
  std::uniform_real_distribution dist(0.0F, 1.0F);

  const std::size_t atomsCount = atoms.shapes.size();

  startPositions_.resize(atomsCount);
  positions_.resize(atomsCount);
  moveX_.resize(atomsCount);
  moveY_.resize(atomsCount);
  angle_.resize(atomsCount);
  atoms_.resize(atomsCount);
  velocity_.resize(atomsCount);

  for (std::size_t i = 0; const scene::Shape &shape : atoms.shapes)
  {
    const scene::MaterialParams material = materials[shape.materialIndex];

    atoms_[i]     = material.atom;
    startPositions_[i] = shape.offset;
    positions_[i] = shape.offset;

    glm::vec3 moveX = normalize(glm::vec3(dist(gen) - 0.5F, dist(gen) - 0.5F, dist(gen) - 0.5F));
    glm::vec3 moveY = normalize(glm::vec3(dist(gen) - 0.5F, dist(gen) - 0.5F, dist(gen) - 0.5F));

    {
      glm::vec3 normal = glm::cross(moveX, moveY);
      moveY            = glm::cross(normal, moveX);
    }

    moveX_[i] = moveX;
    moveY_[i] = moveY;
    angle_[i] = dist(gen) * 2 * PI - PI;

    velocity_[i] = 1.0F + dist(gen) * 40;

    ++i;
  }
}

void world::World_01::Impl::writeToShapesBeforeRender(scene::ShapeGroup &shapeGroup)
{
  calculateIdle();

  for (std::size_t i = 0; scene::Shape &shape : shapeGroup.shapes)
  {
    shape.offset = positions_[i];
    ++i;
  }
}
void world::World_01::Impl::calculateIdle()
{
  const float timeSec = std::chrono::duration<float>(std::chrono::steady_clock::now() - startedAt_).count();

  for (std::size_t i = 0; auto &pos : startPositions_)
  {
    const float velocity = velocity_[i];
    const float angle    = angle_[i] + timeSec * velocity;
    const float s        = std::sin(angle);
    const float c        = std::cos(angle);

    glm::vec3 moveX = moveX_[i];
    glm::vec3 moveY = moveY_[i];

    positions_[i] = pos + moveX * c * R + moveY * s * R;

    ++i;
  }
}
