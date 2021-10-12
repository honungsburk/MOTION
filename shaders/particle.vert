#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 color;

uniform mat4 model;

out vec4 particle_color;

void main()
{
    gl_Position = model * vec4(aPos, 0.0, 1.0);
    particle_color = color;
}