#version 330 core

in  vec3   vertexColor;
in  vec3   fragmentWorldPosition;
out vec4 fragmentColor;

uniform float sunForce;
uniform vec3 sunDirection;
uniform vec3 sunColor;

void main() {
  vec3 normal = normalize(cross(dFdx(fragmentWorldPosition), dFdy(fragmentWorldPosition)));
  float diffuse = max(abs(dot(normal, normalize(-sunDirection))), 0.0);
  vec3 color = vertexColor * (0.12 + diffuse * sunForce) * sunColor;
  fragmentColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
