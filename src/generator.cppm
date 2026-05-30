module;

#include <cmath>
#include <epoxy/gl.h>
#include <cstdint>
#include <stdexcept>
#include <glm/glm.hpp>

export module generator;
import scene;

namespace
{
  constexpr float Pi    = 3.14159265358979323846F;
  constexpr float TwoPi = 6.28318530717958647692F;

  glm::vec3 normalizedCross(const glm::vec3 &lhs, const glm::vec3 &rhs)
  {
    const glm::vec3 crossProduct = glm::cross(lhs, rhs);
    const float     length       = glm::length(crossProduct);
    if (length <= 0.0F)
    {
      throw std::runtime_error("generator::populateMeshWithCylinder(): cylinder axis is degenerate");
    }
    return crossProduct / length;
  }

  void buildCylinderBasis(const glm::vec3 &axisDirection, glm::vec3 &u, glm::vec3 &v)
  {
    if (std::abs(axisDirection.x) >= std::abs(axisDirection.y) && std::abs(axisDirection.x) >= std::abs(axisDirection.z))
    {
      u = normalizedCross(glm::vec3{0.0F, 0.0F, 1.0F}, axisDirection);
      v = normalizedCross(axisDirection, u);
      return;
    }

    if (std::abs(axisDirection.y) >= std::abs(axisDirection.z))
    {
      u = normalizedCross(axisDirection, glm::vec3{0.0F, 0.0F, 1.0F});
      v = normalizedCross(u, axisDirection);
      return;
    }

    u = normalizedCross(glm::vec3{0.0F, 1.0F, 0.0F}, axisDirection);
    v = normalizedCross(axisDirection, u);
  }

  void appendRing(scene::Mesh &mesh, const glm::vec3 &center, const glm::vec3 &u, const glm::vec3 &v, const float radius, const int loopSegments)
  {
    for (int segment = 0; segment < loopSegments; ++segment)
    {
      const float angle = static_cast<float>(segment) * TwoPi / static_cast<float>(loopSegments);
      const float c     = std::cos(angle);
      const float s     = std::sin(angle);
      const glm::vec3 point = center + radius * (c * u + s * v);
      mesh.vertices.push_back(point.x);
      mesh.vertices.push_back(point.y);
      mesh.vertices.push_back(point.z);
    }
  }

  void appendCap(scene::Mesh &mesh, const std::uint32_t centerIndex, const std::uint32_t ringStartIndex, const int loopSegments, const bool topCap)
  {
    for (int segment = 0; segment < loopSegments; ++segment)
    {
      const std::uint32_t current = ringStartIndex + static_cast<std::uint32_t>(segment);
      const std::uint32_t next    = ringStartIndex + static_cast<std::uint32_t>((segment + 1) % loopSegments);
      if (topCap)
      {
        mesh.indexes.push_back(centerIndex);
        mesh.indexes.push_back(current);
        mesh.indexes.push_back(next);
      }
      else
      {
        mesh.indexes.push_back(centerIndex);
        mesh.indexes.push_back(next);
        mesh.indexes.push_back(current);
      }
    }
  }
} // namespace

export namespace gen
{
  struct GenCylinderParams
  {
    glm::vec3 bottomBaseCenter;
    glm::vec3 topBaseCenter;
    float     radius;
    int       loopSegments;
    int       axisSegments;
  };

  struct GenSpheraParams
  {
    glm::vec3 center;
    glm::vec3 northPoleDirection;
    float     radius;
    int       latitudeSegments;
    int       longitudeSegments;
  };

  void populateMeshWithCylinder(scene::Mesh *mesh, const GenCylinderParams &params)
  {
    if (mesh == nullptr)
    {
      throw std::runtime_error("sF9UKS0LVx :: populateMeshWithCylinder(): mesh is null");
    }
    if (params.loopSegments < 3)
    {
      throw std::runtime_error("Q2WUTpK5en :: populateMeshWithCylinder(): loopSegments must be at least 3");
    }
    if (params.axisSegments < 1)
    {
      throw std::runtime_error("5P7ituNGro :: populateMeshWithCylinder(): axisSegments must be at least 1");
    }
    if (params.radius <= 0.0F)
    {
      throw std::runtime_error("62f4ZgvQzE :: populateMeshWithCylinder(): radius must be positive");
    }

    mesh->vertices.clear();
    mesh->indexes.clear();
    mesh->vertices.reserve(static_cast<std::size_t>(params.axisSegments + 1) * static_cast<std::size_t>(params.loopSegments) * 3U + 6U);
    mesh->indexes.reserve(static_cast<std::size_t>(params.axisSegments) * static_cast<std::size_t>(params.loopSegments) * 6U +
                          static_cast<std::size_t>(params.loopSegments) * 6U);

    const glm::vec3 axisVector = params.topBaseCenter - params.bottomBaseCenter;
    const float     axisLength = glm::length(axisVector);
    if (axisLength <= 0.0F)
    {
      throw std::runtime_error("kRcMr1IA8s :: populateMeshWithCylinder(): cylinder axis length must be positive");
    }

    const glm::vec3 axisDirection = axisVector / axisLength;
    glm::vec3 u;
    glm::vec3 v;
    buildCylinderBasis(axisDirection, u, v);

    for (int axisSegment = 0; axisSegment <= params.axisSegments; ++axisSegment)
    {
      const float t = static_cast<float>(axisSegment) / static_cast<float>(params.axisSegments);
      const glm::vec3 center = params.bottomBaseCenter + axisVector * t;
      appendRing(*mesh, center, u, v, params.radius, params.loopSegments);
    }

    const std::uint32_t bottomCenterIndex = static_cast<std::uint32_t>(mesh->vertices.size() / 3U);
    mesh->vertices.push_back(params.bottomBaseCenter.x);
    mesh->vertices.push_back(params.bottomBaseCenter.y);
    mesh->vertices.push_back(params.bottomBaseCenter.z);

    const std::uint32_t topCenterIndex = static_cast<std::uint32_t>(mesh->vertices.size() / 3U);
    mesh->vertices.push_back(params.topBaseCenter.x);
    mesh->vertices.push_back(params.topBaseCenter.y);
    mesh->vertices.push_back(params.topBaseCenter.z);

    for (int axisSegment = 0; axisSegment < params.axisSegments; ++axisSegment)
    {
      const std::uint32_t currentRingStart = static_cast<std::uint32_t>(axisSegment * params.loopSegments);
      const std::uint32_t nextRingStart    = static_cast<std::uint32_t>((axisSegment + 1) * params.loopSegments);
      for (int segment = 0; segment < params.loopSegments; ++segment)
      {
        const std::uint32_t current = currentRingStart + static_cast<std::uint32_t>(segment);
        const std::uint32_t next    = currentRingStart + static_cast<std::uint32_t>((segment + 1) % params.loopSegments);
        const std::uint32_t upperCurrent = nextRingStart + static_cast<std::uint32_t>(segment);
        const std::uint32_t upperNext    = nextRingStart + static_cast<std::uint32_t>((segment + 1) % params.loopSegments);

        mesh->indexes.push_back(current);
        mesh->indexes.push_back(next);
        mesh->indexes.push_back(upperNext);

        mesh->indexes.push_back(current);
        mesh->indexes.push_back(upperNext);
        mesh->indexes.push_back(upperCurrent);
      }
    }

    appendCap(*mesh, bottomCenterIndex, 0U, params.loopSegments, false);
    appendCap(*mesh, topCenterIndex, static_cast<std::uint32_t>(params.axisSegments * params.loopSegments), params.loopSegments, true);

  }

  void populateMeshWithSphera(scene::Mesh *mesh, const GenSpheraParams &params)
  {
    if (mesh == nullptr)
    {
      throw std::runtime_error("q5C4eGqW8m :: populateMeshWithSphera(): mesh is null");
    }
    if (params.latitudeSegments < 2)
    {
      throw std::runtime_error("x2bY6gQ2fK :: populateMeshWithSphera(): latitudeSegments must be at least 2");
    }
    if (params.longitudeSegments < 3)
    {
      throw std::runtime_error("p8K2rN7zV1 :: populateMeshWithSphera(): longitudeSegments must be at least 3");
    }
    if (params.radius <= 0.0F)
    {
      throw std::runtime_error("t4Y1hM5cQ0 :: populateMeshWithSphera(): radius must be positive");
    }

    mesh->vertices.clear();
    mesh->indexes.clear();
    mesh->vertices.reserve(static_cast<std::size_t>(params.latitudeSegments - 1) * static_cast<std::size_t>(params.longitudeSegments) * 3U + 6U);
    mesh->indexes.reserve(static_cast<std::size_t>(params.latitudeSegments) * static_cast<std::size_t>(params.longitudeSegments) * 6U);

    const float northDirectionLength = glm::length(params.northPoleDirection);
    if (northDirectionLength <= 0.0F)
    {
      throw std::runtime_error("n1R0fG8mQ7 :: populateMeshWithSphera(): northPoleDirection must be non-zero");
    }
    const glm::vec3 northDirection = params.northPoleDirection / northDirectionLength;
    glm::vec3       u;
    glm::vec3       v;
    buildCylinderBasis(northDirection, u, v);

    const std::uint32_t northPoleIndex = 0U;
    mesh->vertices.push_back((params.center + params.radius * northDirection).x);
    mesh->vertices.push_back((params.center + params.radius * northDirection).y);
    mesh->vertices.push_back((params.center + params.radius * northDirection).z);

    for (int latitude = 1; latitude < params.latitudeSegments; ++latitude)
    {
      const float polarAngle = static_cast<float>(latitude) * Pi / static_cast<float>(params.latitudeSegments);
      const float ringRadius = params.radius * std::sin(polarAngle);
      const float ringOffset = params.radius * std::cos(polarAngle);
      const glm::vec3 ringCenter = params.center + ringOffset * northDirection;
      appendRing(*mesh, ringCenter, u, v, ringRadius, params.longitudeSegments);
    }

    const glm::vec3 southPole = params.center - params.radius * northDirection;
    const std::uint32_t southPoleIndex = static_cast<std::uint32_t>(mesh->vertices.size() / 3U);
    mesh->vertices.push_back(southPole.x);
    mesh->vertices.push_back(southPole.y);
    mesh->vertices.push_back(southPole.z);

    const std::uint32_t firstRingStart = 1U;
    const std::uint32_t lastRingStart  = static_cast<std::uint32_t>(1 + (params.latitudeSegments - 2) * params.longitudeSegments);

    appendCap(*mesh, northPoleIndex, firstRingStart, params.longitudeSegments, true);

    for (int latitude = 1; latitude < params.latitudeSegments - 1; ++latitude)
    {
      const std::uint32_t upperRingStart = static_cast<std::uint32_t>(1 + latitude * params.longitudeSegments);
      const std::uint32_t lowerRingStart = static_cast<std::uint32_t>((latitude - 1) * params.longitudeSegments + 1U);
      for (int segment = 0; segment < params.longitudeSegments; ++segment)
      {
        const std::uint32_t upperCurrent = upperRingStart + static_cast<std::uint32_t>(segment);
        const std::uint32_t upperNext    = upperRingStart + static_cast<std::uint32_t>((segment + 1) % params.longitudeSegments);
        const std::uint32_t lowerCurrent = lowerRingStart + static_cast<std::uint32_t>(segment);
        const std::uint32_t lowerNext    = lowerRingStart + static_cast<std::uint32_t>((segment + 1) % params.longitudeSegments);

        mesh->indexes.push_back(lowerCurrent);
        mesh->indexes.push_back(lowerNext);
        mesh->indexes.push_back(upperNext);

        mesh->indexes.push_back(lowerCurrent);
        mesh->indexes.push_back(upperNext);
        mesh->indexes.push_back(upperCurrent);
      }
    }

    appendCap(*mesh, southPoleIndex, lastRingStart, params.longitudeSegments, false);
  }
} // namespace gen
