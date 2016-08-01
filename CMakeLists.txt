# Master CMakeLists to build all the workshop projects.
cmake_minimum_required(VERSION 3.6.1)
project(gfx-workshops)

# Allow debug and release configurations for all projects
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Use only static linking to avoid hassle of shared libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Use static linking for MSVC runtime as well
# See: https://cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
if (MSVC)
	foreach (flag CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
				  CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		if (${flag} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
		endif()
		if (${flag} MATCHES "/MDd")
			string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
		endif()
	endforeach()
endif()

# Build GLFW, following instructions from: http://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_source
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if (WIN32)
	set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
endif()
add_subdirectory(glfw)

# Build the individual workshop projects
set(WORKSHOPS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(workshop01)
