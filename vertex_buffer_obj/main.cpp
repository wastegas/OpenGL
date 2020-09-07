#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cassert>
#include "logging.h"
#include "gl_utils.h"

int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow* g_window = NULL;

static void _update_fps_counter(GLFWwindow* window) {
  static double previous_seconds = glfwGetTime();
  static int frame_count;
  double current_seconds = glfwGetTime();
  double elapse_seconds = current_seconds - previous_seconds;
  if (elapse_seconds > 0.25) {
    previous_seconds = current_seconds;
    double fps = (double)frame_count / elapse_seconds;
    char tmp[128];
    sprintf(tmp, "opengl @ fps: %.2f", fps);
    glfwSetWindowTitle(window, tmp);
    frame_count = 0;
  }
  frame_count++;
}

int main()
{
  
  assert(restart_gl_log());
  restart_gl_log();
  start_gl();
  

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable (GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

  /* other stuff goes here */
  GLfloat points[] = {
		      0.0f, 0.5f, 0.0f, // top point
		      0.5f, -0.5f, 0.0f, // bot right point
		      -0.5f, -0.5f, 0.0f  // bot left point
  };
  GLfloat colours[] = {
		       1.0f, 0.0f, 0.0f,
		       0.0f, 1.0f, 0.0f,
		       0.0f, 0.0f, 1.0f
  };

  GLuint points_vbo = 0; // our vertex buffer
  glGenBuffers (1, &points_vbo); // set as the current buffer
  glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);

  GLuint colours_vbo = 0;
  glGenBuffers(1, &colours_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof (colours), colours, GL_STATIC_DRAW);

  GLuint vao = 0; 
  glGenVertexArrays (1, &vao); 
  glBindVertexArray(vao); 
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo); 
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(0); // points_vbo
  glEnableVertexAttribArray(1); // colours_vbo

  char vertex_shader[1024 * 256];
  char fragment_shader[1024 * 256];  
  parse_file_into_str("test2_vs.glsl", vertex_shader, 1024 * 256);
  parse_file_into_str("test2_fs.glsl", fragment_shader, 1024 * 256);

  const GLchar* p;
  
  GLuint vs = glCreateShader (GL_VERTEX_SHADER);
  p = (const GLchar*)vertex_shader;
  glShaderSource (vs, 1, &p, NULL);
  glCompileShader(vs);

  // check for compile errors
  int params = -1;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(stderr, "ERROR: GL shader index %i did not compile\n",
	    vs);
    _print_shader_info_log(vs);
    return -1;
  }
  
  GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
  p = (const GLchar*)fragment_shader;
  glShaderSource (fs, 1, &p, NULL);
  glCompileShader(fs);
  params = -1;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(stderr, "ERROR: GL shader index %i did not compile\n",
	    fs);
    _print_shader_info_log(fs);
    return false;
  }
  
  GLuint shader_programme = glCreateProgram();
  glAttachShader (shader_programme, fs);
  glAttachShader (shader_programme, vs);

  // location binding code
  glBindAttribLocation (shader_programme, 0, "vertex_position");
  glBindAttribLocation (shader_programme, 1, "vertex_colour");
  
  glLinkProgram (shader_programme);

  params = -1;
  glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(stderr, "ERROR: could not link shader programm GL index %u\n",
	    shader_programme);
    _print_programme_info_log(shader_programme);
    return false;
  }
  
  bool result = is_valid(shader_programme);
  assert(result);

  glEnable (GL_CULL_FACE); // cull face
  glCullFace (GL_BACK); // cull back face
  glFrontFace (GL_CW); // GL_CCW for counter clock-wise

  // draw our triangle
  while(!glfwWindowShouldClose (g_window)) {
    //    _update_fps_counter(window);
      //wipe the drawing surface clear
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glViewport(0, 0, g_gl_width, g_gl_height);
      glUseProgram(shader_programme);
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      //update other events like input handling
      glfwPollEvents();
      if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE))
	{
	  glfwSetWindowShouldClose(g_window, 1);
	}
      //put the stuff we've been drawing onto the display
      glfwSwapBuffers (g_window);
    }
  
  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
