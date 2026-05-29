#version 460 core

in vec2 textureCoordinates;
out vec4 fragmentColor;

uniform sampler2D textTexture;
uniform vec3 textColor;

void main() {
  float alpha = texture(textTexture, textureCoordinates).r;
  fragmentColor = vec4(textColor, alpha);
}
