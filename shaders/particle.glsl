#version 430 core

struct Particle{
	vec2 pos;
};

layout(std430, binding = 1) buffer particleBuffer
{
	Particle particles[];
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

void main()
{
	uint i = gl_GlobalInvocationID.x;

    particles[i].pos += vec2(0.01, 0.0); //particles[i].pos + vec2(0.01, 0.0);
}