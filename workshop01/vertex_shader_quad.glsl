// Vertex shader for simple particle system
#version 410

// Uniforms passed from main app: this matches struct uniform_data in the C++ code
layout(std140) uniform uniform_data
{
	vec2 window_size;		// window size in world space
	vec2 window_center;		// window center in world space	
    vec3 light_dir;         // direction of a light source
	float time;				// current simulation time in seconds
};

// Input data from vertex buffer
layout(location = 0) in vec2 vertex_position;

// Input data from particle data buffer
layout(location = 1) in vec2 particle_position;
layout(location = 2) in vec2 particle_velocity;
layout(location = 3) in vec4 particle_angle_spin_size_creationtime;	// Four values packed together in a vec4

// Output data to send to fragment shader
layout(location = 0) out vec2 o_vertex_position;

void main()
{
	gl_Position = vec4(vertex_position, 0.0, 1.0);
    o_vertex_position = vertex_position;
}
