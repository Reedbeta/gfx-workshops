// Vertex shader for simple particle system
#version 410

// Uniforms passed from main app: this matches struct uniform_data in the C++ code
layout(std140) uniform uniform_data
{
	vec2 window_size;		// window size in world space
	vec2 window_center;		// window center in world space	
};

// Input data from vertex buffer
layout(location = 0) in vec2 vertex_position;

// Input data from particle data buffer
layout(location = 1) in vec2 particle_position;
layout(location = 2) in vec2 particle_velocity;
layout(location = 3) in vec4 particle_angle_spin_size_age;	// Four values packed together in a vec4

void main()
{
	float particle_angle = particle_angle_spin_size_age.x;
	float particle_spin  = particle_angle_spin_size_age.y;
	float particle_size  = particle_angle_spin_size_age.z;
	float particle_age   = particle_angle_spin_size_age.w;

	// Calculate the world-space position of the vertex, by applying the vertex's local offset to
	// the particle's position, and applying the particle's rotation and size.
	float sin_angle = sin(particle_angle);
	float cos_angle = cos(particle_angle);
	mat2 particle_transform = mat2(cos_angle, sin_angle, -sin_angle, cos_angle) * particle_size;
	vec2 world_space_pos = particle_position + particle_transform * vertex_position;
	
	// Use the window size to scale the vertex position from world space to [-1, 1] screen space
	vec2 screen_space_pos = (world_space_pos - window_center) / (0.5 * window_size);
	gl_Position = vec4(screen_space_pos, 0.0, 1.0);
}
