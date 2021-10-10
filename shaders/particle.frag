#version 430 core

// The color of the line
uniform vec4 u_color;

layout(location = 0) out vec4 color;

void main()
{
    color = u_color;
}