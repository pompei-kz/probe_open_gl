module;

#include <epoxy/gl.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <vector>

export module scene;

import atom;

export namespace scene
{
  struct Shape
  {
    glm::vec3     offset{0.0F, 0.0F, 0.0F};
    std::uint32_t materialIndex = 0;
    std::uint32_t meshIndex     = 0;

    static constexpr size_t OffsetOffset        = 0U;
    static constexpr size_t MaterialIndexOffset = 3U * sizeof(float);
    static constexpr size_t Stride              = 4U * sizeof(float);
  };

  struct MaterialParams
  {
    glm::vec3  color{1.0F, 1.0F, 1.0F};
    float      scale = 1.0F;
    atom::Atom atom;
  };

  struct Mesh
  {
    std::vector<float>       vertices;
    static constexpr size_t  VertexPosFloatCount = 3;
    static constexpr size_t  VertexFloatCount    = VertexPosFloatCount;
    static constexpr GLsizei VertexStride        = VertexFloatCount * sizeof(float);
    static constexpr size_t  VertexPosOffset     = 0 * sizeof(float);

    std::vector<GLuint> indexes;

    GLsizeiptr indexesSizeBytes() const { return static_cast<GLsizeiptr>(indexes.size() * sizeof(GLuint)); }

    GLsizeiptr verticesSizeBytes() const { return static_cast<GLsizeiptr>(vertices.size() * sizeof(float)); }
  };

  struct ShapeGroup
  {
    std::string   shaderName = "triangle";
    std::uint32_t meshIndex  = 0;

    // Индекс первого инстанса этой группы в общем массиве инстансов сцены.
    std::size_t firstInstance = 0;

    // Количество подряд идущих инстансов этой группы в общем массиве инстансов сцены.
    std::size_t instanceCount = 0;

    size_t firstInstanceOffset() const { return firstInstance * static_cast<std::size_t>(Shape::Stride); }
  };

  struct Camera
  {
    glm::vec3 position{0.0F, 0.0F, 1.0F};
    glm::vec3 forward{0.0F, 0.0F, -1.0F};
    glm::vec3 up{0.0F, 1.0F, 0.0F};
    float     nearPlane               = 0.1F;
    float     farPlane                = 100.0F;
    float     fovDegrees              = 45.0F;
    float     forwardVelocity         = 0.0F;
    float     sideVelocity            = 0.0F;
    float     forwardMouseSensitivity = 0.0F;
    float     forwardScrollStep       = 0.0F;
    float     forwardRotateDegPSec    = 0.0F;
  };

  struct Sun
  {
    float     force     = 1.0F;
    glm::vec3 direction = {0.0F, 0.0F, -1.0F};
    glm::vec3 color     = {1.0F, 1.0F, 1.0F};
  };

  struct SceneParams
  {
    glm::vec3 backgroundColor{0.08F, 0.10F, 0.14F};
  };

  class Scene
  {
  public:
    std::vector<Mesh>           meshes;
    std::vector<Shape>          shapes;
    std::vector<ShapeGroup>     shapeGroups;
    std::vector<MaterialParams> materials;
    Camera                      camera;
    Sun                         sun;
    SceneParams                 params;

    void load(const std::filesystem::path &path);

    GLsizeiptr shapesSizeBytes() const { return static_cast<GLsizeiptr>(shapes.size() * Shape::Stride); }

    GLsizeiptr materialsSizeBytes() const { return static_cast<GLsizeiptr>(materials.size() * sizeof(MaterialParams)); }
  };

} // namespace scene
