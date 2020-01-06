#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 lightSpaceMatrix;
uniform mat4 lightProjection;


void main() {
  gl_Position = lightProjection * lightSpaceMatrix * vec4(pos, 1.0);
}
