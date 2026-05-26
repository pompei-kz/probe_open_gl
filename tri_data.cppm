module;

#include <epoxy/gl.h>

#include <array>
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
  };

  constexpr int kVertexFloatCount = 6;
  constexpr int kPositionFloatCount = 3;
  constexpr int kColorFloatCount = 3;

  TriData loadTriData(const std::filesystem::path &path);
}

namespace {
  std::string trim(std::string_view value) {
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

  std::vector<std::string> extractDataBlock(const std::string &yaml, std::string_view sectionName) {
    std::istringstream input(yaml);
    std::string line;
    bool inMainTriData = false;
    bool inSection = false;
    bool inData = false;
    std::vector<std::string> lines;

    while (std::getline(input, line)) {
      const std::string trimmed = trim(line);
      if (trimmed.empty()) {
        if (inData) {
          lines.emplace_back();
        }
        continue;
      }

      if (trimmed == "main-tri-data:") {
        inMainTriData = true;
        inSection = false;
        inData = false;
        continue;
      }

      if (!inMainTriData) {
        continue;
      }

      if (!line.starts_with(" ") && trimmed.ends_with(":")) {
        break;
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

      if (inSection && trimmed == "data: |") {
        inData = true;
        continue;
      }

      if (inData) {
        lines.push_back(line);
      }
    }

    return lines;
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
}

tri_data::TriData tri_data::loadTriData(const std::filesystem::path &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("8rJR8DpT9m :: Failed to open " + path.string());
  }

  const std::string yaml((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
  const std::vector<std::string> pointLines = extractDataBlock(yaml, "points");
  const std::vector<std::string> indexLines = extractDataBlock(yaml, "indexes");

  TriData result;
  std::unordered_map<int, GLuint> pointIndexById;

  for (const std::string &line: pointLines) {
    const std::vector<float> values = parseFloatLine(line);
    if (values.empty()) {
      continue;
    }
    if (values.size() != 1 + kVertexFloatCount) {
      throw std::runtime_error("pSo3Hqn3wN :: Invalid point row in " + path.string() + ": " + trim(line));
    }

    const int id = static_cast<int>(values[0]);
    pointIndexById.emplace(id, static_cast<GLuint>(result.vertices.size() / kVertexFloatCount));
    result.vertices.insert(result.vertices.end(), values.begin() + 1, values.end());
  }

  for (const std::string &line: indexLines) {
    const std::vector<float> values = parseFloatLine(line);
    if (values.empty()) {
      continue;
    }
    if (values.size() != 3) {
      throw std::runtime_error("w3jRjFEhtq :: Invalid index row in " + path.string() + ": " + trim(line));
    }

    std::array<GLuint, 3> triangle{};
    bool validTriangle = true;
    for (std::size_t i = 0; i < triangle.size(); ++i) {
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
