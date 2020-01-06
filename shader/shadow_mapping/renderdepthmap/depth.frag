#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMapTexture;

void main()
{
  float depthValue = texture(depthMapTexture, TexCoords).r;
  FragColor = vec4(vec3(depthValue), 1.0f);
  // FragColor = vec4(1,1,1,1);
}
