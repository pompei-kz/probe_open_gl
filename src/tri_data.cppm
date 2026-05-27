module;

#include <epoxy/gl.h>
#include <yaml-cpp/yaml.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module tri_data;

export namespace tri_data {
  struct Camera {
    std::array<float, 3> position{0.0F, 0.0F, 1.0F};
    std::array<float, 3> forward{0.0F, 0.0F, -1.0F};
    std::array<float, 3> up{0.0F, 1.0F, 0.0F};
    float nearPlane = 0.1F;
    float farPlane = 100.0F;
    float fovDegrees = 45.0F;
  };

  struct TriData {
    std::vector<float> vertices;
    std::vector<GLuint> indexes;
    Camera camera;
    int vertexFloatCount = 0;
    int positionFloatCount = 0;
    int colorFloatCount = 0;
  };

  TriData loadTriData(const std::filesystem::path &path, std::string_view figureName);
  TriData loadTriData(const std::filesystem::path &path);
}

namespace {
  struct DataSection {
    std::string type;
    std::vector<std::string> lines;
  };

  struct PointLayout {
    int vertexFloatCount = 0;
    int positionFloatCount = 0;
    int colorFloatCount = 0;
  };

  struct Material {
    std::optional<std::array<float, 3>> solidColor;
  };

  struct MeshRef {
    std::filesystem::path path;
    std::string id;
  };

  std::vector<float> parseFloatLine(std::string line);

  std::array<float, 3> parseVector3(const YAML::Node &node,
                                    const std::filesystem::path &path,
                                    const std::string_view name) {
    if (!node || !node.IsScalar()) {
      throw std::runtime_error("U3GddI64FP :: Missing YAML scalar '" + std::string(name)
                               + "' in " + path.string());
    }

    const std::vector<float> values = parseFloatLine(node.as<std::string>());
    if (values.size() != 3U) {
      throw std::runtime_error("zAGgTfSyde :: YAML scalar '" + std::string(name)
                               + "' must contain 3 floats in " + path.string());
    }
    return {values[0], values[1], values[2]};
  }

  float parseFloatScalar(const YAML::Node &node,
                         const std::filesystem::path &path,
                         const std::string_view name) {
    if (!node || !node.IsScalar()) {
      throw std::runtime_error("OtQRq1JqtC :: Missing YAML scalar '" + std::string(name)
                               + "' in " + path.string());
    }

    const std::vector<float> values = parseFloatLine(node.as<std::string>());
    if (values.size() != 1U) {
      throw std::runtime_error("MkrD0LxmpS :: YAML scalar '" + std::string(name)
                               + "' must contain 1 float in " + path.string());
    }
    return values[0];
  }

  std::string trim(const std::string_view value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string_view::npos) {
      return {};
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return std::string(value.substr(begin, end - begin + 1));
  }

  std::string stripComment(std::string value) {
    const auto commentPosition = value.find('#');
    if (commentPosition != std::string::npos) {
      value.erase(commentPosition);
    }
    return value;
  }

  std::vector<std::string> splitWords(const std::string_view value) {
    std::istringstream input{std::string(value)};
    std::vector<std::string> words;
    std::string word;
    while (input >> word) {
      words.push_back(word);
    }
    return words;
  }

  YAML::Node requiredMapChild(const YAML::Node &node,
                              const std::string_view childName,
                              const std::filesystem::path &path) {
    const YAML::Node child = node[std::string(childName)];
    if (!child || !child.IsMap()) {
      throw std::runtime_error("Afz2db44CN :: Missing YAML map '" + std::string(childName)
                               + "' in " + path.string());
    }
    return child;
  }

  std::vector<std::string> readLines(std::istream &input) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
      lines.push_back(line);
    }
    return lines;
  }

  std::vector<std::string> readDataRefLines(const YAML::Node &dataRef,
                                            const std::filesystem::path &sectionPath,
                                            const std::string_view sectionName) {
    if (!dataRef.IsScalar()) {
      throw std::runtime_error("pW2zTu5Lwd :: YAML '" + std::string(sectionName)
                               + ".data-ref' must be scalar in " + sectionPath.string());
    }

    const std::filesystem::path dataPath =
      (sectionPath.parent_path() / dataRef.as<std::string>()).lexically_normal();
    std::ifstream input(dataPath);
    if (!input) {
      throw std::runtime_error("KIuhrNQxT0 :: Failed to open YAML '" + std::string(sectionName)
                               + ".data-ref' file " + dataPath.string());
    }
    return readLines(input);
  }

  DataSection extractDataSection(const YAML::Node &mesh,
                                 const std::string_view sectionName,
                                 const std::filesystem::path &path) {
    const YAML::Node yamlSection = requiredMapChild(mesh, sectionName, path);
    const YAML::Node type = yamlSection["type"];
    if (!type || !type.IsScalar()) {
      throw std::runtime_error("WQ6V9pIWLa :: Missing YAML scalar '" + std::string(sectionName)
                               + ".type' in " + path.string());
    }

    const YAML::Node data = yamlSection["data"];
    const YAML::Node dataRef = yamlSection["data-ref"];
    if (data && dataRef) {
      throw std::runtime_error("AX1zWXwocG :: YAML section '" + std::string(sectionName)
                               + "' must not define both data and data-ref in " + path.string());
    }
    if (!data && !dataRef) {
      throw std::runtime_error("Z3H1qvTv5H :: YAML section '" + std::string(sectionName)
                               + "' must define data or data-ref in " + path.string());
    }

    DataSection section;
    section.type = type.as<std::string>();

    if (dataRef) {
      section.lines = readDataRefLines(dataRef, path, sectionName);
      return section;
    }

    if (!data.IsScalar()) {
      throw std::runtime_error("Om2FLJyLYx :: YAML '" + std::string(sectionName)
                               + ".data' must be scalar in " + path.string());
    }

    std::istringstream input(data.as<std::string>());
    section.lines = readLines(input);

    return section;
  }

  std::vector<float> parseFloatLine(std::string line) {
    line = stripComment(std::move(line));
    for (char &symbol: line) {
      if (symbol == ',') {
        symbol = ' ';
      }
    }

    std::istringstream input(line);
    std::vector<float> values;
    float value = 0.0F;
    while (input >> value) {
      values.push_back(value);
    }

    return values;
  }

  PointLayout parsePointLayout(const std::string &type, const std::filesystem::path &path) {
    const std::vector<std::string> fields = splitWords(type);
    if (fields.empty() || fields.front() != "i") {
      throw std::runtime_error("oGl0vjKJbD :: Invalid points type in " + path.string() + ": " + type);
    }

    PointLayout layout;
    if (fields == std::vector<std::string>{"i", "X", "Y", "Z", "CX", "CY", "CZ"}) {
      layout.vertexFloatCount = 6;
      layout.positionFloatCount = 3;
      layout.colorFloatCount = 3;
      return layout;
    }
    if (fields == std::vector<std::string>{"i", "X", "Y", "Z"}) {
      layout.vertexFloatCount = 3;
      layout.positionFloatCount = 3;
      layout.colorFloatCount = 0;
      return layout;
    }

    throw std::runtime_error("KjYkVw6sAv :: Points type must be 'i X Y Z CX CY CZ' or 'i X Y Z' in "
                             + path.string() + ": " + type);
  }

  int parseIndexCount(const std::string &type, const std::filesystem::path &path) {
    const std::vector<std::string> fields = splitWords(type);
    if (fields.empty()) {
      throw std::runtime_error("gMEOd5Wvmn :: Missing indexes type in " + path.string());
    }
    for (const std::string &field: fields) {
      if (field != "i") {
        throw std::runtime_error("LVw0x4xKiG :: Invalid indexes type in " + path.string() + ": " + type);
      }
    }
    return static_cast<int>(fields.size());
  }

  std::array<float, 3> parseSolidColor(const YAML::Node &material,
                                       const std::filesystem::path &path,
                                       const std::string_view figureName) {
    const YAML::Node color = material["color"];
    if (!color || !color.IsScalar()) {
      throw std::runtime_error("qP8hh93VfV :: Solid material for figure '" + std::string(figureName)
                               + "' must define scalar color in " + path.string());
    }

    const std::vector<float> values = parseFloatLine(color.as<std::string>());
    if (values.size() != 3U) {
      throw std::runtime_error("ZtG0L06ctZ :: Solid material color must contain 3 floats in " + path.string());
    }
    return {values[0], values[1], values[2]};
  }

  Material parseMaterial(const YAML::Node &figure,
                         const std::filesystem::path &path,
                         const std::string_view figureName) {
    const YAML::Node material = requiredMapChild(figure, "material", path);
    const YAML::Node type = material["type"];
    if (!type || !type.IsScalar()) {
      throw std::runtime_error("xLVuVqM5tJ :: Missing material.type for figure '" + std::string(figureName)
                               + "' in " + path.string());
    }

    const std::string materialType = type.as<std::string>();
    if (materialType != "solid") {
      throw std::runtime_error("UAJtwMZ764 :: Unsupported material type '" + materialType
                               + "' in " + path.string());
    }

    Material result;
    if (material["color"]) {
      result.solidColor = parseSolidColor(material, path, figureName);
    }
    return result;
  }

  MeshRef parseMeshRef(const YAML::Node &figure,
                       const std::filesystem::path &figurePath,
                       const std::string_view figureName) {
    const YAML::Node mesh = requiredMapChild(figure, "mesh", figurePath);
    const YAML::Node ref = mesh["ref"];
    if (!ref || !ref.IsScalar()) {
      throw std::runtime_error("AoiWfWDZDz :: Missing mesh.ref for figure '" + std::string(figureName)
                               + "' in " + figurePath.string());
    }

    const std::string value = ref.as<std::string>();
    const auto hashPosition = value.find('#');
    if (hashPosition == std::string::npos || hashPosition + 1 >= value.size()) {
      throw std::runtime_error("lJebVCn9ky :: Invalid mesh ref '" + value + "' in " + figurePath.string());
    }

    MeshRef result;
    result.id = value.substr(hashPosition + 1);
    const std::string relativePath = value.substr(0, hashPosition);
    result.path = relativePath.empty()
      ? figurePath
      : (figurePath.parent_path() / relativePath).lexically_normal();
    return result;
  }

  void appendMesh(const YAML::Node &mesh,
                  const Material &material,
                  const std::filesystem::path &meshPath,
                  tri_data::TriData &result) {
    const DataSection points = extractDataSection(mesh, "points", meshPath);
    const DataSection indexes = extractDataSection(mesh, "indexes", meshPath);
    const PointLayout layout = parsePointLayout(points.type, meshPath);
    const int indexCount = parseIndexCount(indexes.type, meshPath);

    if (indexCount != 3) {
      throw std::runtime_error("cDO6R2F96Y :: Index type must be 'i i i' for GL_TRIANGLES in "
                               + meshPath.string() + ": " + indexes.type);
    }
    if (layout.colorFloatCount == 0 && !material.solidColor) {
      throw std::runtime_error("S9A8rL1zLy :: Mesh without point colors needs solid material color in "
                               + meshPath.string());
    }

    std::unordered_map<int, GLuint> pointIndexById;
    for (const std::string &line: points.lines) {
      const std::vector<float> values = parseFloatLine(line);
      if (values.empty()) {
        continue;
      }
      if (values.size() != 1 + static_cast<std::size_t>(layout.vertexFloatCount)) {
        throw std::runtime_error("pSo3Hqn3wN :: Invalid point row in " + meshPath.string() + ": " + trim(line));
      }

      const int id = static_cast<int>(values[0]);
      pointIndexById.emplace(id, static_cast<GLuint>(result.vertices.size() / result.vertexFloatCount));
      result.vertices.insert(result.vertices.end(), values.begin() + 1, values.begin() + 1 + layout.positionFloatCount);
      if (layout.colorFloatCount > 0) {
        result.vertices.insert(result.vertices.end(),
                               values.begin() + 1 + layout.positionFloatCount,
                               values.end());
      } else {
        result.vertices.insert(result.vertices.end(), material.solidColor->begin(), material.solidColor->end());
      }
    }

    for (const std::string &line: indexes.lines) {
      const std::vector<float> values = parseFloatLine(line);
      if (values.empty()) {
        continue;
      }
      if (values.size() != static_cast<std::size_t>(indexCount)) {
        throw std::runtime_error("w3jRjFEh8q :: Invalid index row in " + meshPath.string() + ": " + trim(line));
      }

      std::array<GLuint, 3> triangle{};
      bool validTriangle = true;
      for (std::size_t i = 0; i < values.size(); ++i) {
        const int pointId = static_cast<int>(values[i]);
        const auto pointIndex = pointIndexById.find(pointId);
        if (pointIndex == pointIndexById.end()) {
          std::cerr << "K5fcGkrP2g :: Skipping triangle with missing point id " << pointId
                    << " in " << meshPath << '\n';
          validTriangle = false;
          break;
        }
        triangle[i] = pointIndex->second;
      }

      if (validTriangle) {
        result.indexes.insert(result.indexes.end(), triangle.begin(), triangle.end());
      }
    }
  }

  void appendFigure(const std::filesystem::path &path,
                    const YAML::Node &figure,
                    const std::string_view figureName,
                    tri_data::TriData &result) {
    const MeshRef meshRef = parseMeshRef(figure, path, figureName);
    const Material material = parseMaterial(figure, path, figureName);
    const YAML::Node meshDocument = YAML::LoadFile(meshRef.path.string());
    const YAML::Node meshes = requiredMapChild(meshDocument, "meshes", meshRef.path);
    const YAML::Node mesh = requiredMapChild(meshes, meshRef.id, meshRef.path);
    appendMesh(mesh, material, meshRef.path, result);
  }

  void setDefaultLayout(tri_data::TriData &data) {
    data.vertexFloatCount = 6;
    data.positionFloatCount = 3;
    data.colorFloatCount = 3;
  }

  void parseSceneCamera(const YAML::Node &document,
                        const YAML::Node &scene,
                        const std::filesystem::path &path,
                        tri_data::TriData &result) {
    const YAML::Node cameraName = scene["camera"];
    if (!cameraName || !cameraName.IsScalar()) {
      throw std::runtime_error("GM21xOpQV4 :: Missing YAML scalar 'scene.camera' in " + path.string());
    }

    const YAML::Node cameras = requiredMapChild(document, "cameras", path);
    const YAML::Node camera = requiredMapChild(cameras, cameraName.as<std::string>(), path);
    result.camera.position = parseVector3(camera["position"], path, "camera.position");
    result.camera.forward = parseVector3(camera["forward"], path, "camera.forward");
    result.camera.up = parseVector3(camera["up"], path, "camera.up");
    result.camera.nearPlane = parseFloatScalar(camera["near"], path, "camera.near");
    result.camera.farPlane = parseFloatScalar(camera["far"], path, "camera.far");
    result.camera.fovDegrees = parseFloatScalar(camera["fov"], path, "camera.fov");
  }
}

tri_data::TriData tri_data::loadTriData(const std::filesystem::path &path, std::string_view figureName) {
  const YAML::Node document = YAML::LoadFile(path.string());
  TriData result;
  setDefaultLayout(result);
  if (figureName.empty()) {
    throw std::runtime_error("d5J2Mmx9Ar :: Figure name must not be empty");
  }
  const YAML::Node figures = requiredMapChild(document, "figures", path);
  const YAML::Node figure = requiredMapChild(figures, figureName, path);
  appendFigure(path, figure, figureName, result);

  if (result.vertices.empty() || result.indexes.empty()) {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }

  return result;
}

tri_data::TriData tri_data::loadTriData(const std::filesystem::path &path) {
  const YAML::Node document = YAML::LoadFile(path.string());
  const YAML::Node scene = requiredMapChild(document, "scene", path);
  const YAML::Node sceneFigures = scene["figures"];
  if (!sceneFigures || !sceneFigures.IsSequence()) {
    throw std::runtime_error("mXM3mP9a4X :: Missing YAML sequence 'scene.figures' in " + path.string());
  }
  const YAML::Node figures = requiredMapChild(document, "figures", path);
  TriData result;
  setDefaultLayout(result);
  parseSceneCamera(document, scene, path, result);

  for (const YAML::Node figureName: sceneFigures) {
    if (!figureName.IsScalar()) {
      throw std::runtime_error("r6FNtRCvBb :: YAML 'scene.figures' values must be scalar in " + path.string());
    }
    const std::string name = figureName.as<std::string>();
    const YAML::Node figure = requiredMapChild(figures, name, path);
    appendFigure(path, figure, name, result);
  }

  if (result.vertices.empty() || result.indexes.empty()) {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }

  return result;
}
