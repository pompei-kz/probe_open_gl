module;

#include <epoxy/gl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module tri_data;

export namespace tri_data {
  struct TriData {
    std::vector<float> vertices;
    std::vector<GLuint> indexes;
    int vertexFloatCount = 0;
    int positionFloatCount = 0;
    int colorFloatCount = 0;
  };

  TriData loadTriData(const std::filesystem::path &path, std::string_view figureName);
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

  std::string stripQuotes(std::string value) {
    value = trim(value);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
      return value.substr(1, value.size() - 2);
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

  DataSection extractDataSection(const std::string &yaml,
                                 const std::string_view figureName,
                                 const std::string_view sectionName) {
    if (figureName.empty()) {
      throw std::runtime_error("d5J2Mmx9Ar :: Figure name must not be empty");
    }

    std::istringstream input(yaml);
    std::string line;
    bool inFigure = false;
    bool inSection = false;
    bool inData = false;
    DataSection section;

    while (std::getline(input, line)) {
      const std::string trimmed = trim(line);
      if (trimmed.empty()) {
        if (inData) {
          section.lines.emplace_back();
        }
        continue;
      }

      if (!line.starts_with(" ") && trimmed.ends_with(":")) {
        if (inFigure) {
          break;
        }

        inFigure = trimmed == std::string(figureName) + ":";
        inSection = false;
        inData = false;
        continue;
      }

      if (!inFigure) {
        continue;
      }

      if (trimmed == std::string(sectionName) + ":") {
        inSection = true;
        inData = false;
        continue;
      }

      if (inSection && trimmed.ends_with(":") && trimmed != "data: |") {
        inSection = false;
        inData = false;
      }

      if (inSection && trimmed.starts_with("type:")) {
        section.type = stripQuotes(trimmed.substr(std::string_view("type:").size()));
        continue;
      }

      if (inSection && trimmed == "data: |") {
        inData = true;
        continue;
      }

      if (inData) {
        section.lines.push_back(line);
      }
    }

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
    if (fields.size() < 2 || fields.front() != "i") {
      throw std::runtime_error("oGl0vjKJbD :: Invalid points type in " + path.string() + ": " + type);
    }

    PointLayout layout;
    bool inColors = false;
    for (auto field = fields.begin() + 1; field != fields.end(); ++field) {
      ++layout.vertexFloatCount;
      if (field->starts_with("C")) {
        inColors = true;
        ++layout.colorFloatCount;
      } else {
        if (inColors) {
          throw std::runtime_error("nFRYw69ZVw :: Point position fields must appear before color fields in "
                                   + path.string() + ": " + type);
        }
        ++layout.positionFloatCount;
      }
    }

    if (layout.positionFloatCount <= 0 || layout.colorFloatCount <= 0) {
      throw std::runtime_error("KjYkVw6sAv :: Points type must define position and color fields in "
                               + path.string() + ": " + type);
    }

    return layout;
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
}

tri_data::TriData tri_data::loadTriData(const std::filesystem::path &path, std::string_view figureName) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("8rJR8DpT9m :: Failed to open " + path.string());
  }

  const std::string yaml((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
  const DataSection points = extractDataSection(yaml, figureName, "points");
  const DataSection indexes = extractDataSection(yaml, figureName, "indexes");
  const PointLayout layout = parsePointLayout(points.type, path);
  const int indexCount = parseIndexCount(indexes.type, path);

  TriData result;
  result.vertexFloatCount = layout.vertexFloatCount;
  result.positionFloatCount = layout.positionFloatCount;
  result.colorFloatCount = layout.colorFloatCount;

  std::unordered_map<int, GLuint> pointIndexById;

  for (const std::string &line: points.lines) {
    const std::vector<float> values = parseFloatLine(line);
    if (values.empty()) {
      continue;
    }
    if (values.size() != 1 + static_cast<std::size_t>(result.vertexFloatCount)) {
      throw std::runtime_error("pSo3Hqn3wN :: Invalid point row in " + path.string() + ": " + trim(line));
    }

    const int id = static_cast<int>(values[0]);
    pointIndexById.emplace(id, static_cast<GLuint>(result.vertices.size() / result.vertexFloatCount));
    result.vertices.insert(result.vertices.end(), values.begin() + 1, values.end());
  }

  for (const std::string &line: indexes.lines) {
    const std::vector<float> values = parseFloatLine(line);
    if (values.empty()) {
      continue;
    }
    if (values.size() != static_cast<std::size_t>(indexCount)) {
      throw std::runtime_error("w3jRjFEh8q :: Invalid index row in " + path.string() + ": " + trim(line));
    }

    std::vector<GLuint> triangle(static_cast<std::size_t>(indexCount));
    bool validTriangle = true;
    for (std::size_t i = 0; i < values.size(); ++i) {
      const int pointId = static_cast<int>(values[i]);
      const auto pointIndex = pointIndexById.find(pointId);
      if (pointIndex == pointIndexById.end()) {
        std::cerr << "K5fcGkrP2g :: Skipping triangle with missing point id " << pointId
                  << " in " << path << '\n';
        validTriangle = false;
        break;
      }
      triangle[i] = pointIndex->second;
    }

    if (validTriangle) {
      result.indexes.insert(result.indexes.end(), triangle.begin(), triangle.end());
    }
  }

  if (result.vertices.empty() || result.indexes.empty()) {
    throw std::runtime_error("F8gTBaZnSl :: No drawable triangle data in " + path.string());
  }

  return result;
}
