#version 330 core

in vec3 FragColor;

out vec4 color;

void main()
{
  // if (sin(FragColor.x * 100) + cos(FragColor.y * 100) < 0.5)
  //   color = vec4(0.4, 0.2, 0.5, 1.0);
  // else
  //   color = vec4(1.0, 1.0, 1.0, 1.0);
    color = vec4(FragColor, 1.0);
}
