#version 460 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 instance;
layout (location = 3) in uint instanceMaterialIndex;

layout (std430, binding = 0) readonly buffer MaterialParams {
  vec4 materialParams[];
};

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 vertexColor;
out vec3 fragmentWorldPosition;

void main() {
  vec4 material = materialParams[instanceMaterialIndex];
  vec3 color = material.rgb;
  float scale = material.w;

  vec4 worldPosition = modelMatrix * vec4(position * scale, 1.0);

  worldPosition.xyz += instance;
  fragmentWorldPosition = worldPosition.xyz;
  vertexColor = color;
  gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
