#include <epoxy/gl.h>
#include <gtest/gtest.h>

import scene;

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
  std::filesystem::path writeYaml(const std::string &testName, const std::string &yaml)
  {
    std::string fileName = "probe_open_gl_" + testName + ".yaml";
    for (char &symbol : fileName)
    {
      if (symbol == '/' || symbol == '\\' || symbol == ' ')
      {
        symbol = '_';
      }
    }

    const std::filesystem::path path = std::filesystem::temp_directory_path() / fileName;
    std::ofstream               output(path);
    output << yaml;
    return path;
  }

  std::filesystem::path writeTextFile(const std::string &fileName, const std::string &content)
  {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / fileName;
    std::ofstream               output(path);
    output << content;
    return path;
  }

  void expectRuntimeErrorContains(const std::filesystem::path &path, const std::string &text)
  {
    try
    {
      scene::Scene data;
      data.load(path);
      FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error &exception)
    {
      EXPECT_NE(std::string(exception.what()).find(text), std::string::npos);
    }
  }

  void expectRuntimeErrorContains(const std::filesystem::path &path, const std::string &shapeName, const std::string &text)
  {
    try
    {
      scene::Scene data;
      data.load(path, shapeName);
      FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error &exception)
    {
      EXPECT_NE(std::string(exception.what()).find(text), std::string::npos);
    }
  }
} // namespace

TEST(LoadScene, LoadsRequestedShapeFromShapesRoot)
{
  const std::filesystem::path path = writeYaml("loads_requested_shape", R"yaml(
meshes:
  ignored:
    points:
      type: "i X Y Z"
      data: |
        1 0 0 0
        2 1 0 0
        3 0 1 0
    indexes:
      type: "i i i"
      data: |
        1 2 3
  target:
    points:
      type: "i X Y Z"
      data: |
        10 -1.0 0.0 0.0
        20  1.0 0.0 0.0
        30  0.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        10 20 30
shapes:
  ignored_shape:
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

  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.instances.size(), 1U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[0], -1.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[3], 1.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[4], 1.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[5], 1.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[6], 1.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[12], 0.0F);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_EQ(data.instances[0].shapeIndex, 0U);
  EXPECT_EQ(data.shapes[0].firstInstance, 0U);
  EXPECT_EQ(data.shapes[0].instanceCount, 1U);
}

TEST(LoadScene, TreatsMissingShapesContainerAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_shapes_container", R"yaml(
meshes: {}
)yaml");

  expectRuntimeErrorContains(path, "target", " :: Missing YAML map 'target'");
}

TEST(LoadScene, TreatsMissingMeshesContainerAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_meshes_container", R"yaml(
shapes:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
)yaml");

  expectRuntimeErrorContains(path, "target", " :: Missing YAML map 'triangle'");
}

TEST(LoadScene, TreatsMissingCamerasContainerAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_cameras_container", R"yaml(
scene:
  camera: "main"
)yaml");

  expectRuntimeErrorContains(path, " :: Missing YAML map 'main'");
}

TEST(LoadScene, TreatsMissingShapeInstanceGroupsContainerAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_shape_instance_groups_container", R"yaml(
scene:
  camera: "main"
  shape-instance-groups:
    - selected_group
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
      sideVelocity: "1"
      forwardMouseSensitivity: "0.1"
      forwardScrollStep: "1"
)yaml");

  expectRuntimeErrorContains(path, " :: Missing YAML map 'selected_group'");
}

TEST(LoadScene, TreatsMissingSceneShapeInstanceGroupsAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_scene_shape_instance_groups", R"yaml(
scene:
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
      sideVelocity: "1"
      forwardMouseSensitivity: "0.1"
      forwardScrollStep: "1"
)yaml");

  expectRuntimeErrorContains(path, " :: No drawable triangle data");
}

TEST(LoadScene, UsesSolidMaterialColorWhenMeshHasNoColors)
{
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
shapes:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.2 0.4 0.6"
)yaml");

  scene::Scene data;
  data.load(path, "target");

  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[3], 0.2F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[4], 0.4F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[5], 0.6F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[15], 0.2F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[16], 0.4F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[17], 0.6F);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
}

TEST(LoadScene, DoesNotDrawSceneShapesDirectly)
{
  const std::filesystem::path path = writeYaml("does_not_draw_scene_shapes", R"yaml(
scene:
  shapes:
    - shape_01
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
      type: "i X Y Z"
      data: |
        1 0.0 0.0 0.0
        2 1.0 0.0 0.0
        3 0.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
shapes:
  shape_01:
    mesh:
      ref: "#first"
    material:
      type: "solid"
)yaml");

  EXPECT_THROW(
      {
        scene::Scene data;
        data.load(path);
      },
      std::runtime_error);
}

TEST(LoadScene, LoadsSelectedShapeInstanceGroupsWithOffsets)
{
  const std::filesystem::path path = writeYaml("loads_shape_instance_groups", R"yaml(
scene:
  shapes:
    - ignored_direct_shape
  shape-instance-groups:
    - selected_group
  camera: "main"
  sun:
    force: "2.5"
    direction: "0 3 4"
    color: "0.7 0.8 0.9"
  params:
    background-color: "0.11 0.22 0.33"
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
      forwardRotateDegPSec: "12.5"
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
shapes:
  ignored_direct_shape:
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
shape-instance-groups:
  selected_group:
    shape:
      ref: "#target"
    offsets:
      type: "i X Y Z"
      data: |
        0 10 20 30
        1 -1 -2 -3
  ignored_group:
    shape:
      ref: "#target"
    offsets:
      type: "i X Y Z"
      data: |
        0 100 200 300
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.instances.size(), 2U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[0], 0.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[6], 1.0F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[12], 0.0F);
  EXPECT_FLOAT_EQ(data.instances[0].offset[0], 10.0F);
  EXPECT_FLOAT_EQ(data.instances[0].offset[1], 20.0F);
  EXPECT_FLOAT_EQ(data.instances[0].offset[2], 30.0F);
  EXPECT_FLOAT_EQ(data.instances[1].offset[0], -1.0F);
  EXPECT_FLOAT_EQ(data.instances[1].offset[1], -2.0F);
  EXPECT_FLOAT_EQ(data.instances[1].offset[2], -3.0F);
  EXPECT_EQ(data.instances[0].shapeIndex, 0U);
  EXPECT_EQ(data.instances[1].shapeIndex, 0U);
  EXPECT_EQ(data.shapes[0].firstInstance, 0U);
  EXPECT_EQ(data.shapes[0].instanceCount, 2U);
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
  EXPECT_FLOAT_EQ(data.camera.forwardRotateDegPSec, 12.5F);
  EXPECT_FLOAT_EQ(data.sun.force, 2.5F);
  EXPECT_FLOAT_EQ(data.sun.direction[0], 0.0F);
  EXPECT_FLOAT_EQ(data.sun.direction[1], 0.6F);
  EXPECT_FLOAT_EQ(data.sun.direction[2], 0.8F);
  EXPECT_FLOAT_EQ(data.sun.color[0], 0.7F);
  EXPECT_FLOAT_EQ(data.sun.color[1], 0.8F);
  EXPECT_FLOAT_EQ(data.sun.color[2], 0.9F);
  EXPECT_FLOAT_EQ(data.params.backgroundColor[0], 0.11F);
  EXPECT_FLOAT_EQ(data.params.backgroundColor[1], 0.22F);
  EXPECT_FLOAT_EQ(data.params.backgroundColor[2], 0.33F);
}

TEST(LoadScene, ThrowsWhenSunDirectionIsZero)
{
  const std::filesystem::path path = writeYaml("zero_sun_direction", R"yaml(
scene:
  camera: "main"
  sun:
    force: "1"
    direction: "0 0 0"
    color: "1 1 1"
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
      sideVelocity: "1"
      forwardMouseSensitivity: "0.1"
      forwardScrollStep: "1"
)yaml");

  expectRuntimeErrorContains(path, " :: YAML vector 'scene.sun.direction' must not be zero");
}

TEST(LoadScene, StoresOnlyShapesUsedBySelectedInstanceGroups)
{
  const std::filesystem::path path = writeYaml("stores_only_used_shapes", R"yaml(
scene:
  shape-instance-groups:
    - first_group
    - second_group
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
      sideVelocity: "1"
      forwardMouseSensitivity: "0.1"
      forwardScrollStep: "1"
meshes:
  first_mesh:
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
  second_mesh:
    points:
      type: "i X Y Z"
      data: |
        1 2.0 0.0 0.0
        2 3.0 0.0 0.0
        3 2.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
  unused_mesh:
    points:
      type: "i X Y Z"
      data: |
        1 9.0 0.0 0.0
        2 9.0 1.0 0.0
        3 9.0 0.0 1.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
shapes:
  first:
    mesh:
      ref: "#first_mesh"
    material:
      type: "solid"
      color: "1 0 0"
  second:
    mesh:
      ref: "#second_mesh"
    material:
      type: "solid"
      color: "0 1 0"
  unused:
    mesh:
      ref: "#unused_mesh"
    material:
      type: "solid"
      color: "0 0 1"
shape-instance-groups:
  first_group:
    shape:
      ref: "#first"
    offsets:
      type: "i X Y Z"
      data: |
        0 10 0 0
        1 11 0 0
  second_group:
    shape:
      ref: "#second"
    offsets:
      type: "i X Y Z"
      data: |
        0 20 0 0
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.shapes.size(), 2U);
  ASSERT_EQ(data.instances.size(), 3U);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[0], 0.0F);
  EXPECT_FLOAT_EQ(data.shapes[1].vertices[0], 2.0F);
  EXPECT_EQ(data.shapes[0].firstInstance, 0U);
  EXPECT_EQ(data.shapes[0].instanceCount, 2U);
  EXPECT_EQ(data.shapes[1].firstInstance, 2U);
  EXPECT_EQ(data.shapes[1].instanceCount, 1U);
  EXPECT_EQ(data.instances[0].shapeIndex, 0U);
  EXPECT_EQ(data.instances[1].shapeIndex, 0U);
  EXPECT_EQ(data.instances[2].shapeIndex, 1U);
}

TEST(LoadScene, ResolvesMeshReferencesFromRelativeFiles)
{
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
shapes:
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
  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[3], 0.1F);
}

TEST(LoadScene, ReadsPointAndIndexDataRefs)
{
  const std::filesystem::path pointsPath  = writeTextFile("probe_open_gl_points_data_ref.txt", R"txt(
1 0.0 0.0 0.0
2 1.0 0.0 0.0
3 0.0 1.0 0.0
)txt");
  const std::filesystem::path indexesPath = writeTextFile("probe_open_gl_indexes_data_ref.txt", R"txt(
1 2 3
)txt");
  const std::filesystem::path path        = writeYaml("data_refs", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data-ref: "probe_open_gl_points_data_ref.txt"
    indexes:
      type: "i i i"
      data-ref: "probe_open_gl_indexes_data_ref.txt"
shapes:
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
  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[3], 0.1F);
}

TEST(LoadScene, IgnoresEmptyLinesAndCommentsInInlineData)
{
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
shapes:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.7 0.8 0.9"
)yaml");

  scene::Scene data;
  data.load(path, "target");

  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[3], 0.7F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[4], 0.8F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[5], 0.9F);
}

TEST(LoadScene, IgnoresEmptyLinesAndCommentsInDataRefs)
{
  const std::filesystem::path pointsPath  = writeTextFile("probe_open_gl_points_comments_ref.txt", R"txt(
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
  const std::filesystem::path path        = writeYaml("data_refs_empty_lines_and_comments", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data-ref: "probe_open_gl_points_comments_ref.txt"
    indexes:
      type: "i i i"
      data-ref: "probe_open_gl_indexes_comments_ref.txt"
shapes:
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
  ASSERT_EQ(data.shapes.size(), 1U);
  ASSERT_EQ(data.shapes[0].vertices.size(), 18U);
  EXPECT_EQ(data.shapes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[3], 0.4F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[4], 0.5F);
  EXPECT_FLOAT_EQ(data.shapes[0].vertices[5], 0.6F);
}

TEST(LoadScene, ThrowsWhenSectionHasDataAndDataRef)
{
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
shapes:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.1 0.2 0.3"
)yaml");

  EXPECT_THROW(
      {
        scene::Scene data;
        data.load(path, "target");
      },
      std::runtime_error);
}

TEST(LoadScene, ThrowsWhenSectionHasNeitherDataNorDataRef)
{
  const std::filesystem::path path = writeYaml("no_data_or_data_ref", R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
    indexes:
      type: "i i i"
      data: |
        1 2 3
shapes:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
      color: "0.1 0.2 0.3"
)yaml");

  EXPECT_THROW(
      {
        scene::Scene data;
        data.load(path, "target");
      },
      std::runtime_error);
}

TEST(LoadScene, ThrowsWhenMeshIsMissing)
{
  const std::filesystem::path path = writeYaml("missing_mesh", R"yaml(
meshes:
  existing:
    points:
      type: "i X Y Z"
      data: |
        1 0 0 0
    indexes:
      type: "i i i"
      data: |
        1 1 1
shapes:
  existing:
    mesh:
      ref: "#existing"
    material:
      type: "solid"
)yaml");

  EXPECT_THROW(
      {
        scene::Scene data;
        data.load(path, "missing");
      },
      std::runtime_error);
}

TEST(LoadScene, ThrowsWhenPointRowDoesNotMatchType)
{
  const std::filesystem::path path = writeYaml("invalid_point_row", R"yaml(
meshes:
  bad:
    points:
      type: "i X Y Z"
      data: |
        1 0 0 0 1
    indexes:
      type: "i i i"
      data: |
        1 1 1
shapes:
  bad:
    mesh:
      ref: "#bad"
    material:
      type: "solid"
)yaml");

  EXPECT_THROW(
      {
        scene::Scene data;
        data.load(path, "bad");
      },
      std::runtime_error);
}
