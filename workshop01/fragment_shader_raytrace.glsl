// Fragment shader for simple particle system
#version 410

// Uniforms passed from main app: this matches struct uniform_data in the C++ code
layout(std140) uniform uniform_data
{
	vec2 window_size;		// window size in world space
	vec2 window_center;		// window center in world space	
    vec3 light_dir;         // direction of a light source
	float time;				// current simulation time in seconds
};

// Input data from vertex shader
layout(location = 0) in vec2 o_vertex_position;

// Output data to the framebuffer
layout(location = 0) out vec4 o_color;

// Test a ray against a sphere and output hit info
bool ray_sphere_intersect (in vec3 raydir, in vec3 rayorig, in vec3 pos,
    in float rad, out vec3 hitpoint, out float distance, out vec3 normal)
{
    float a = dot(raydir, raydir);
    float b = dot(raydir, (2.0f * ( rayorig - pos)));
    float c = dot(pos, pos) + dot(rayorig, rayorig) -2.0f*dot(rayorig, pos) - rad*rad;
    float D = b*b + (-4.0f)*a*c;

    // If ray can not intersect then stop
    if (D < 0)
        return false;
    D=sqrt(D);

    // Ray can intersect the sphere, solve the closer hitpoint
    float t = (-0.5f)*(b+D)/a;
    if (t > 0.0f)
    {
        distance = sqrt(a)*t;
        hitpoint = rayorig + t*raydir;
        normal = (hitpoint - pos) / rad;
    }
    else
        return false;
    return true;
}

// Gamma correction for proper linear lighting
vec3 gamma_correct(vec3 linear_color)
{
    return vec3(pow(linear_color.x, 1.0f / 2.2f),
                pow(linear_color.y, 1.0f / 2.2f),
                pow(linear_color.z, 1.0f / 2.2f));
}

void main()
{
    // Set up sphere
    float sphere_radius = 4.0f;
    vec3 sphere_center = vec3(0.0f, 0.0f, 0.0f);
    vec3 camera_position = vec3(-10.0f, 0.0f, 0.0f);
    vec3 sphere_base_color = vec3(0.3f, 0.7f, 1.0f);
    
    // Set up camera
    vec3 camera_facing = vec3(1.0f, 0.0f, 0.0f);
    vec3 camera_up = vec3(0.0f, 1.0f, 0.0f);
    vec3 camera_right = vec3(0.0f, 0.0f, 1.0f);
    
    // Calculate our ray vector
    vec3 camera_ray = camera_facing +
                      camera_up * o_vertex_position.y +
                      camera_right * o_vertex_position.x * window_size.x / window_size.y;
	
    
    // Perform the ray/sphere intersection
    vec3 hit_point;
    float hit_distance;
    vec3 hit_normal;
    bool intersects_sphere = ray_sphere_intersect(camera_ray, camera_position, sphere_center, sphere_radius, hit_point, hit_distance, hit_normal);
    
    // Output color based on hit result
    if (intersects_sphere)
    {
        // Simple light calculation
        vec3 linear_out = max(dot(hit_normal, light_dir) * sphere_base_color, 0.0);
        o_color = vec4(gamma_correct(linear_out), 0.0f);
    }
    else
    {
        // Solid color elsewhere
        o_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}
