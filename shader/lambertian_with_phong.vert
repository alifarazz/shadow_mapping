#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

// fragment shader
out vec3 FragColor;
out vec2 TexCoords;

// uniform mat4 mvp;
uniform mat4 modelToView;
uniform mat4 viewToWorld;
uniform mat4 worldToPrespective;

// out gl_PerVertex { vec4 gl_Position; };

void main()
{
  gl_Position = worldToPrespective * viewToWorld * modelToView * vec4(pos, 1.0);
  // gl_Position = vec4(pos, 1.0);
  // FragColor = vec3(.4,.4,1);
  FragColor = normal;
  TexCoords = texCoords;
}
