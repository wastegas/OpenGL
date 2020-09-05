#ifndef _GL_UTILS_H
#define _GL_UTILS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "logging.h"

extern int g_gl_width;
extern int g_gl_height;

void glfw_error_callback(int, const char*);
void glfw_window_size_callback(GLFWwindow*, int, int);
void log_gl_params();
const char* GL_type_to_string(GLenum);
void _print_shader_info_log(GLuint);
void _print_programme_info_log(GLuint);
void print_all(GLuint);
bool is_valid(GLuint);
bool parse_file_into_str(const char*, char*, int);


#endif 
