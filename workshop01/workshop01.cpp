// workshop01: simple OpenGL scene

#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Close the window when the user presses Escape
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main (int, const char **)
{
	printf("Starting up!\n");

    GLFWwindow* window;

	// Initialize the library
	if (!glfwInit())
		return -1;

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(1280, 720, "SushiGL", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Set up event-handling callbacks
	glfwSetKeyCallback(window, &key_callback);

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Now that we have a context, load all OpenGL functions
	gladLoadGL();

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		// Render a shifting color
		double time = glfwGetTime();
		glClearColor(
			float(sin(time))*0.5f+0.5f,
			float(cos(time))*0.5f+0.5f,
			1.0f,
			1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	printf("Shutting down!\n");
	glfwTerminate();
	return 0;
}
