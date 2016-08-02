// Vertex shader for simple particle system
#version 410

// Input data from vertex buffer
layout(location = 0) in vec2 vertex_position;

void main()
{
	// Set the output position in screen space
	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
