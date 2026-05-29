#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 instance;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 vertexColor;
out vec3 fragmentWorldPosition;

void main() {
  vec4 worldPosition = modelMatrix * vec4(position + instance, 1.0);
  fragmentWorldPosition = worldPosition.xyz;
  vertexColor = color;
  gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
