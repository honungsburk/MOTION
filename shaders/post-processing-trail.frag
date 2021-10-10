#version 430 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform sampler2D trailTexture;

uniform vec4 clearColor;

void main()
{ 
    vec4 trail = texture(trailTexture, TexCoords);
    vec4 particle = texture(screenTexture, TexCoords);
    if (particle == clearColor) {
        color = mix(particle, trail, 0.90);
    } else {
        color = particle;
    }
}