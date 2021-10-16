#version 430 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform sampler2D trailTexture;

uniform vec4 u_clearColor;
uniform float u_trail_mix;

uniform bool u_loop_record_mode;

void main()
{ 
    vec4 trail = texture(trailTexture, TexCoords);
    vec4 particle = texture(screenTexture, TexCoords);

    if (particle == u_clearColor) {
        // In loop record mode we want to use the alpha channel
        // but using the window the alpha channel can't be used
        if(u_loop_record_mode) {
            color = vec4(trail.xyz, trail.w * u_trail_mix);
        } else {
            color = mix(particle, trail, u_trail_mix);

        }
    } else {
        color = particle;
    }
}