#version 330 core

// The color of the line
uniform vec4 u_color;

out vec4 FragColor;

void main()
{
    FragColor = u_color;
}