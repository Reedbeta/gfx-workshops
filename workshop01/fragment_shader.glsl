// Fragment shader for simple particle system
#version 410

// Uniforms passed from main app: this matches struct uniform_data in the C++ code
layout(std140) uniform uniform_data
{
	vec2 window_size;		// window size in world space
	vec2 window_center;		// window center in world space	
	float time;				// current simulation time in seconds
};

// Input data from vertex shader
layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 particle_position;
layout(location = 2) in vec2 particle_velocity;
layout(location = 3) in vec4 particle_angle_spin_size_creationtime;	// Four values packed together in a vec4

// Output data to the framebuffer
layout(location = 0) out vec4 o_color;

void main()
{
	// Set the output color to a nice golden yellow
	o_color = vec4(1.0, 0.79, 0.03, 1.0);
}
