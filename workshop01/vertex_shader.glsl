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

void main()
{
	// Use the window size to scale the vertex position from world space to [-1, 1] screen space
	vec2 screen_space_pos = (vertex_position - window_center) / (0.5 * window_size);
	gl_Position = vec4(screen_space_pos, 0.0, 1.0);
}
