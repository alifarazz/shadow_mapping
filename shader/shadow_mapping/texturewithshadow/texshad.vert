#version 330 core

layout(location = 0) in vec3 pos_modelspace;

uniform mat4 depthMVP;


void main() {
  gl_Position = depthMVP * vec4(pos_modelspace, 1);
}
