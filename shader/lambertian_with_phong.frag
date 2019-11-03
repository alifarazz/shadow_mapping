#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragColor;

uniform mat4 mvp;

// out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    // gl_Position = vec4(pos, 1.0);
    // FragColor = vec3(.4,.4,1);
    FragColor = normal;
}
