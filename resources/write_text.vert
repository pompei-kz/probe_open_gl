#version 460 core

layout (location = 0) in vec4 vertex;

uniform mat4 projectionMatrix;

out vec2 textureCoordinates;

void main() {
  gl_Position = projectionMatrix * vec4(vertex.xy, 0.0, 1.0);
  textureCoordinates = vertex.zw;
}
