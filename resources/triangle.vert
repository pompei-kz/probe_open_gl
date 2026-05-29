#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
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
  vec4 worldPosition = modelMatrix * vec4(position * material.w, 1.0);
  worldPosition.xyz += instance;
  fragmentWorldPosition = worldPosition.xyz;
  vertexColor = color * material.rgb;
  gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
