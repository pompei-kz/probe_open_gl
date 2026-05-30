#include <epoxy/gl.h>
#include <gtest/gtest.h>

import scene;
import atom;

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

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

  std::string cameraYaml()
  {
    return R"yaml(
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
)yaml";
  }

  std::string triangleMeshYaml()
  {
    return R"yaml(
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
)yaml";
  }
} // namespace

TEST(LoadScene, TreatsMissingCamerasContainerAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_cameras_container", R"yaml(
scene:
  camera: "main"
)yaml");

  expectRuntimeErrorContains(path, " :: Missing YAML map 'main'");
}

TEST(LoadScene, TreatsMissingSceneShapeGroupsAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_scene_shape_groups", R"yaml(
scene:
  camera: "main"
)yaml" + cameraYaml());

  expectRuntimeErrorContains(path, " :: No drawable triangle data");
}

TEST(LoadScene, TreatsMissingShapeGroupsContainerAsEmpty)
{
  const std::filesystem::path path = writeYaml("missing_shape_groups_container", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml());

  expectRuntimeErrorContains(path, " :: Missing YAML map 'selected_group'");
}

TEST(LoadScene, LoadsShapeGroupsWithShaderMeshRefMaterialsAndOffsets)
{
  const std::filesystem::path path = writeYaml("loads_shape_groups", R"yaml(
scene:
  shape-groups:
    - selected_group
  camera: "main"
  world-shape-group: "selected_group"
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
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    materials:
      - index: 0
        color: "0.2 0.4 0.6"
        scale: "2.5"
        atom: "Oxygen"
      - index: 1
        color: "0.7 0.8 0.9"
        scale: "3.5"
        atom: "Nitrogen"
    offsets:
      type: "i X Y Z Mi"
      data: |
        0 10 20 30 0
        1 -1 -2 -3 1
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.meshes.size(), 1U);
  ASSERT_EQ(data.shapeGroups.size(), 1U);
  EXPECT_EQ(data.shapeGroups[0].shaderName, "triangle");
  EXPECT_EQ(data.shapeGroups[0].meshIndex, 0U);
  EXPECT_EQ(data.shapeGroups[0].firstInstance, 0U);
  EXPECT_EQ(data.shapeGroups[0].instanceCount, 2U);
  ASSERT_EQ(data.shapeGroups[0].shapes.size(), 2U);
  EXPECT_EQ(data.worldShapeGroup, "selected_group");
  EXPECT_EQ(data.meshes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  ASSERT_EQ(data.meshes[0].vertices.size(), 9U);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[0], 0.0F);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[3], 1.0F);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[4], 0.0F);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[5], 0.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[0], 10.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[1], 20.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[2], 30.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[1].offset[0], -1.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[1].offset[1], -2.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[1].offset[2], -3.0F);
  EXPECT_EQ(data.shapeGroups[0].shapes[0].meshIndex, 0U);
  EXPECT_EQ(data.shapeGroups[0].shapes[1].meshIndex, 0U);

  ASSERT_EQ(data.materials.size(), 2U);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].color[0], 0.2F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].color[1], 0.4F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].color[2], 0.6F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].scale, 2.5F);
  EXPECT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].atom, atom::Oxygen);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[1].materialIndex].color[0], 0.7F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[1].materialIndex].color[1], 0.8F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[1].materialIndex].color[2], 0.9F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[1].materialIndex].scale, 3.5F);
  EXPECT_EQ(data.materials[data.shapeGroups[0].shapes[1].materialIndex].atom, atom::Nitrogen);

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

TEST(LoadScene, LoadsMeshRefFromExternalYamlFile)
{
  writeTextFile("probe_open_gl_external_meshes.yaml", triangleMeshYaml());
  const std::filesystem::path path = writeYaml("external_mesh_ref", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
  world-shape-group: "selected_group"
)yaml" + cameraYaml() + R"yaml(
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "probe_open_gl_external_meshes.yaml#triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.meshes.size(), 1U);
  ASSERT_EQ(data.shapeGroups.size(), 1U);
  ASSERT_EQ(data.shapeGroups[0].shapes.size(), 1U);
  EXPECT_EQ(data.meshes[0].indexes, (std::vector<GLuint>{0, 1, 2}));
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[0], 1.0F);
  EXPECT_FLOAT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].scale, 1.0F);
}

TEST(LoadScene, ReadsPointIndexAndOffsetDataRefs)
{
  writeTextFile("probe_open_gl_points.txt", R"txt(
1 0.0 0.0 0.0
2 1.0 0.0 0.0
3 0.0 1.0 0.0
)txt");
  writeTextFile("probe_open_gl_indexes.txt", R"txt(
1 2 3
)txt");
  writeTextFile("probe_open_gl_offsets.txt", R"txt(
0 4 5 6
)txt");
  const std::filesystem::path path = writeYaml("data_refs", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
  world-shape-group: "selected_group"
)yaml" + cameraYaml() + R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data-ref: "probe_open_gl_points.txt"
    indexes:
      type: "i i i"
      data-ref: "probe_open_gl_indexes.txt"
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z"
      data-ref: "probe_open_gl_offsets.txt"
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.meshes.size(), 1U);
  ASSERT_EQ(data.shapeGroups.size(), 1U);
  ASSERT_EQ(data.shapeGroups[0].shapes.size(), 1U);
  ASSERT_EQ(data.meshes[0].vertices.size(), 9U);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[3], 1.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[0], 4.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[1], 5.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[2], 6.0F);
}

TEST(LoadScene, IgnoresCommentsAndBlankLines)
{
  const std::filesystem::path path = writeYaml("comments_and_blank_lines", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
  world-shape-group: "selected_group"
)yaml" + cameraYaml() + R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data: |
        # comment
        1 0.0 0.0 0.0

        2 1.0 0.0 0.0 # inline comment
        3 0.0 1.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3 # inline comment
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z Mi"
      data: |
        # comment
        0 7 8 9 0
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.meshes.size(), 1U);
  ASSERT_EQ(data.shapeGroups.size(), 1U);
  ASSERT_EQ(data.shapeGroups[0].shapes.size(), 1U);
  ASSERT_EQ(data.meshes[0].vertices.size(), 9U);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[3], 1.0F);
  EXPECT_FLOAT_EQ(data.shapeGroups[0].shapes[0].offset[2], 9.0F);
}

TEST(LoadScene, StoresOnlySelectedShapeGroups)
{
  const std::filesystem::path path = writeYaml("selected_shape_groups_only", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - first_group
    - second_group
)yaml" + cameraYaml() + R"yaml(
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
shape-groups:
  first_group:
    shader: "triangle"
    mesh-ref: "#first_mesh"
    offsets:
      type: "i X Y Z"
      data: |
        0 10 0 0
        1 11 0 0
  second_group:
    shader: "triangle"
    mesh-ref: "#second_mesh"
    offsets:
      type: "i X Y Z"
      data: |
        0 20 0 0
  unused_group:
    shader: "triangle"
    mesh-ref: "#unused_mesh"
    offsets:
      type: "i X Y Z"
      data: |
        0 90 0 0
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.meshes.size(), 2U);
  ASSERT_EQ(data.shapeGroups.size(), 2U);
  EXPECT_FLOAT_EQ(data.meshes[0].vertices[0], 0.0F);
  EXPECT_FLOAT_EQ(data.meshes[1].vertices[0], 2.0F);
  EXPECT_EQ(data.shapeGroups[0].firstInstance, 0U);
  EXPECT_EQ(data.shapeGroups[0].instanceCount, 2U);
  EXPECT_EQ(data.shapeGroups[1].firstInstance, 2U);
  EXPECT_EQ(data.shapeGroups[1].instanceCount, 1U);
  ASSERT_EQ(data.shapeGroups[0].shapes.size(), 2U);
  ASSERT_EQ(data.shapeGroups[1].shapes.size(), 1U);
  EXPECT_EQ(data.shapeGroups[0].shapes[0].meshIndex, 0U);
  EXPECT_EQ(data.shapeGroups[1].shapes[0].meshIndex, 1U);
}

TEST(LoadScene, DoesNotReadLegacyRootShapesOrShapeInstanceGroups)
{
  const std::filesystem::path path = writeYaml("legacy_root_shapes_ignored", R"yaml(
scene:
  camera: "main"
  shape-instance-groups:
    - selected_group
)yaml" + cameraYaml() + triangleMeshYaml() + R"yaml(
shapes:
  target:
    mesh:
      ref: "#triangle"
    material:
      type: "solid"
shape-instance-groups:
  selected_group:
    shape:
      ref: "#target"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: No drawable triangle data");
}

TEST(LoadScene, ThrowsWhenShapeGroupHasNoShader)
{
  const std::filesystem::path path = writeYaml("missing_shader", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml() + triangleMeshYaml() + R"yaml(
shape-groups:
  selected_group:
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: Missing shader for shape group 'selected_group'");
}

TEST(LoadScene, ThrowsWhenShapeGroupHasNoMeshRef)
{
  const std::filesystem::path path = writeYaml("missing_mesh_ref", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml() + triangleMeshYaml() + R"yaml(
shape-groups:
  selected_group:
    shader: "triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: Missing mesh-ref for shape group 'selected_group'");
}

TEST(LoadScene, ThrowsWhenDataAndDataRefAreBothSet)
{
  const std::filesystem::path path = writeYaml("data_and_data_ref", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
  world-shape-group: "selected_group"
)yaml" + cameraYaml() + R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data: |
        1 0 0 0
      data-ref: "probe_open_gl_points.txt"
    indexes:
      type: "i i i"
      data: |
        1 2 3
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: YAML section 'points' must not define both data and data-ref");
}

TEST(LoadScene, ThrowsWhenDataAndDataRefAreMissing)
{
  const std::filesystem::path path = writeYaml("missing_data_and_data_ref", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
  world-shape-group: "selected_group"
)yaml" + cameraYaml() + R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
    indexes:
      type: "i i i"
      data: |
        1 2 3
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: YAML section 'points' must define data or data-ref");
}

TEST(LoadScene, ThrowsWhenMeshIsMissing)
{
  const std::filesystem::path path = writeYaml("missing_mesh", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml() + R"yaml(
meshes: {}
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: Missing YAML map 'triangle'");
}

TEST(LoadScene, ThrowsWhenPointRowHasWrongSize)
{
  const std::filesystem::path path = writeYaml("invalid_point_row", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml() + R"yaml(
meshes:
  triangle:
    points:
      type: "i X Y Z"
      data: |
        1 0.0 0.0
    indexes:
      type: "i i i"
      data: |
        1 2 3
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    offsets:
      type: "i X Y Z"
      data: |
        0 1 2 3
)yaml");

  expectRuntimeErrorContains(path, " :: Invalid point row");
}

TEST(LoadScene, ThrowsWhenOffsetMaterialIndexIsMissing)
{
  const std::filesystem::path path = writeYaml("missing_material_index", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml() + triangleMeshYaml() + R"yaml(
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    materials:
      - index: 1
        color: "0.2 0.4 0.6"
        atom: "Oxygen"
    offsets:
      type: "i X Y Z Mi"
      data: |
        0 1 2 3 0
)yaml");

  expectRuntimeErrorContains(path, " :: Missing material index 0 for shape group 'selected_group'");
}

TEST(LoadScene, TreatsUnknownAtomNameAsUnknown)
{
  const std::filesystem::path path = writeYaml("unknown_atom_name", R"yaml(
scene:
  camera: "main"
  shape-groups:
    - selected_group
)yaml" + cameraYaml() + triangleMeshYaml() + R"yaml(
shape-groups:
  selected_group:
    shader: "triangle"
    mesh-ref: "#triangle"
    materials:
      - index: 0
        color: "0.2 0.4 0.6"
        scale: "2.5"
        atom: "Helium"
    offsets:
      type: "i X Y Z Mi"
      data: |
        0 1 2 3 0
)yaml");

  scene::Scene data;
  data.load(path);

  ASSERT_EQ(data.shapeGroups.size(), 1U);
  ASSERT_EQ(data.shapeGroups[0].shapes.size(), 1U);
  EXPECT_EQ(data.materials[data.shapeGroups[0].shapes[0].materialIndex].atom, atom::Unknown);
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
)yaml" + cameraYaml());

  expectRuntimeErrorContains(path, " :: YAML vector 'scene.sun.direction' must not be zero");
}
