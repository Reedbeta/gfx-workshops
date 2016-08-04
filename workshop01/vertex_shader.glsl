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
layout(location = 0) out vec2 v_vertex_position;
layout(location = 1) out vec2 v_particle_position;
layout(location = 2) out vec2 v_particle_velocity;
layout(location = 3) out vec4 v_particle_angle_spin_size_creationtime;

void main()
{
	float particle_angle = particle_angle_spin_size_creationtime.x;
	float particle_size  = particle_angle_spin_size_creationtime.z;

	// Calculate the world-space position of the vertex, by applying the vertex's local offset to
	// the particle's position, and applying the particle's rotation and size.
	float sin_angle = sin(particle_angle);
	float cos_angle = cos(particle_angle);
	mat2 particle_transform = mat2(cos_angle, sin_angle, -sin_angle, cos_angle) * particle_size;
	vec2 world_space_pos = particle_position + particle_transform * vertex_position;
	
	// Use the window size to scale the vertex position from world space to [-1, 1] screen space
	vec2 screen_space_pos = (world_space_pos - window_center) / (0.5 * window_size);
	gl_Position = vec4(screen_space_pos, 0.0, 1.0);

	// Pass through vertex attributes to fragment shader, so we can
	// do calculations based on these values there too, if we want.
	v_vertex_position = vertex_position;
	v_particle_position = particle_position;
	v_particle_velocity = particle_velocity;
	v_particle_angle_spin_size_creationtime = particle_angle_spin_size_creationtime;
}
