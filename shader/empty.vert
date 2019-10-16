#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

out vec3 FragColor;

uniform mat4 mvp;

// out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    FragColor = color;
}

 
