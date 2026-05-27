module;

#include <epoxy/gl.h>
#include <yaml-cpp/yaml.h>

#include <array>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module scene;

export namespace scene
{
  struct ShapeInstanceGroup
  {
    std::array<float, 3> offset{0.0F, 0.0F, 0.0F};
    std::uint32_t shapeIndex = 0;
  };

  struct Shape
  {
    std::vector<float> vertices;
    std::vector<GLuint> indexes;
    std::size_t firstInstance = 0;
    std::size_t instanceCount = 0;
  };

  struct Camera
  {
    std::array<float, 3> position{0.0F, 0.0F, 1.0F};
    std::array<float, 3> forward{0.0F, 0.0F, -1.0F};
    std::array<float, 3> up{0.0F, 1.0F, 0.0F};
    float nearPlane               = 0.1F;
    float farPlane                = 100.0F;
    float fovDegrees              = 45.0F;
    float forwardVelocity         = 0.0F;
    float sideVelocity            = 0.0F;
    float forwardMouseSensitivity = 0.0F;
    float forwardScrollStep       = 0.0F;
  };

  class Scene
  {
  public:
    std::vector<Shape> shapes;
    std::vector<ShapeInstanceGroup> instances;
    Camera camera;
    int vertexFloatCount   = 0;
    int positionFloatCount = 0;
    int colorFloatCount    = 0;

    void load(const std::filesystem::path &path, std::string_view figureName);

    void load(const std::filesystem::path &path);
  };

} // namespace scene

namespace
{
  struct DataSection
  {
    std::string type;
    std::vector<std::string> lines;
  };

  struct PointLayout
  {
    int vertexFloatCount   = 0;
    int positionFloatCount = 0;
    int colorFloatCount    = 0;
  };

  struct Material
  {
    std::optional<std::array<float, 3>> solidColor;
  };

  struct MeshRef
  {
    std::filesystem::path path;
    std::string id;
  };

  constexpr std::array zeroOffset{0.0F, 0.0F, 0.0F};

  std::vector<float> parseFloatLine(std::string line);

  std::array<float, 3> parseVector3(const YAML::Node &node, const std::filesystem::path &path, const std::string_view name)
  {
    if (!node || !node.IsScalar())
    {
      throw std::runtime_error("U3GddI64FP :: Missing YAML scalar '" + std::string(name) + "' in " + path.string());
    }

    const std::vector<float> values = parseFloatLine(node.as<std::string>());
    if (values.size() != 3U)
    {
      throw std::runtime_error("zAGgTfSy2e :: YAML scalar '" + std::string(name) + "' must contain 3 floats in " + path.string());
    }
    return {values[0], values[1], values[2]};
  }

  float parseFloatScalar(const YAML::Node &node, const std::filesystem::path &path, const std::string_view name)
  {
    if (!node || !node.IsScalar())
    {
      throw std::runtime_error("OtQRq1JqtC :: Missing YAML scalar '" + std::string(name) + "' in " + path.string());
    }

    const std::vector<float> values = parseFloatLine(node.as<std::string>());
    if (values.size() != 1U)
    {
      throw std::runtime_error("MkrD0Lx4pS :: YAML scalar '" + std::string(name) + "' must contain 1 float in " + path.string());
    }
    return values[0];
  }

  std::string trim(const std::string_view value)
  {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos)
    {
      return {};
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(begin, end - begin + 1));
  }

  std::string stripComment(std::string value)
  {
    if (const auto commentPosition = value.find('#'); commentPosition != std::string::npos)
    {
      value.erase(commentPosition);
    }
    return value;
  }

  std::vector<std::string> splitWords(const std::string_view value)
  {
    std::istringstream input{std::string(value)};
    std::vector<std::string> words;
    std::string word;
    while (input >> word)
    {
      words.push_back(word);
    }
    return words;
  }

  YAML::Node requiredMapChild(const YAML::Node &node, const std::string_view childName, const std::filesystem::path &path)
  {
    const YAML::Node child = node[std::string(childName)];
    if (!child || !child.IsMap())
    {
      throw std::runtime_error("Afz2db44CN :: Missing YAML map '" + std::string(childName) + "' in " + path.string());
    }
    return child;
  }

  std::vector<std::string> readLines(std::istream &input)
  {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line))
    {
      lines.push_back(line);
    }
    return lines;
  }

  std::vector<std::string> readDataRefLines(const YAML::Node &dataRef, const std::filesystem::path &sectionPath,
                                            const std::string_view sectionName)
  {
    if (!dataRef.IsScalar())
    {
      throw std::runtime_error("pW2zTu5Lwd :: YAML '" + std::string(sectionName) + ".data-ref' must be scalar in " +
                               sectionPath.string());
    }

    const std::filesystem::path dataPath = (sectionPath.parent_path() / dataRef.as<std::string>()).lexically_normal();
    std::ifstream input(dataPath);
    if (!input)
    {
      throw std::runtime_error("KIu7rNQxT0 :: Failed to open YAML '" + std::string(sectionName) + ".data-ref' file " +
                               dataPath.string());
    }
    return readLines(input);
  }

  DataSection extractDataSection(const YAML::Node &mesh, const std::string_view sectionName, const std::filesystem::path &path)
  {
    const YAML::Node yamlSection = requiredMapChild(mesh, sectionName, path);
    const YAML::Node type        = yamlSection["type"];
    if (!type || !type.IsScalar())
    {
      throw std::runtime_error("WQ6V9pIWLa :: Missing YAML scalar '" + std::string(sectionName) + ".type' in " + path.string());
    }

    const YAML::Node data    = yamlSection["data"];
    const YAML::Node dataRef = yamlSection["data-ref"];
    if (data && dataRef)
    {
      throw std::runtime_error("AX1zWXw0cG :: YAML section '" + std::string(sectionName) +
                               "' must not define both data and data-ref in " + path.string());
    }
    if (!data && !dataRef)
    {
      throw std::runtime_error("Z3H1qvTv5H :: YAML section '" + std::string(sectionName) + "' must define data or data-ref in " +
                               path.string());
    }

    DataSection section;
    section.type = type.as<std::string>();

    if (dataRef)
    {
      section.lines = readDataRefLines(dataRef, path, sectionName);
      return section;
    }

    if (!data.IsScalar())
    {
      throw std::runtime_error("Om2FLJyLYx :: YAML '" + std::string(sectionName) + ".data' must be scalar in " + path.string());
    }

    std::istringstream input(data.as<std::string>());
    section.lines = readLines(input);

    return section;
  }

  std::vector<float> parseFloatLine(std::string line)
  {
    line = stripComment(std::move(line));
    for (char &symbol : line)
    {
      if (symbol == ',')
      {
        symbol = ' ';
      }
    }

    std::istringstream input(line);
    std::vector<float> values;
    float value = 0.0F;
    while (input >> value)
    {
      values.push_back(value);
    }

    return values;
  }

  PointLayout parsePointLayout(const std::string &type, const std::filesystem::path &path)
  {
    const std::vector<std::string> fields = splitWords(type);
    if (fields.empty() || fields.front() != "i")
    {
      throw std::runtime_error("oGl0vjKJbD :: Invalid points type in " + path.string() + ": " + type);
    }

    PointLayout layout;

    if (fields == std::vector<std::string>{"i", "X", "Y", "Z", "CX", "CY", "CZ"})
    {
      layout.vertexFloatCount   = 6;
      layout.positionFloatCount = 3;
      layout.colorFloatCount    = 3;
      return layout;
    }

    if (fields == std::vector<std::string>{"i", "X", "Y", "Z"})
    {
      layout.vertexFloatCount   = 3;
      layout.positionFloatCount = 3;
      layout.colorFloatCount    = 0;
      return layout;
    }

    throw std::runtime_error("KjYkVw6sAv :: Points type must be 'i X Y Z CX CY CZ' or 'i X Y Z' in " + path.string() + ": " + type);
  }

  int parseIndexCount(const std::string &type, const std::filesystem::path &path)
  {
    const std::vector<std::string> fields = splitWords(type);
    if (fields.empty())
    {
      throw std::runtime_error("gMEOd5Wvmn :: Missing indexes type in " + path.string());
    }
    for (const std::string &field : fields)
    {
      if (field != "i")
      {
        throw std::runtime_error("LVw0x4xKiG :: Invalid indexes type in " + path.string() + ": " + type);
      }
    }
    return static_cast<int>(fields.size());
  }

  std::array<float, 3> parseSolidColor(const YAML::Node &material, const std::filesystem::path &path,
                                       const std::string_view figureName)
  {
    const YAML::Node color = material["color"];
    if (!color || !color.IsScalar())
    {
      throw std::runtime_error("qP8hh93VfV :: Solid material for figure '" + std::string(figureName) +
                               "' must define scalar color in " + path.string());
    }

    const std::vector<float> values = parseFloatLine(color.as<std::string>());
    if (values.size() != 3U)
    {
      throw std::runtime_error("ZtG0L06ctZ :: Solid material color must contain 3 floats in " + path.string());
    }
    return {values[0], values[1], values[2]};
  }

  Material parseMaterial(const YAML::Node &figure, const std::filesystem::path &path, const std::string_view figureName)
  {
    const YAML::Node material = requiredMapChild(figure, "material", path);
    const YAML::Node type     = material["type"];
    if (!type || !type.IsScalar())
    {
      throw std::runtime_error("xLVuVqM5tJ :: Missing material.type for figure '" + std::string(figureName) + "' in " + path.string());
    }

    if (const std::string materialType = type.as<std::string>(); materialType != "solid")
    {
      throw std::runtime_error("UAJtwMZ764 :: Unsupported material type '" + materialType + "' in " + path.string());
    }

    Material result;
    if (material["color"])
    {
      result.solidColor = parseSolidColor(material, path, figureName);
    }
    return result;
  }

  MeshRef parseRefValue(const std::string &value, const std::filesystem::path &path)
  {
    const auto hashPosition = value.find('#');
    if (hashPosition == std::string::npos || hashPosition + 1 >= value.size())
    {
      throw std::runtime_error("lJebVCn9ky :: Invalid ref '" + value + "' in " + path.string());
    }

    MeshRef result;
    result.id                      = value.substr(hashPosition + 1);
    const std::string relativePath = value.substr(0, hashPosition);
    result.path                    = relativePath.empty() ? path : (path.parent_path() / relativePath).lexically_normal();
    return result;
  }

  MeshRef parseMeshRef(const YAML::Node &figure, const std::filesystem::path &figurePath, const std::string_view figureName)
  {
    const YAML::Node mesh = requiredMapChild(figure, "mesh", figurePath);
    const YAML::Node ref  = mesh["ref"];
    if (!ref || !ref.IsScalar())
    {
      throw std::runtime_error("AoiWfWDZDz :: Missing mesh.ref for figure '" + std::string(figureName) + "' in " +
                               figurePath.string());
    }

    return parseRefValue(ref.as<std::string>(), figurePath);
  }

  MeshRef parseFigureRef(const YAML::Node &instanceGroup, const std::filesystem::path &groupPath, const std::string_view groupName)
  {
    const YAML::Node figure = requiredMapChild(instanceGroup, "figure", groupPath);
    const YAML::Node ref    = figure["ref"];
    if (!ref || !ref.IsScalar())
    {
      throw std::runtime_error("wt3wM8XF8Y :: Missing figure.ref for figure instance group '" + std::string(groupName) + "' in " +
                               groupPath.string());
    }

    return parseRefValue(ref.as<std::string>(), groupPath);
  }

  std::vector<std::array<float, 3>> parseOffsets(const YAML::Node &instanceGroup, const std::filesystem::path &groupPath,
                                                 const std::string_view groupName)
  {
    const DataSection offsets = extractDataSection(instanceGroup, "offsets", groupPath);
    if (const std::vector<std::string> fields = splitWords(offsets.type); fields != std::vector<std::string>{"i", "X", "Y", "Z"})
    {
      throw std::runtime_error("vAdL3uEtH2 :: Offsets type must be 'i X Y Z' for figure instance group '" + std::string(groupName) +
                               "' in " + groupPath.string());
    }

    std::vector<std::array<float, 3>> result;
    for (const std::string &line : offsets.lines)
    {
      const std::vector<float> values = parseFloatLine(line);
      if (values.empty())
      {
        continue;
      }
      if (values.size() != 4U)
      {
        throw std::runtime_error("Pvh3WQ8zGl :: Invalid offset row for figure instance group '" + std::string(groupName) + "' in " +
                                 groupPath.string() + ": " + trim(line));
      }
      result.push_back({values[1], values[2], values[3]});
    }
    return result;
  }

  scene::Shape parseMesh(const YAML::Node &mesh, const Material &material, const std::filesystem::path &meshPath,
                         const scene::Scene &sceneData)
  {
    const DataSection points  = extractDataSection(mesh, "points", meshPath);
    const DataSection indexes = extractDataSection(mesh, "indexes", meshPath);
    const PointLayout layout  = parsePointLayout(points.type, meshPath);
    const int indexCount      = parseIndexCount(indexes.type, meshPath);

    if (indexCount != 3)
    {
      throw std::runtime_error("cDO6R2F96Y :: Index type must be 'i i i' for GL_TRIANGLES in " + meshPath.string() + ": " +
                               indexes.type);
    }
    if (layout.colorFloatCount == 0 && !material.solidColor)
    {
      throw std::runtime_error("S9A8rL1zLy :: Mesh without point colors needs solid material color in " + meshPath.string());
    }

    scene::Shape result;
    std::unordered_map<int, GLuint> pointIndexById;
    for (const std::string &line : points.lines)
    {
      const std::vector<float> values = parseFloatLine(line);
      if (values.empty())
      {
        continue;
      }
      if (values.size() != 1 + static_cast<std::size_t>(layout.vertexFloatCount))
      {
        throw std::runtime_error("pSo3Hqn3wN :: Invalid point row in " + meshPath.string() + ": " + trim(line));
      }

      const int id = static_cast<int>(values[0]);
      pointIndexById.emplace(id, static_cast<GLuint>(result.vertices.size() / sceneData.vertexFloatCount));
      for (int i = 0; i < layout.positionFloatCount; ++i)
      {
        result.vertices.push_back(values[1 + static_cast<std::size_t>(i)]);
      }
      if (layout.colorFloatCount > 0)
      {
        result.vertices.insert(result.vertices.end(), values.begin() + 1 + layout.positionFloatCount, values.end());
      }
      else
      {
        result.vertices.insert(result.vertices.end(), material.solidColor->begin(), material.solidColor->end());
      }
    }

    for (const std::string &line : indexes.lines)
    {
      const std::vector<float> values = parseFloatLine(line);
      if (values.empty())
      {
        continue;
      }
      if (values.size() != static_cast<std::size_t>(indexCount))
      {
        throw std::runtime_error("w3jRjFEh8q :: Invalid index row in " + meshPath.string() + ": " + trim(line));
      }

      std::array<GLuint, 3> triangle{};
      bool validTriangle = true;
      for (std::size_t i = 0; i < values.size(); ++i)
      {
        const int pointId     = static_cast<int>(values[i]);
        const auto pointIndex = pointIndexById.find(pointId);
        if (pointIndex == pointIndexById.end())
        {
          std::cerr << "K5fcGkrP2g :: Skipping triangle with missing point id " << pointId << " in " << meshPath << '\n';
          validTriangle = false;
          break;
        }
        triangle[i] = pointIndex->second;
      }

      if (validTriangle)
      {
        result.indexes.insert(result.indexes.end(), triangle.begin(), triangle.end());
      }
    }
    return result;
  }

  std::string shapeKey(const std::filesystem::path &path, const std::string_view figureName)
  {
    return path.lexically_normal().string() + "#" + std::string(figureName);
  }

  std::uint32_t ensureFigureShape(const std::filesystem::path &path, const YAML::Node &figure, const std::string_view figureName,
                                  scene::Scene &result, std::unordered_map<std::string, std::uint32_t> &shapeIndexByKey)
  {
    const std::string key = shapeKey(path, figureName);
    if (const auto shapeIndex = shapeIndexByKey.find(key); shapeIndex != shapeIndexByKey.end())
    {
      return shapeIndex->second;
    }

    const MeshRef meshRef         = parseMeshRef(figure, path, figureName);
    const Material material       = parseMaterial(figure, path, figureName);
    const YAML::Node meshDocument = YAML::LoadFile(meshRef.path.string());
    const YAML::Node meshes       = requiredMapChild(meshDocument, "meshes", meshRef.path);
    const YAML::Node mesh         = requiredMapChild(meshes, meshRef.id, meshRef.path);
    scene::Shape shape            = parseMesh(mesh, material, meshRef.path, result);
    if (shape.vertices.empty() || shape.indexes.empty())
    {
      throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + meshRef.path.string());
    }
    result.shapes.push_back(std::move(shape));
    const auto shapeIndex = static_cast<std::uint32_t>(result.shapes.size() - 1U);
    shapeIndexByKey.emplace(key, shapeIndex);
    return shapeIndex;
  }

  void appendFigure(const std::filesystem::path &path, const YAML::Node &figure, const std::string_view figureName,
                    scene::Scene &result, std::unordered_map<std::string, std::uint32_t> &shapeIndexByKey)
  {
    const std::uint32_t shapeIndex = ensureFigureShape(path, figure, figureName, result, shapeIndexByKey);
    result.instances.push_back(scene::ShapeInstanceGroup{.offset = zeroOffset, .shapeIndex = shapeIndex});
  }

  void appendFigureInstanceGroup(const std::filesystem::path &path, const YAML::Node &instanceGroup, const std::string_view groupName,
                                 scene::Scene &result, std::unordered_map<std::string, std::uint32_t> &shapeIndexByKey)
  {
    const MeshRef figureRef         = parseFigureRef(instanceGroup, path, groupName);
    const YAML::Node figureDocument = YAML::LoadFile(figureRef.path.string());
    const YAML::Node figures        = requiredMapChild(figureDocument, "figures", figureRef.path);
    const YAML::Node figure         = requiredMapChild(figures, figureRef.id, figureRef.path);
    const std::uint32_t shapeIndex  = ensureFigureShape(figureRef.path, figure, figureRef.id, result, shapeIndexByKey);

    for (const std::array<float, 3> &offset : parseOffsets(instanceGroup, path, groupName))
    {
      result.instances.push_back(scene::ShapeInstanceGroup{.offset = offset, .shapeIndex = shapeIndex});
    }
  }

  void updateShapeInstanceRanges(scene::Scene &data)
  {
    std::ranges::sort(data.instances, {}, &scene::ShapeInstanceGroup::shapeIndex);
    for (scene::Shape &shape : data.shapes)
    {
      shape.firstInstance = 0;
      shape.instanceCount = 0;
    }
    for (std::size_t i = 0; i < data.instances.size(); ++i)
    {
      scene::Shape &shape = data.shapes[data.instances[i].shapeIndex];
      if (shape.instanceCount == 0)
      {
        shape.firstInstance = i;
      }
      ++shape.instanceCount;
    }
  }

  void setDefaultLayout(scene::Scene &data)
  {
    data.vertexFloatCount   = 6;
    data.positionFloatCount = 3;
    data.colorFloatCount    = 3;
  }

  void parseSceneCamera(const YAML::Node &document, const YAML::Node &scene, const std::filesystem::path &path, scene::Scene &result)
  {
    const YAML::Node cameraName = scene["camera"];
    if (!cameraName || !cameraName.IsScalar())
    {
      throw std::runtime_error("GM21xOpQV4 :: Missing YAML scalar 'scene.camera' in " + path.string());
    }

    const YAML::Node cameras      = requiredMapChild(document, "cameras", path);
    const YAML::Node camera       = requiredMapChild(cameras, cameraName.as<std::string>(), path);
    const YAML::Node geom         = requiredMapChild(camera, "geom", path);
    result.camera.position        = parseVector3(geom["position"], path, "camera.geom.position");
    result.camera.forward         = parseVector3(geom["forward"], path, "camera.geom.forward");
    result.camera.up              = parseVector3(geom["up"], path, "camera.geom.up");
    result.camera.nearPlane       = parseFloatScalar(geom["near"], path, "camera.geom.near");
    result.camera.farPlane        = parseFloatScalar(geom["far"], path, "camera.geom.far");
    result.camera.fovDegrees      = parseFloatScalar(geom["fov"], path, "camera.geom.fov");
    const YAML::Node params       = requiredMapChild(camera, "params", path);
    result.camera.forwardVelocity = parseFloatScalar(params["forwardVelocity"], path, "camera.params.forwardVelocity");
    result.camera.sideVelocity    = parseFloatScalar(params["sideVelocity"], path, "camera.params.sideVelocity");
    result.camera.forwardMouseSensitivity =
        parseFloatScalar(params["forwardMouseSensitivity"], path, "camera.params.forwardMouseSensitivity");
    result.camera.forwardScrollStep = parseFloatScalar(params["forwardScrollStep"], path, "camera.params.forwardScrollStep");
  }
} // namespace

void scene::Scene::load(const std::filesystem::path &path, const std::string_view figureName)
{
  const YAML::Node document = YAML::LoadFile(path.string());
  shapes.clear();
  instances.clear();
  camera = Camera{};
  setDefaultLayout(*this);
  if (figureName.empty())
  {
    throw std::runtime_error("d5J2Mmx9Ar :: Figure name must not be empty");
  }
  const YAML::Node figures = requiredMapChild(document, "figures", path);
  const YAML::Node figure  = requiredMapChild(figures, figureName, path);
  std::unordered_map<std::string, std::uint32_t> shapeIndexByKey;
  appendFigure(path, figure, figureName, *this, shapeIndexByKey);
  updateShapeInstanceRanges(*this);

  if (shapes.empty() || instances.empty())
  {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }
}

void scene::Scene::load(const std::filesystem::path &path)
{
  const YAML::Node document  = YAML::LoadFile(path.string());
  const YAML::Node sceneNode = requiredMapChild(document, "scene", path);
  shapes.clear();
  instances.clear();
  camera = Camera{};
  setDefaultLayout(*this);
  parseSceneCamera(document, sceneNode, path, *this);

  const YAML::Node sceneFigureInstanceGroups = sceneNode["figure-instance-groups"];
  if (!sceneFigureInstanceGroups || !sceneFigureInstanceGroups.IsSequence())
  {
    throw std::runtime_error("EJEw5s6sPl :: Missing YAML sequence 'scene.figure-instance-groups' in " + path.string());
  }

  const YAML::Node figureInstanceGroups = requiredMapChild(document, "figure-instance-groups", path);
  std::unordered_map<std::string, std::uint32_t> shapeIndexByKey;
  for (const YAML::Node groupName : sceneFigureInstanceGroups)
  {
    if (!groupName.IsScalar())
    {
      throw std::runtime_error("eLfoLjxgQh :: YAML 'scene.figure-instance-groups' values must be scalar in " + path.string());
    }
    const std::string name         = groupName.as<std::string>();
    const YAML::Node instanceGroup = requiredMapChild(figureInstanceGroups, name, path);
    appendFigureInstanceGroup(path, instanceGroup, name, *this, shapeIndexByKey);
  }
  updateShapeInstanceRanges(*this);

  if (shapes.empty() || instances.empty())
  {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }
}
