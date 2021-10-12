#version 430 core

in vec4 particle_color;

layout(location = 0) out vec4 color;


void main()
{
    color = particle_color;
}