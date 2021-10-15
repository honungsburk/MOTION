#version 430 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform sampler2D trailTexture;

uniform vec4 u_clearColor;
uniform float u_trail_mix;

void main()
{ 
    vec4 trail = texture(trailTexture, TexCoords);
    vec4 particle = texture(screenTexture, TexCoords);

    if (particle == u_clearColor) {
        color = mix(particle, trail, u_trail_mix);
    } else {
        color = particle;
    }
}