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

TEST(LoadTriData, LoadsRequestedFigureFromFiguresRoot) {
  const std::filesystem::path path = writeYaml("loads_requested_figure", R"yaml(
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
figures:
  ignored_figure:
    mesh:
      ref: "#ignored"
    material:
      type: "solid"
  target:
    mesh:
      ref: "#target"
    material:
      type: "solid"
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

TEST(LoadTriData, UsesSolidMaterialColorWhenMeshHasNoColors) {
  const std::filesystem::path path = writeYaml("uses_material_color", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data: |
        1 0.0 0.0 0.0
        2 1.0 0.0 0.0
        3 1.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
figures:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.2 0.4 0.6"
)yaml");

  const tri_data::TriData data = tri_data::loadTriData(path, "target");

  EXPECT_EQ(data.vertexFloatCount, 6);
  EXPECT_EQ(data.positionFloatCount, 3);
  EXPECT_EQ(data.colorFloatCount, 3);
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_FLOAT_EQ(data.vertices[3], 0.2F);
  EXPECT_FLOAT_EQ(data.vertices[4], 0.4F);
  EXPECT_FLOAT_EQ(data.vertices[5], 0.6F);
  EXPECT_FLOAT_EQ(data.vertices[15], 0.2F);
  EXPECT_FLOAT_EQ(data.vertices[16], 0.4F);
  EXPECT_FLOAT_EQ(data.vertices[17], 0.6F);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
}

TEST(LoadTriData, LoadsAllFigures) {
  const std::filesystem::path path = writeYaml("loads_all_figures", R"yaml(
meshes:
  first:
    points:
      type: "i X Y Z CX CY CZ"
      data: |
        1 0.0 0.0 0.0 1.0 0.0 0.0
        2 1.0 0.0 0.0 0.0 1.0 0.0
        3 0.0 1.0 0.0 0.0 0.0 1.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
  second:
    points:
      type: "i X Y Z"
      data: |
        1 -1.0 0.0 0.0
        2  0.0 0.0 0.0
        3 -1.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
figures:
  figure_01:
    mesh:
      ref: "#first"
    material:
      type: "solid"
  figure_02:
    mesh:
      ref: "#second"
    material:
      type: "solid"
      color: "0.25 0.5 0.75"
)yaml");

  const tri_data::TriData data = tri_data::loadTriData(path);

  EXPECT_EQ(data.vertexFloatCount, 6);
  EXPECT_EQ(data.vertices.size(), 36U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2, 3, 4, 5}));
  EXPECT_FLOAT_EQ(data.vertices[21], 0.25F);
  EXPECT_FLOAT_EQ(data.vertices[22], 0.5F);
  EXPECT_FLOAT_EQ(data.vertices[23], 0.75F);
}

TEST(LoadTriData, ResolvesMeshReferencesFromRelativeFiles) {
  const std::filesystem::path meshPath = writeYaml("external_meshes", R"yaml(
meshes:
  external:
    points:
      type: "i X Y Z"
      data: |
        1 0.0 0.0 0.0
        2 1.0 0.0 0.0
        3 0.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
)yaml");

  const std::filesystem::path path = writeYaml("relative_mesh_ref", R"yaml(
figures:
  target:
    mesh:
      ref: "probe_open_gl_external_meshes.yaml#external"
    material:
      type: "solid"
      color: "0.1 0.2 0.3"
)yaml");

  const tri_data::TriData data = tri_data::loadTriData(path, "target");

  (void)meshPath;
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.vertices[3], 0.1F);
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
figures:
  existing:
    mesh:
      ref: "#existing"
    material:
      type: "solid"
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
figures:
  bad:
    mesh:
      ref: "#bad"
    material:
      type: "solid"
)yaml");

  EXPECT_THROW((void)tri_data::loadTriData(path, "bad"), std::runtime_error);
}
