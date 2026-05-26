#include <epoxy/gl.h>
#include <gtest/gtest.h>

import tri_data;

#include <filesystem>
#include <fstream>
#include <string>

namespace {
  std::filesystem::path writeYaml(const std::string &testName, const std::string &yaml) {
    std::string fileName = "probe_open_gl_" + testName + ".yaml";
    for (char &symbol: fileName) {
      if (symbol == '/' || symbol == '\\' || symbol == ' ') {
        symbol = '_';
      }
    }

    const std::filesystem::path path = std::filesystem::temp_directory_path() / fileName;
    std::ofstream output(path);
    output << yaml;
    return path;
  }
}

TEST(LoadTriData, LoadsRequestedMeshFromMeshesRoot) {
  const std::filesystem::path path = writeYaml("loads_requested_mesh", R"yaml(
meshes:
  ignored:
    points:
      type: "i X Y Z CX CY CZ"
      data: |
        1 0 0 0 1 0 0
        2 1 0 0 0 1 0
        3 0 1 0 0 0 1
    indexes:
      type: "i i i"
      data: |
        1 2 3
  target:
    points:
      type: "i X Y Z CX CY CZ"
      data: |
        10 -1.0 0.0 0.0 1.0 0.0 0.0
        20  1.0 0.0 0.0 0.0 1.0 0.0
        30  0.0 1.0 0.0 0.0 0.0 1.0
    indexes:
      type: "i i i"
      data: |
        10 20 30
)yaml");

  const tri_data::TriData data = tri_data::loadTriData(path, "target");

  EXPECT_EQ(data.vertexFloatCount, 6);
  EXPECT_EQ(data.positionFloatCount, 3);
  EXPECT_EQ(data.colorFloatCount, 3);
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_FLOAT_EQ(data.vertices[0], -1.0F);
  EXPECT_FLOAT_EQ(data.vertices[6], 1.0F);
  EXPECT_FLOAT_EQ(data.vertices[12], 0.0F);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
}

TEST(LoadTriData, UsesYamlTypesForLayoutAndIndexWidth) {
  const std::filesystem::path path = writeYaml("uses_yaml_types", R"yaml(
meshes:
  quad:
    points:
      type: "i X Y CX CY"
      data: |
        1 0.0 0.0 1.0 0.0
        2 1.0 0.0 0.0 1.0
        3 1.0 1.0 1.0 1.0
        4 0.0 1.0 0.5 0.5
    indexes:
      type: "i i i i"
      data: |
        1 2 3 4
)yaml");

  const tri_data::TriData data = tri_data::loadTriData(path, "quad");

  EXPECT_EQ(data.vertexFloatCount, 4);
  EXPECT_EQ(data.positionFloatCount, 2);
  EXPECT_EQ(data.colorFloatCount, 2);
  EXPECT_EQ(data.vertices.size(), 16U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2, 3}));
}

TEST(LoadTriData, ThrowsWhenMeshIsMissing) {
  const std::filesystem::path path = writeYaml("missing_mesh", R"yaml(
meshes:
  existing:
    points:
      type: "i X Y Z CX CY CZ"
      data: |
        1 0 0 0 1 0 0
    indexes:
      type: "i i i"
      data: |
        1 1 1
)yaml");

  EXPECT_THROW((void)tri_data::loadTriData(path, "missing"), std::runtime_error);
}

TEST(LoadTriData, ThrowsWhenPointRowDoesNotMatchType) {
  const std::filesystem::path path = writeYaml("invalid_point_row", R"yaml(
meshes:
  bad:
    points:
      type: "i X Y Z CX CY CZ"
      data: |
        1 0 0 0 1 0
    indexes:
      type: "i i i"
      data: |
        1 1 1
)yaml");

  EXPECT_THROW((void)tri_data::loadTriData(path, "bad"), std::runtime_error);
}
