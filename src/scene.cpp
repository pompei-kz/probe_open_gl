module;

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

module scene;

namespace
{
  struct DataSection
  {
    std::string              type;
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
    int                  index = 0;
    std::array<float, 3> solidColor{1.0F, 1.0F, 1.0F};
    float                scale = 1.0F;
  };

  struct OffsetRow
  {
    glm::vec3 offset{0.0F, 0.0F, 0.0F};
    int       materialIndex = 0;
  };

  struct MeshRef
  {
    std::filesystem::path path;
    std::string           id;
  };

  constexpr glm::vec3 zeroOffset{0.0F, 0.0F, 0.0F};

  std::vector<float> parseFloatLine(std::string line);

  glm::vec3 parseVector3(const YAML::Node &node, const std::filesystem::path &path, const std::string_view name)
  {
    if (!node || !node.IsScalar())
    {
      throw std::runtime_error("gkiGXwc6kc :: Missing YAML scalar '" + std::string(name) + "' in " + path.string());
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

  glm::vec3 normalizeVector(const glm::vec3 &value, const std::filesystem::path &path, const std::string_view name)
  {
    if (glm::length(value) <= 0.0F)
    {
      throw std::runtime_error("R3r6lgbSG2 :: YAML vector '" + std::string(name) + "' must not be zero in " + path.string());
    }
    return glm::normalize(value);
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
    std::istringstream       input{std::string(value)};
    std::vector<std::string> words;
    std::string              word;
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

  YAML::Node optionalMapChild(const YAML::Node &node, const std::string_view childName, const std::filesystem::path &path)
  {
    const YAML::Node child = node[std::string(childName)];
    if (!child)
    {
      return YAML::Node(YAML::NodeType::Map);
    }
    if (!child.IsMap())
    {
      throw std::runtime_error("pOqSmJ7kVE :: YAML container '" + std::string(childName) + "' must be map in " + path.string());
    }
    return child;
  }

  YAML::Node loadYamlFile(const std::filesystem::path &path)
  {
    try
    {
      return YAML::LoadFile(path.string());
    }
    catch (const YAML::Exception &exception)
    {
      throw std::runtime_error("iLwL6RWoAE :: Failed to load YAML file " + path.string() + ": " + exception.what());
    }
  }

  std::vector<std::string> readLines(std::istream &input)
  {
    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(input, line))
    {
      lines.push_back(line);
    }
    return lines;
  }

  std::vector<std::string> readDataRefLines(const YAML::Node &dataRef, const std::filesystem::path &sectionPath, const std::string_view sectionName)
  {
    if (!dataRef.IsScalar())
    {
      throw std::runtime_error("pW2zTu5Lwd :: YAML '" + std::string(sectionName) + ".data-ref' must be scalar in " + sectionPath.string());
    }

    const std::filesystem::path dataPath = (sectionPath.parent_path() / dataRef.as<std::string>()).lexically_normal();
    std::ifstream               input(dataPath);
    if (!input)
    {
      throw std::runtime_error("KIu7rNQxT0 :: Failed to open YAML '" + std::string(sectionName) + ".data-ref' file " + dataPath.string());
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
      throw std::runtime_error("AX1zWXw0cG :: YAML section '" + std::string(sectionName) + "' must not define both data and data-ref in " +
                               path.string());
    }
    if (!data && !dataRef)
    {
      throw std::runtime_error("Z3H1qvTv5H :: YAML section '" + std::string(sectionName) + "' must define data or data-ref in " + path.string());
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
    float              value = 0.0F;
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

    if (fields == std::vector<std::string>{"i", "X", "Y", "Z"})
    {
      PointLayout layout;
      layout.vertexFloatCount   = 3;
      layout.positionFloatCount = 3;
      layout.colorFloatCount    = 0;
      return layout;
    }

    throw std::runtime_error("KjYkVw6sAv :: Points type must be 'i X Y Z' in " + path.string() + ": " + type);
  }

  int parseIndexCount(const std::string &type, const std::filesystem::path &path)
  {
    const std::vector<std::string> fields = splitWords(type);
    if (fields.empty())
    {
      throw std::runtime_error("gMEOd5Wv3n :: Missing indexes type in " + path.string());
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

  std::array<float, 3> parseSolidColor(const YAML::Node &material, const std::filesystem::path &path, const std::string_view shapeName)
  {
    const YAML::Node color = material["color"];
    if (!color || !color.IsScalar())
    {
      throw std::runtime_error("qP8hh93VfV :: Solid material for shape '" + std::string(shapeName) + "' must define scalar color in " +
                               path.string());
    }

    const std::vector<float> values = parseFloatLine(color.as<std::string>());
    if (values.size() != 3U)
    {
      throw std::runtime_error("ZtG0L06ctZ :: Solid material color must contain 3 floats in " + path.string());
    }
    return {values[0], values[1], values[2]};
  }

  Material parseMaterial(const YAML::Node &shape, const std::filesystem::path &path, const std::string_view shapeName)
  {
    const YAML::Node material = requiredMapChild(shape, "material", path);
    const YAML::Node type     = material["type"];
    if (!type || !type.IsScalar())
    {
      throw std::runtime_error("xLVuVqM5tJ :: Missing material.type for shape '" + std::string(shapeName) + "' in " + path.string());
    }

    if (const std::string materialType = type.as<std::string>(); materialType != "solid")
    {
      throw std::runtime_error("UAJtwMZ764 :: Unsupported material type '" + materialType + "' in " + path.string());
    }

    Material result;
    if (material["color"])
    {
      result.solidColor = parseSolidColor(material, path, shapeName);
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

  MeshRef parseMeshRef(const YAML::Node &shape, const std::filesystem::path &shapePath, const std::string_view shapeName)
  {
    const YAML::Node mesh = requiredMapChild(shape, "mesh", shapePath);
    const YAML::Node ref  = mesh["ref"];
    if (!ref || !ref.IsScalar())
    {
      throw std::runtime_error("AoiWfWDZDz :: Missing mesh.ref for shape '" + std::string(shapeName) + "' in " + shapePath.string());
    }

    return parseRefValue(ref.as<std::string>(), shapePath);
  }

  MeshRef parseShapeRef(const YAML::Node &instanceGroup, const std::filesystem::path &groupPath, const std::string_view groupName)
  {
    const YAML::Node shape = requiredMapChild(instanceGroup, "shape", groupPath);
    const YAML::Node ref   = shape["ref"];
    if (!ref || !ref.IsScalar())
    {
      throw std::runtime_error("wt3wM8XF8Y :: Missing shape.ref for shape instance group '" + std::string(groupName) + "' in " + groupPath.string());
    }

    return parseRefValue(ref.as<std::string>(), groupPath);
  }

  MeshRef parseGroupMeshRef(const YAML::Node &shapeGroup, const std::filesystem::path &groupPath, const std::string_view groupName)
  {
    const YAML::Node ref = shapeGroup["mesh-ref"];
    if (!ref || !ref.IsScalar())
    {
      throw std::runtime_error("p3EsGrzP71 :: Missing mesh-ref for shape group '" + std::string(groupName) + "' in " + groupPath.string());
    }

    return parseRefValue(ref.as<std::string>(), groupPath);
  }

  std::string parseShaderName(const YAML::Node &shapeGroup, const std::filesystem::path &groupPath, const std::string_view groupName)
  {
    const YAML::Node shader = shapeGroup["shader"];
    if (!shader || !shader.IsScalar())
    {
      throw std::runtime_error("XAl9Vrbz1y :: Missing shader for shape group '" + std::string(groupName) + "' in " + groupPath.string());
    }

    const std::string shaderName = trim(shader.as<std::string>());
    if (shaderName.empty())
    {
      throw std::runtime_error("hdQkB9MnF3 :: Empty shader for shape group '" + std::string(groupName) + "' in " + groupPath.string());
    }
    return shaderName;
  }

  std::unordered_map<int, Material>
  parseShapeGroupMaterials(const YAML::Node &shapeGroup, const std::filesystem::path &path, const std::string_view groupName)
  {
    std::unordered_map<int, Material> result;
    const YAML::Node                  materials = shapeGroup["materials"];
    if (!materials)
    {
      result.emplace(0, Material{});
      return result;
    }
    if (!materials.IsSequence() || materials.size() == 0U)
    {
      throw std::runtime_error("v6uVtDjYNi :: Shape group '" + std::string(groupName) + "' materials must be non-empty sequence in " + path.string());
    }

    for (const YAML::Node materialNode : materials)
    {
      if (!materialNode.IsMap())
      {
        throw std::runtime_error("wYzXRq2T4E :: Shape group '" + std::string(groupName) + "' material must be map in " + path.string());
      }

      const YAML::Node index = materialNode["index"];
      if (!index || !index.IsScalar())
      {
        throw std::runtime_error("wAOCGDwBmv :: Shape group '" + std::string(groupName) + "' material must define scalar index in " + path.string());
      }

      Material material;
      material.index = index.as<int>();
      if (materialNode["color"])
      {
        material.solidColor = parseSolidColor(materialNode, path, groupName);
      }
      if (materialNode["scale"])
      {
        material.scale = parseFloatScalar(materialNode["scale"], path, "shape-groups." + std::string(groupName) + ".materials.scale");
      }
      if (material.scale <= 0.0F)
      {
        throw std::runtime_error("N8VNXRBUAx :: Shape group '" + std::string(groupName) + "' material scale must be positive in " + path.string());
      }
      if (!result.emplace(material.index, material).second)
      {
        throw std::runtime_error("LbLYzv3vdh :: Duplicate material index " + std::to_string(material.index) + " for shape group '" +
                                 std::string(groupName) + "' in " + path.string());
      }
    }

    return result;
  }

  std::vector<OffsetRow> parseOffsets(const YAML::Node &instanceGroup, const std::filesystem::path &groupPath, const std::string_view groupName)
  {
    const DataSection              offsets = extractDataSection(instanceGroup, "offsets", groupPath);
    const std::vector<std::string> fields  = splitWords(offsets.type);
    if (fields != std::vector<std::string>{"i", "X", "Y", "Z"} && fields != std::vector<std::string>{"i", "X", "Y", "Z", "Mi"})
    {
      throw std::runtime_error("vAdL3uEtH2 :: Offsets type must be 'i X Y Z' or 'i X Y Z Mi' for shape group '" + std::string(groupName) + "' in " +
                               groupPath.string());
    }

    const bool             hasMaterialIndex = fields == std::vector<std::string>{"i", "X", "Y", "Z", "Mi"};
    std::vector<OffsetRow> result;
    for (const std::string &line : offsets.lines)
    {
      const std::vector<float> values = parseFloatLine(line);
      if (values.empty())
      {
        continue;
      }
      if (values.size() != fields.size())
      {
        throw std::runtime_error("Pvh3WQ8zGl :: Invalid offset row for shape group '" + std::string(groupName) + "' in " + groupPath.string() + ": " +
                                 trim(line));
      }
      result.push_back(OffsetRow{.offset = {values[1], values[2], values[3]}, .materialIndex = hasMaterialIndex ? static_cast<int>(values[4]) : 0});
    }
    return result;
  }

  scene::Shape parseMesh(const YAML::Node &mesh, const Material &material, const std::filesystem::path &meshPath)
  {
    const DataSection points     = extractDataSection(mesh, "points", meshPath);
    const DataSection indexes    = extractDataSection(mesh, "indexes", meshPath);
    const PointLayout layout     = parsePointLayout(points.type, meshPath);
    const int         indexCount = parseIndexCount(indexes.type, meshPath);

    if (indexCount != 3)
    {
      throw std::runtime_error("cDO6R2F96Y :: Index type must be 'i i i' for GL_TRIANGLES in " + meshPath.string() + ": " + indexes.type);
    }
    scene::Shape                    result;
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
      pointIndexById.emplace(id, static_cast<GLuint>(result.vertices.size() / scene::Shape::VertexFloatCount));
      for (int i = 0; i < layout.positionFloatCount; ++i)
      {
        result.vertices.push_back(values[1 + static_cast<std::size_t>(i)]);
      }
      result.vertices.insert(result.vertices.end(), material.solidColor.begin(), material.solidColor.end());
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
      bool                  validTriangle = true;

      for (std::size_t i = 0; i < values.size(); ++i)
      {
        const int  pointId    = static_cast<int>(values[i]);
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

  std::string shapeKey(const std::filesystem::path &path, const std::string_view shapeName)
  {
    return path.lexically_normal().string() + "#" + std::string(shapeName);
  }

  std::uint32_t ensureShape(const std::filesystem::path                    &path,
                            const YAML::Node                               &shapeNode,
                            const std::string_view                          shapeName,
                            scene::Scene                                   &result,
                            std::unordered_map<std::string, std::uint32_t> &shapeIndexByKey)
  {
    const std::string key = shapeKey(path, shapeName);
    if (const auto shapeIndex = shapeIndexByKey.find(key); shapeIndex != shapeIndexByKey.end())
    {
      return shapeIndex->second;
    }

    const MeshRef    meshRef      = parseMeshRef(shapeNode, path, shapeName);
    const Material   material     = parseMaterial(shapeNode, path, shapeName);
    const YAML::Node meshDocument = loadYamlFile(meshRef.path);
    const YAML::Node meshes       = optionalMapChild(meshDocument, "meshes", meshRef.path);
    const YAML::Node mesh         = requiredMapChild(meshes, meshRef.id, meshRef.path);
    scene::Shape     parsedShape  = parseMesh(mesh, material, meshRef.path);
    if (parsedShape.vertices.empty() || parsedShape.indexes.empty())
    {
      throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + meshRef.path.string());
    }
    result.shapes.push_back(std::move(parsedShape));
    const auto shapeIndex = static_cast<std::uint32_t>(result.shapes.size() - 1U);
    shapeIndexByKey.emplace(key, shapeIndex);
    return shapeIndex;
  }

  void appendShape(const std::filesystem::path                    &path,
                   const YAML::Node                               &shape,
                   const std::string_view                          shapeName,
                   scene::Scene                                   &result,
                   std::unordered_map<std::string, std::uint32_t> &shapeIndexByKey)
  {
    const std::uint32_t shapeIndex = ensureShape(path, shape, shapeName, result, shapeIndexByKey);
    result.instances.push_back(scene::ShapeInstance{.offset = zeroOffset, .shapeIndex = shapeIndex});
    result.shapeGroups.push_back(scene::ShapeGroup{.shaderName = "triangle", .shapeIndex = shapeIndex, .firstInstance = 0, .instanceCount = 1});
  }

  void appendShapeInstanceGroup(const std::filesystem::path                    &path,
                                const YAML::Node                               &instanceGroup,
                                const std::string_view                          groupName,
                                scene::Scene                                   &result,
                                std::unordered_map<std::string, std::uint32_t> &shapeIndexByKey)
  {
    const MeshRef       shapeRef      = parseShapeRef(instanceGroup, path, groupName);
    const YAML::Node    shapeDocument = loadYamlFile(shapeRef.path);
    const YAML::Node    shapeNodes    = optionalMapChild(shapeDocument, "shapes", shapeRef.path);
    const YAML::Node    shape         = requiredMapChild(shapeNodes, shapeRef.id, shapeRef.path);
    const std::uint32_t shapeIndex    = ensureShape(shapeRef.path, shape, shapeRef.id, result, shapeIndexByKey);
    const std::size_t   firstInstance = result.instances.size();

    for (const OffsetRow &offset : parseOffsets(instanceGroup, path, groupName))
    {
      result.instances.push_back(scene::ShapeInstance{.offset = offset.offset, .shapeIndex = shapeIndex});
    }
    result.shapeGroups.push_back(scene::ShapeGroup{
        .shaderName    = instanceGroup["shader"] && instanceGroup["shader"].IsScalar() ? trim(instanceGroup["shader"].as<std::string>()) : "triangle",
        .shapeIndex    = shapeIndex,
        .firstInstance = firstInstance,
        .instanceCount = result.instances.size() - firstInstance});
  }

  void appendShapeGroup(const std::filesystem::path &path, const YAML::Node &shapeGroup, const std::string_view groupName, scene::Scene &result)
  {
    const std::string shaderName    = parseShaderName(shapeGroup, path, groupName);
    const MeshRef     meshRef       = parseGroupMeshRef(shapeGroup, path, groupName);
    const auto        materials     = parseShapeGroupMaterials(shapeGroup, path, groupName);
    const YAML::Node  meshDocument  = loadYamlFile(meshRef.path);
    const YAML::Node  meshes        = optionalMapChild(meshDocument, "meshes", meshRef.path);
    const YAML::Node  mesh          = requiredMapChild(meshes, meshRef.id, meshRef.path);
    scene::Shape      parsedShape   = parseMesh(mesh, Material{}, meshRef.path);
    const std::size_t firstInstance = result.instances.size();

    if (parsedShape.vertices.empty() || parsedShape.indexes.empty())
    {
      throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + meshRef.path.string());
    }
    result.shapes.push_back(std::move(parsedShape));
    const auto shapeIndex = static_cast<std::uint32_t>(result.shapes.size() - 1U);

    for (const OffsetRow &offset : parseOffsets(shapeGroup, path, groupName))
    {
      const auto material = materials.find(offset.materialIndex);
      if (material == materials.end())
      {
        throw std::runtime_error("r8eA2R1nmB :: Missing material index " + std::to_string(offset.materialIndex) + " for shape group '" +
                                 std::string(groupName) + "' in " + path.string());
      }
      result.instances.push_back(
          scene::ShapeInstance{.offset     = offset.offset,
                               .color      = {material->second.solidColor[0], material->second.solidColor[1], material->second.solidColor[2]},
                               .scale      = material->second.scale,
                               .shapeIndex = shapeIndex});
    }
    result.shapeGroups.push_back(scene::ShapeGroup{.shaderName    = shaderName,
                                                   .shapeIndex    = shapeIndex,
                                                   .firstInstance = firstInstance,
                                                   .instanceCount = result.instances.size() - firstInstance});
  }

  void updateShapeInstanceRanges(scene::Scene &data)
  {
    for (scene::Shape &shape : data.shapes)
    {
      shape.firstInstance = 0;
      shape.instanceCount = 0;
    }
    for (std::size_t i = 0; i < data.instances.size(); ++i)
    {
      const scene::ShapeInstance shapeInstance = data.instances[i];
      scene::Shape              &shape         = data.shapes[shapeInstance.shapeIndex];

      if (shape.instanceCount == 0)
      {
        shape.firstInstance = i;
      }

      ++shape.instanceCount;
    }
  }

  void parseSceneCamera(const YAML::Node &document, const YAML::Node &scene, const std::filesystem::path &path, scene::Scene &result)
  {
    const YAML::Node cameraName = scene["camera"];
    if (!cameraName || !cameraName.IsScalar())
    {
      throw std::runtime_error("GM21xOpQV4 :: Missing YAML scalar 'scene.camera' in " + path.string());
    }

    const YAML::Node cameras              = optionalMapChild(document, "cameras", path);
    const YAML::Node camera               = requiredMapChild(cameras, cameraName.as<std::string>(), path);
    const YAML::Node geom                 = requiredMapChild(camera, "geom", path);
    result.camera.position                = parseVector3(geom["position"], path, "camera.geom.position");
    result.camera.forward                 = parseVector3(geom["forward"], path, "camera.geom.forward");
    result.camera.up                      = parseVector3(geom["up"], path, "camera.geom.up");
    result.camera.nearPlane               = parseFloatScalar(geom["near"], path, "camera.geom.near");
    result.camera.farPlane                = parseFloatScalar(geom["far"], path, "camera.geom.far");
    result.camera.fovDegrees              = parseFloatScalar(geom["fov"], path, "camera.geom.fov");
    const YAML::Node params               = requiredMapChild(camera, "params", path);
    result.camera.forwardVelocity         = parseFloatScalar(params["forwardVelocity"], path, "camera.params.forwardVelocity");
    result.camera.sideVelocity            = parseFloatScalar(params["sideVelocity"], path, "camera.params.sideVelocity");
    result.camera.forwardMouseSensitivity = parseFloatScalar(params["forwardMouseSensitivity"], path, "camera.params.forwardMouseSensitivity");
    result.camera.forwardScrollStep       = parseFloatScalar(params["forwardScrollStep"], path, "camera.params.forwardScrollStep");
    if (const YAML::Node forwardRotateDegPSec = params["forwardRotateDegPSec"])
    {
      result.camera.forwardRotateDegPSec = parseFloatScalar(forwardRotateDegPSec, path, "camera.params.forwardRotateDegPSec");
    }
  }

  void parseSceneSun(const YAML::Node &scene, const std::filesystem::path &path, scene::Scene &result)
  {
    const YAML::Node sun = scene["sun"];
    if (!sun)
    {
      return;
    }
    if (!sun.IsMap())
    {
      throw std::runtime_error("W2KgPJpzT7 :: YAML container 'scene.sun' must be map in " + path.string());
    }

    result.sun.force     = parseFloatScalar(sun["force"], path, "scene.sun.force");
    result.sun.direction = normalizeVector(parseVector3(sun["direction"], path, "scene.sun.direction"), path, "scene.sun.direction");
    result.sun.color     = parseVector3(sun["color"], path, "scene.sun.color");
  }

  void parseSceneParams(const YAML::Node &scene, const std::filesystem::path &path, scene::Scene &result)
  {
    const YAML::Node params = scene["params"];
    if (!params)
    {
      return;
    }
    if (!params.IsMap())
    {
      throw std::runtime_error("kIrW8mD13T :: YAML container 'scene.params' must be map in " + path.string());
    }
    if (const YAML::Node backgroundColor = params["background-color"])
    {
      result.params.backgroundColor = parseVector3(backgroundColor, path, "scene.params.background-color");
    }
  }
} // namespace

void scene::Scene::load(const std::filesystem::path &path, const std::string_view shapeName)
{
  const YAML::Node document = loadYamlFile(path);
  shapes.clear();
  instances.clear();
  shapeGroups.clear();
  camera = Camera{};
  sun    = Sun{};
  params = SceneParams{};

  if (shapeName.empty())
  {
    throw std::runtime_error("d5J2Mmx9Ar :: Shape name must not be empty");
  }
  const YAML::Node                               shapeNodes = optionalMapChild(document, "shapes", path);
  const YAML::Node                               shape      = requiredMapChild(shapeNodes, shapeName, path);
  std::unordered_map<std::string, std::uint32_t> shapeIndexByKey;
  appendShape(path, shape, shapeName, *this, shapeIndexByKey);
  updateShapeInstanceRanges(*this);

  if (shapes.empty() || instances.empty())
  {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }
}

void scene::Scene::load(const std::filesystem::path &path)
{
  const YAML::Node document  = loadYamlFile(path);
  const YAML::Node sceneNode = requiredMapChild(document, "scene", path);
  shapes.clear();
  instances.clear();
  shapeGroups.clear();
  camera = Camera{};
  sun    = Sun{};
  params = SceneParams{};

  parseSceneCamera(document, sceneNode, path, *this);
  parseSceneSun(sceneNode, path, *this);
  parseSceneParams(sceneNode, path, *this);

  const YAML::Node sceneShapeGroups         = sceneNode["shape-groups"];
  const YAML::Node sceneShapeInstanceGroups = sceneNode["shape-instance-groups"];
  if (sceneShapeGroups && !sceneShapeGroups.IsSequence())
  {
    throw std::runtime_error("xPH2tQyCKp :: YAML container 'scene.shape-groups' must be sequence in " + path.string());
  }
  if (sceneShapeInstanceGroups && !sceneShapeInstanceGroups.IsSequence())
  {
    throw std::runtime_error("EJEw5s6sPl :: YAML container 'scene.shape-instance-groups' must be sequence in " + path.string());
  }

  const YAML::Node shapeGroups         = optionalMapChild(document, "shape-groups", path);
  const YAML::Node shapeInstanceGroups = optionalMapChild(document, "shape-instance-groups", path);

  if (sceneShapeGroups)
  {
    for (const YAML::Node groupName : sceneShapeGroups)
    {
      if (!groupName.IsScalar())
      {
        throw std::runtime_error("g9I6G4iURO :: YAML 'scene.shape-groups' values must be scalar in " + path.string());
      }

      const std::string name       = groupName.as<std::string>();
      const YAML::Node  shapeGroup = requiredMapChild(shapeGroups, name, path);

      appendShapeGroup(path, shapeGroup, name, *this);
    }
  }
  else if (sceneShapeInstanceGroups)
  {
    std::unordered_map<std::string, std::uint32_t> shapeIndexByKey;
    for (const YAML::Node groupName : sceneShapeInstanceGroups)
    {
      if (!groupName.IsScalar())
      {
        throw std::runtime_error("eLfoLj0gQh :: YAML 'scene.shape-instance-groups' values must be scalar in " + path.string());
      }

      const std::string name          = groupName.as<std::string>();
      const YAML::Node  instanceGroup = requiredMapChild(shapeInstanceGroups, name, path);

      appendShapeInstanceGroup(path, instanceGroup, name, *this, shapeIndexByKey);
    }
  }
  updateShapeInstanceRanges(*this);

  if (shapes.empty() || instances.empty())
  {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }
}
