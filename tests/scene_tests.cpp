#include <epoxy/gl.h>
#include <gtest/gtest.h>

import scene;

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

  std::filesystem::path writeTextFile(const std::string &fileName, const std::string &content) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / fileName;
    std::ofstream output(path);
    output << content;
    return path;
  }
}

TEST(LoadScene, LoadsRequestedFigureFromFiguresRoot) {
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

  scene::Scene data;
  data.load(path, "target");

  EXPECT_EQ(data.vertexFloatCount, 6);
  EXPECT_EQ(data.positionFloatCount, 3);
  EXPECT_EQ(data.colorFloatCount, 3);
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_FLOAT_EQ(data.vertices[0], -1.0F);
  EXPECT_FLOAT_EQ(data.vertices[6], 1.0F);
  EXPECT_FLOAT_EQ(data.vertices[12], 0.0F);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
}

TEST(LoadScene, UsesSolidMaterialColorWhenMeshHasNoColors) {
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

  scene::Scene data;
  data.load(path, "target");

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

TEST(LoadScene, DoesNotDrawSceneFiguresDirectly) {
  const std::filesystem::path path = writeYaml("does_not_draw_scene_figures", R"yaml(
scene:
  figures:
    - figure_01
  camera: "main"
cameras:
  main:
    geom:
      position: "0 0 10"
      forward: "0 0 -1"
      up: "0 1 0"
      near: "0.1"
      far: "100"
      fov: "45"
    params:
      forwardVelocity: "1"
      sideVelocity: "4.5"
      forwardMouseSensitivity: "0.1"
      forwardScrollStep: "1"
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
figures:
  figure_01:
    mesh:
      ref: "#first"
    material:
      type: "solid"
)yaml");

  EXPECT_THROW({ scene::Scene data; data.load(path); }, std::runtime_error);
}

TEST(LoadScene, LoadsSelectedFigureInstanceGroupsWithOffsets) {
  const std::filesystem::path path = writeYaml("loads_figure_instance_groups", R"yaml(
scene:
  figures:
    - ignored_direct_figure
  figure-instance-groups:
    - selected_group
  camera: "main"
cameras:
  main:
    geom:
      position: "0 0 10"
      forward: "0 0 -2"
      up: "0 1 0.2"
      near: "0.2"
      far: "250"
      fov: "60"
    params:
      forwardVelocity: "1.5"
      sideVelocity: "4.5"
      forwardMouseSensitivity: "0.2"
      forwardScrollStep: "2.5"
meshes:
  triangle:
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
figures:
  ignored_direct_figure:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "1.0 1.0 1.0"
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.2 0.4 0.6"
figure-instance-groups:
  selected_group:
    figure:
      ref: "#target"
    offsets:
      type: "i X Y Z"
      data: |
        0 10 20 30
        1 -1 -2 -3
  ignored_group:
    figure:
      ref: "#target"
    offsets:
      type: "i X Y Z"
      data: |
        0 100 200 300
)yaml");

  scene::Scene data;
  data.load(path);

  EXPECT_EQ(data.vertexFloatCount, 6);
  ASSERT_EQ(data.vertices.size(), 36U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2, 3, 4, 5}));
  EXPECT_FLOAT_EQ(data.vertices[0], 10.0F);
  EXPECT_FLOAT_EQ(data.vertices[1], 20.0F);
  EXPECT_FLOAT_EQ(data.vertices[2], 30.0F);
  EXPECT_FLOAT_EQ(data.vertices[6], 11.0F);
  EXPECT_FLOAT_EQ(data.vertices[12], 10.0F);
  EXPECT_FLOAT_EQ(data.vertices[18], -1.0F);
  EXPECT_FLOAT_EQ(data.vertices[19], -2.0F);
  EXPECT_FLOAT_EQ(data.vertices[20], -3.0F);
  EXPECT_FLOAT_EQ(data.vertices[24], 0.0F);
  EXPECT_FLOAT_EQ(data.vertices[30], -1.0F);
  EXPECT_FLOAT_EQ(data.camera.position[2], 10.0F);
  EXPECT_FLOAT_EQ(data.camera.forward[2], -2.0F);
  EXPECT_FLOAT_EQ(data.camera.up[2], 0.2F);
  EXPECT_FLOAT_EQ(data.camera.nearPlane, 0.2F);
  EXPECT_FLOAT_EQ(data.camera.farPlane, 250.0F);
  EXPECT_FLOAT_EQ(data.camera.fovDegrees, 60.0F);
  EXPECT_FLOAT_EQ(data.camera.forwardVelocity, 1.5F);
  EXPECT_FLOAT_EQ(data.camera.sideVelocity, 4.5F);
  EXPECT_FLOAT_EQ(data.camera.forwardMouseSensitivity, 0.2F);
  EXPECT_FLOAT_EQ(data.camera.forwardScrollStep, 2.5F);
}

TEST(LoadScene, ResolvesMeshReferencesFromRelativeFiles) {
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

  scene::Scene data;
  data.load(path, "target");

  (void)meshPath;
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.vertices[3], 0.1F);
}

TEST(LoadScene, ReadsPointAndIndexDataRefs) {
  const std::filesystem::path pointsPath = writeTextFile("probe_open_gl_points_data_ref.txt", R"txt(
1 0.0 0.0 0.0
2 1.0 0.0 0.0
3 0.0 1.0 0.0
)txt");
  const std::filesystem::path indexesPath = writeTextFile("probe_open_gl_indexes_data_ref.txt", R"txt(
1 2 3
)txt");
  const std::filesystem::path path = writeYaml("data_refs", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data-ref: "probe_open_gl_points_data_ref.txt"
    indexes:
      type: "i i i"
      data-ref: "probe_open_gl_indexes_data_ref.txt"
figures:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.1 0.2 0.3"
)yaml");

  scene::Scene data;
  data.load(path, "target");

  (void)pointsPath;
  (void)indexesPath;
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.vertices[3], 0.1F);
}

TEST(LoadScene, IgnoresEmptyLinesAndCommentsInInlineData) {
  const std::filesystem::path path = writeYaml("inline_empty_lines_and_comments", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data: |
        # point id and coordinates
        1 0.0 0.0 0.0

        2 1.0 0.0 0.0 # trailing comment

        # another comment
        3 0.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        # one triangle

        1 2 3 # by point ids
figures:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.7 0.8 0.9"
)yaml");

  scene::Scene data;
  data.load(path, "target");

  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.vertices[3], 0.7F);
  EXPECT_FLOAT_EQ(data.vertices[4], 0.8F);
  EXPECT_FLOAT_EQ(data.vertices[5], 0.9F);
}

TEST(LoadScene, IgnoresEmptyLinesAndCommentsInDataRefs) {
  const std::filesystem::path pointsPath = writeTextFile("probe_open_gl_points_comments_ref.txt", R"txt(
# point id and coordinates
1 0.0 0.0 0.0

2 1.0 0.0 0.0 # trailing comment

# another comment
3 0.0 1.0 0.0
)txt");
  const std::filesystem::path indexesPath = writeTextFile("probe_open_gl_indexes_comments_ref.txt", R"txt(
# one triangle

1 2 3 # by point ids
)txt");
  const std::filesystem::path path = writeYaml("data_refs_empty_lines_and_comments", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data-ref: "probe_open_gl_points_comments_ref.txt"
    indexes:
      type: "i i i"
      data-ref: "probe_open_gl_indexes_comments_ref.txt"
figures:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.4 0.5 0.6"
)yaml");

  scene::Scene data;
  data.load(path, "target");

  (void)pointsPath;
  (void)indexesPath;
  ASSERT_EQ(data.vertices.size(), 18U);
  EXPECT_EQ(data.indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.vertices[3], 0.4F);
  EXPECT_FLOAT_EQ(data.vertices[4], 0.5F);
  EXPECT_FLOAT_EQ(data.vertices[5], 0.6F);
}

TEST(LoadScene, ThrowsWhenSectionHasDataAndDataRef) {
  const std::filesystem::path path = writeYaml("data_and_data_ref", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data: |
        1 0.0 0.0 0.0
        2 1.0 0.0 0.0
        3 0.0 1.0 0.0
      data-ref: "unused.txt"
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
      color: "0.1 0.2 0.3"
)yaml");

  EXPECT_THROW({ scene::Scene data; data.load(path, "target"); }, std::runtime_error);
}

TEST(LoadScene, ThrowsWhenSectionHasNeitherDataNorDataRef) {
  const std::filesystem::path path = writeYaml("no_data_or_data_ref", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
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
      color: "0.1 0.2 0.3"
)yaml");

  EXPECT_THROW({ scene::Scene data; data.load(path, "target"); }, std::runtime_error);
}

TEST(LoadScene, ThrowsWhenMeshIsMissing) {
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

  EXPECT_THROW({ scene::Scene data; data.load(path, "missing"); }, std::runtime_error);
}

TEST(LoadScene, ThrowsWhenPointRowDoesNotMatchType) {
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

  EXPECT_THROW({ scene::Scene data; data.load(path, "bad"); }, std::runtime_error);
}
