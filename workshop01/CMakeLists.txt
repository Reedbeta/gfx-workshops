cmake_minimum_required(VERSION 3.6.1)
project(workshop01)
add_executable(workshop01
	workshop01.cpp
	../glad/src/glad.c
	stb_image.h)
target_include_directories(workshop01 PRIVATE "${WORKSHOPS_ROOT_DIR}/glad/include")
target_link_libraries(workshop01 glfw)
