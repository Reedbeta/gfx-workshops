# Master CMakeLists to build all the workshop projects.
cmake_minimum_required(VERSION 3.6.1)
project(gfx-workshops)

# Use only static linking to avoid hassle of shared libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Allow debug and release configurations for all projects
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# First build GLFW, following instructions from: http://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_source
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if (WIN32)
	set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
endif()
if (MSVC)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory(glfw)

