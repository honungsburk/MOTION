#version 430 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform sampler2D trailTexture;

uniform vec4 u_clearColor;
uniform float u_trail_mix;

uniform bool u_loop_record_mode;
uniform bool u_first_frame;

vec4 safe_mix(vec4 target, vec4 current, float rate){
    vec4 new_color = (1.0 - rate) * target + current * rate;
    vec4 diff = new_color - current; // Shoul always be in the range -1 to 1
    vec4 step = max(abs(diff), 1.0/255.0);
    return current + sign(diff) * step;
}

void main()
{ 
    vec4 trail = texture(trailTexture, TexCoords);
    vec4 particle = texture(screenTexture, TexCoords);

    if (particle == u_clearColor) {
        // In loop record mode we want to use the alpha channel
        // but using the window the alpha channel can't be used
        if(u_loop_record_mode) {
            color = vec4(trail.xyz, trail.w * u_trail_mix);
        } else if(u_first_frame) {
            color = u_clearColor;
        } else {
            color = safe_mix(u_clearColor, trail, u_trail_mix);
        }
    } else {
        color = particle;
    }

}