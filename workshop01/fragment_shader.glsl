// Fragment shader for simple particle system
#version 410

// Output data to the framebuffer
layout(location = 0) out vec4 color;

void main()
{
	// Set the output color to a nice yellow
	color = vec4(1.0, 0.79, 0.03, 1.0);
}
