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

namespace
{
  constexpr float PI = std::numbers::pi_v<float>;
}

struct world::World_01::Impl
{
  using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

  const Timestamp        startedAt_ = std::chrono::steady_clock::now();
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> moveX_;
  std::vector<glm::vec3> moveY_;
  std::vector<float>     angle_;
  const float            R = 0.5F;

  void initShapes(const std::vector<scene::Shape> &shapes)
  {
    std::random_device             rd;
    std::mt19937                   gen(rd());
    std::uniform_real_distribution dist(0.0F, 1.0F);

    positions_.resize(shapes.size());
    moveX_.resize(shapes.size());
    moveY_.resize(shapes.size());
    angle_.resize(shapes.size());

    for (std::size_t i = 0; const scene::Shape &shape : shapes)
    {
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

      ++i;
    }
  }

  void writeToShapesBeforeRender(std::vector<scene::Shape> &shapes) const
  {
    const float timeSec = std::chrono::duration<float>(std::chrono::steady_clock::now() - startedAt_).count();

    for (std::size_t i = 0; scene::Shape &shape : shapes)
    {
      const float angle = angle_[i] + timeSec * 3.0F;
      const float s     = std::sin(angle);
      const float c     = std::cos(angle);

      glm::vec3 pos   = positions_[i];
      glm::vec3 moveX = moveX_[i];
      glm::vec3 moveY = moveY_[i];

      const glm::vec3 result = pos + moveX * c * R + moveY * s * R;

      shape.offset = result;

      ++i;
    }
  }
};

world::World_01::World_01()
    : impl_(std::make_unique<Impl>())
{
}

world::World_01::~World_01() = default;

void world::World_01::initShapes(std::vector<scene::Shape> &shapes)
{
  impl_->initShapes(shapes);
}

void world::World_01::writeToShapesBeforeRender(std::vector<scene::Shape> &shapes)
{
  impl_->writeToShapesBeforeRender(shapes);
}
