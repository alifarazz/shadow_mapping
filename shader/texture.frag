#version 330 core

out vec4 color;

// input from vertex shader
// in vec3 FragColor;
in vec2 TexCoord;

// uniform textures
struct Material {
  sampler2D texture_specular1;
  sampler2D texture_specular2;
  sampler2D texture_diffuse1;
  sampler2D texture_diffuse2;
};
uniform Material material;

vec3 directional = normalize(vec3(1,1,1));

void main()
{
  // if (sin(FragColor.x * 100) + cos(FragColor.y * 100) < 0.5)
  //   color = vec4(0.4, 0.2, 0.5, 1.0);
  // else
  //   color = vec4(1.0, 1.0, 1.0, 1.0);

  // color = vec4(FragColor, 1.0);
  color = texture(material.texture_diffuse1, TexCoord);
}
