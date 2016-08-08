// workshop01: simple OpenGL particle system

#include <algorithm>	// for std::min, std::max
#include <cmath>
#include <cstdio>
#include <string>

// Windows-specific: prevent Windows headers from defining extra stuff we don't need or want
#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <sys/types.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static const float two_pi = 6.283185308f;

// Definition of the data for a single vertex for our particle system
struct particle_vertex
{
	float position[2];		// position of vertex, as an offset from the center of the particle
};

// And a definition for our screen space quad vertices
struct quad_vertex
{
	float screen_position[2];
};

// Definition of the data for a single particle
struct particle_data
{
	float position[2];		// current position of the particle's center in world space
	float velocity[2];		// velocity vector
	float angle;			// current rotation angle
	float spin;				// how fast it's rotating
	float size;				// particle size
	float creation_time;	// when the particle was created
};

// Definition of the "uniform" data that will be passed to the shaders...
// basically global variables for shaders
struct uniform_data
{
	float window_size[2];		// window size in world space
	float window_center[2];		// window center in world space
	float light_dir[3];			// direction of a moving light source
	float time;					// current simulation time in seconds
};

// Global variables
static const int	num_particles = 1000;
particle_data		particles[num_particles] = {};
int					num_vertices_per_particle = 0;

GLFWwindow*			window = nullptr;
GLuint				quad_vertex_buffer = 0;
GLuint				vertex_buffer = 0;
GLuint				particle_buffer = 0;
GLuint				uniform_buffer = 0;

GLuint				particle_shader_program = 0;
GLuint				raytrace_shader_program = 0;
time_t				vertex_shader_mtime = 0;
time_t				fragment_shader_mtime = 0;
time_t				quad_vertex_shader_mtime = 0;
time_t				raytrace_shader_mtime = 0;

bool raytrace_mode = false;

// Pre-declare functions we'll use later
void init_graphics();
void render_frame();
void generate_particles(float timestep);
void simulate_particles(float timestep);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_resize_callback(GLFWwindow* window, int width, int height);
void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data);
void load_all_shaders();
void load_shaders(const char* vertex_shader_file, const char* fragment_shader_file, time_t* vertex_shader_time_ptr, time_t* fragment_shader_time_ptr, GLuint* out_program);
void reload_shaders_if_changed();



// Everything starts here!
int main (int, const char **)
{
	printf("Starting up!\n");

	// Initialize the library
	if (!glfwInit())
	{
		printf("Error: couldn't initialize GLFW :(\n");
		return -1;
	}

	// Tell GLFW we want debugging support enabled
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// Tell GLFW we want at least an OpenGL 4.1 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(1280, 720, "OpenGL Particle System", NULL, NULL);
	if (!window)
	{
		printf("Error: couldn't create window with GLFW :(\n");
		glfwTerminate();
		return -1;
	}

	// Set up event-handling callbacks
	glfwSetKeyCallback(window, &key_callback);
	glfwSetWindowSizeCallback(window, &window_resize_callback);

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Now that we have a context, load all OpenGL functions
	if (!gladLoadGL())
	{
		printf("Error: couldn't load OpenGL functions :(\n");
		return -1;
	}
	printf("Got OpenGL version %d.%d\n", GLVersion.major, GLVersion.minor);

	// Initialize all our graphics resources such as buffers, shaders, etc
	init_graphics();

	// Loop until the user closes the window
	double prev_time = 0.0;
	double prev_shader_load_time = 0.0;
	while (!glfwWindowShouldClose(window))
	{
		double cur_time = glfwGetTime();
		float timestep = float(cur_time - prev_time);
		prev_time = cur_time;

		// Generate new particles
		generate_particles(timestep);

		// Simulate particles' forward in time using physics
		simulate_particles(timestep);

		// Check shaders for modifications every 0.5 second to allow live editing
		if (cur_time > prev_shader_load_time + 0.5)
		{
			reload_shaders_if_changed();
			prev_shader_load_time = cur_time;
		}

		// Render a new frame
		render_frame();

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	printf("Shutting down!\n");
	glfwTerminate();
	return 0;
}

void init_graphics()
{
	// Configure OpenGL debug messages (assuming it's supported)
	if (GLAD_GL_KHR_debug)
	{
		glDebugMessageCallback(&debug_message_callback, nullptr);
	}
	else
	{
		printf("Warning: OpenGL debug messages not available!\n");
	}

	// Load the vertex and fragment shaders
	load_all_shaders();

	// Set up various buffers that we'll pass to the shaders running on the GPU.
	// 1. The vertex buffer will define the shape of an individual particle.
	// 2. The particle buffer defines the positions and other properties of the particles.
	// 3. The uniform buffer is a set of global variables accessible to all particles' shaders.

	// Generate some vertices. Use math to generate a star shape made out of triangles, just for fun!
	static const int star_points = 5;
	static const int num_vertices = 6 * star_points;
	particle_vertex vertices[num_vertices] = {};
	for (int i = 0; i < star_points; ++i)
	{
		float angle_left   = two_pi * float(2*i + 1) / float(2*star_points);
		float angle_middle = two_pi * float(i) / float(star_points);
		float angle_right  = two_pi * float(2*i - 1) / float(2*star_points);

		static const float inner_radius = 0.5f;
		static const float outer_radius = 1.0f;
		vertices[6*i + 0] = particle_vertex{0.0f, 0.0f};
		vertices[6*i + 1] = particle_vertex{-sin(angle_right) * inner_radius, cos(angle_right) * inner_radius};
		vertices[6*i + 2] = particle_vertex{-sin(angle_middle) * outer_radius, cos(angle_middle) * outer_radius};
		vertices[6*i + 3] = vertices[6*i + 0];
		vertices[6*i + 4] = vertices[6*i + 2];
		vertices[6*i + 5] = particle_vertex{-sin(angle_left) * inner_radius, cos(angle_left) * inner_radius};
	}

	// Generate two triangles that make up a screen-space quad
	quad_vertex quad_vertices[6] = 
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f,
	};

	// Record how many vertices we created in a global variable,
	// so later code will know how many vertices to draw.
	num_vertices_per_particle = num_vertices;

	// Upload this buffer to the GPU, where we'll re-use it each time we draw.
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Screen space quad buffer
	glGenBuffers(1, &quad_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	// Now create the particle buffer. We're going to update this each frame, so we won't put any data in it yet.
	glGenBuffers(1, &particle_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particles), nullptr, GL_DYNAMIC_DRAW);

	// Now create the uniform buffer. Again, this will be updated each frame.
	glGenBuffers(1, &uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniform_data), nullptr, GL_DYNAMIC_DRAW);

	// Create a dummy VAO, since one is required by the OpenGL core profile.
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

void render_frame()
{
	// Set the rendering viewport to match the current size of the framebuffer
	// Note that framebuffer size may differ from "window size" due to DPI shenanigans.
	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
	glViewport(0, 0, framebuffer_width, framebuffer_height);

	// Get time
	float time = float(glfwGetTime());

	// Calculate a moving light source
	float light_dir[3] = { cos(time) * 0.7f, 0.5f, sin(time) * 0.7f };

	// Calculate the uniform buffer parameters
	static const float world_size = 30.0f;		// how many world units across should we be able to see in the window
	float pixels_to_world_scale = world_size / float(std::min(framebuffer_width, framebuffer_height));
	uniform_data uniforms =
	{
		pixels_to_world_scale * float(framebuffer_width),	// window_size.x
		pixels_to_world_scale * float(framebuffer_height),	// window_size.y
		0.0f, 0.4f * world_size,							// window_center
		light_dir[0], light_dir[1], light_dir[2],			// light_dir
		time,												// time
	};

	// Send this frame's uniform data to the GPU. Using INVALIDATE_BUFFER_BIT means that
	// any old data in the buffer is no longer relevant and can be discarded. This allows the
	// GPU and driver to optimize the memory access under the hood.
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);	
	if (void* mapped_buffer = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms), GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT))
	{
		memcpy(mapped_buffer, &uniforms, sizeof(uniforms));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	else
	{
		printf("Warning: couldn't map uniform buffer!\n");
	}

	// Send this frame's particle data to the GPU.
	glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
	if (void* mapped_buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(particles), GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT))
	{
		memcpy(mapped_buffer, &particles, sizeof(particles));
		glUnmapBuffer(GL_ARRAY_BUFFER);
		mapped_buffer = nullptr;
	}
	else
	{
		printf("Warning: couldn't map particle buffer!\n");
	}

	// Render a nice sky blue background
	glClearColor(0.0f, 0.6f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set up the uniform buffer to be loaded by the shaders
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniform_buffer, 0, sizeof(uniform_data));
	
	if (!raytrace_mode)
	{
		// Rasterized scene

		// Set up vertex attributes to be loaded from the vertex buffer by the GPU
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(particle_vertex), (const void *)offsetof(particle_vertex, position));

		// Set up vertex attributes to be loaded from the particle buffer by the GPU
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(particle_data), (const void *)offsetof(particle_data, position));
		glVertexAttribDivisor(1, 1);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(particle_data), (const void *)offsetof(particle_data, velocity));
		glVertexAttribDivisor(2, 1);

		// Angle, spin, size, creation_time all packed into a single vec4 attribute, to save space
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(particle_data), (const void *)offsetof(particle_data, angle));
		glVertexAttribDivisor(3, 1);

		// Draw the particles
		glUseProgram(particle_shader_program);
		glDrawArraysInstanced(GL_TRIANGLES, 0, num_vertices_per_particle, num_particles);
	}
	else
	{
		// Raytraced scene
		// Set up vertex attributes to be loaded from the quad buffer by the GPU
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(quad_vertex), (const void *)offsetof(particle_vertex, position));

		// Draw only a screenspace quad.  Fragment shader does the rest!
		glUseProgram(raytrace_shader_program);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

float random_in_range(float min, float max)
{
	// Xorshift random number algorithm invented by George Marsaglia
	static uint32_t rng_state = 0xf2eec0de;
	rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
	float random_0_to_1 = float(rng_state) * (1.0f/4294967296.0f);
	return min + (max - min) * random_0_to_1;
}

void generate_particles(float timestep)
{
	static int next_particle_index = 0;
	static float particle_generation_accumulator = 0.0f;

	// Calculate how many particles to generate based on a time emission rate
	static const float particles_per_second = 50.0f;
	particle_generation_accumulator += particles_per_second * timestep;
	int particles_to_generate = int(floor(particle_generation_accumulator));
	particle_generation_accumulator -= particles_to_generate;

	float time = float(glfwGetTime());

	// Generate the particles by writing into the particles data array
	for (int i = 0; i < particles_to_generate; ++i)
	{
		// Set up a particle with random starting values
		particles[next_particle_index] = particle_data
		{
			0.0f, 0.0f,								// position
			random_in_range(-12.0f, 12.0f),			// velocity.x
			random_in_range(24.0f, 48.0f),			// velocity.y
			random_in_range(0.0f, two_pi),			// angle
			random_in_range(-5.0f, 5.0f),			// spin
			exp2(random_in_range(-2.0f, 0.5f)),		// size
			time,									// creation_time
		};

		// Increment to the next particle, wrapping around to the beginning
		// of the buffer once we've gone through the whole thing.
		next_particle_index = (next_particle_index + 1) % num_particles;
	}
}

void simulate_particles(float timestep)
{
	static const float gravity = -40.0f;

	for (int i = 0; i < num_particles; ++i)
	{
		// Update position using the velocity vector
		particles[i].position[0] += timestep * particles[i].velocity[0];
		particles[i].position[1] += timestep * particles[i].velocity[1];

		// Update velocity using gravity
		particles[i].velocity[1] += timestep * gravity;

		// Update angle using the spin speed, but keep it within [-two_pi, two_pi]
		particles[i].angle = fmod(particles[i].angle + timestep * particles[i].spin, two_pi);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Close the window when the user presses Escape
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		raytrace_mode = !raytrace_mode;
	}
}

void window_resize_callback(GLFWwindow* window, int width, int height)
{
	// Re-render the scene so that it responds continuously while user is resizing.
	// (Ordinarily, GLFW doesn't resume rendering until the resize is finished.)
	render_frame();
	glfwSwapBuffers(window);
}

void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
{
	// Skip "notification"-level messages as they tend to be spammy, and don't indicate problems.
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	// If we get any debug messages from OpenGL, print them out to the terminal
	printf("[GL] %s\n", msg);
}



// Infrastructure for shader loading and compilation

void print_shader_info_log(GLuint shader, const char* filename)
{
	int info_log_length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

	// If the info log is empty, don't print anything.
	if (!info_log_length)
		return;

	// Allocate enough memory in a string to hold the info log.
	std::string info_log;
	info_log.resize(info_log_length);

	// Read the info log into the string.
	glGetShaderInfoLog(shader, info_log_length, nullptr, &info_log[0]);

	printf(
		"----- Info log for: %s -----\n"
		"%s"
		"----------------------------------------------\n",
		filename,
		info_log.c_str());
}

GLuint try_load_shader(GLenum shader_type, const char* filename, time_t* o_mtime)
{
	// Try to find the shader file. It could be at different relative
	// paths depending on which directory we started the app from.
	struct stat file_stat = {};
	FILE * file = nullptr;
	if (stat(filename, &file_stat) == 0)
	{
		file = fopen(filename, "rb");
	}
	else
	{
		std::string prefixed_filename = "../";
		prefixed_filename += filename;
		stat(prefixed_filename.c_str(), &file_stat);
		file = fopen(prefixed_filename.c_str(), "rb");
	}
	
	if (!file)
	{
		printf("Warning: couldn't find shader source file %s!\n", filename);
		return 0;
	}

	// Store the file's modification time for later use
	*o_mtime = file_stat.st_mtime;

	// Allocate enough memory in a string to hold the shader file.
	fseek(file, 0, SEEK_END);
	int file_size = int(ftell(file));
	std::string shader_source;
	shader_source.resize(file_size);

	// Read the file into memory
	fseek(file, 0, SEEK_SET);
	fread(&shader_source[0], file_size, 1, file);
	fclose(file);

	// Got the source code, now try to compile it.
	GLuint shader = glCreateShader(shader_type);
	const char* source_pointer = shader_source.c_str();
	int source_length = int(shader_source.size());
	glShaderSource(shader, 1, &source_pointer, &source_length);
	glCompileShader(shader);

	// Print the shader info log (we always do this, even if compilation succeeded,
	// in order to display any warnings that may have been generated).
	print_shader_info_log(shader, filename);

	// Check for compilation errors
	int compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		printf("Warning: %s did not compile!\n", filename);
		glDeleteShader(shader);
		return 0;
	}

	printf("%s compiled successfully!\n", filename);
	return shader;
}

void print_program_info_log(GLuint program)
{
	int info_log_length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

	// If the info log is empty, don't print anything.
	if (!info_log_length)
		return;

	// Allocate enough memory in a string to hold the info log.
	std::string info_log;
	info_log.resize(info_log_length);

	// Read the info log into the string.
	glGetProgramInfoLog(program, info_log_length, nullptr, &info_log[0]);

	printf(
		"----- Info log for shader linking -----\n"
		"%s"
		"---------------------------------------\n",
		info_log.c_str());
}

void load_all_shaders()
{
	load_shaders("vertex_shader.glsl", "fragment_shader.glsl", &vertex_shader_mtime, &fragment_shader_mtime, &particle_shader_program);
	load_shaders("vertex_shader_quad.glsl", "fragment_shader_raytrace.glsl", &quad_vertex_shader_mtime, &raytrace_shader_mtime, &raytrace_shader_program);
}

void load_shaders(const char* vertex_shader_file, const char* fragment_shader_file, time_t* vertex_shader_time_ptr, time_t* fragment_shader_time_ptr, GLuint* out_program )
{
	// Try to load and compile the individual shaders
	GLuint vertex_shader = try_load_shader(GL_VERTEX_SHADER, vertex_shader_file, vertex_shader_time_ptr);
	GLuint fragment_shader = try_load_shader(GL_FRAGMENT_SHADER, fragment_shader_file, fragment_shader_time_ptr);
	if (!vertex_shader || !fragment_shader)
	{
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return;
	}

	// Link all the shaders together into a program object
	GLuint new_program = glCreateProgram();
	glAttachShader(new_program, vertex_shader);
	glAttachShader(new_program, fragment_shader);
	glLinkProgram(new_program);

	// The individual shader objects are no longer needed now that the program is linked
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Print the program info log (we always do this, even if linking succeeded,
	// in order to display any warnings that may have been generated).
	print_program_info_log(new_program);

	// Check for linking errors
	int linked = 0;
	glGetProgramiv(new_program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		printf("Warning: shaders did not link!\n");
		glDeleteProgram(new_program);
		new_program = 0;
		return;
	}

	printf("Shaders linked successfully!\n");

	// Set up uniform block binding (OpenGL 4.1 doesn't support explicit bindings in the shader)
	GLuint uniform_block_index = glGetUniformBlockIndex(new_program, "uniform_data");
	if (uniform_block_index != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(new_program, uniform_block_index, 0);
	}

	// Replace the old program
	glDeleteProgram(*out_program);
	*out_program = new_program;
}

bool check_shader_changed(const char* filename, time_t prev_mtime)
{
	// Try to find the shader file. It could be at different relative
	// paths depending on which directory we started the app from.
	struct stat file_stat = {};
	if (stat(filename, &file_stat) != 0)
	{
		std::string prefixed_filename = "../";
		prefixed_filename += filename;
		if (stat(prefixed_filename.c_str(), &file_stat) != 0)
		{
			// Couldn't find the file - treat it as unmodified
			return false;
		}
	}

	return (file_stat.st_mtime > prev_mtime);
}

void reload_shaders_if_changed()
{
	if (check_shader_changed("vertex_shader.glsl", vertex_shader_mtime) ||
		check_shader_changed("fragment_shader.glsl", fragment_shader_mtime) ||
		check_shader_changed("vertex_shader_quad.glsl", quad_vertex_shader_mtime) ||
		check_shader_changed("fragment_shader_raytrace.glsl", raytrace_shader_mtime))
	{
		printf("Shader source files updated; recompiling\n");
		load_all_shaders();
	}
}
