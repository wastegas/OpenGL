#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cassert>
#include "logging.h"
#include "gl_utils.h"

int g_gl_width = 640;
int g_gl_height = 480;

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
  // starg GL context and O/S window using GLFW helper library
  gl_log("starting GLFW\n%s\n", glfwGetVersionString());
  // register the error call-back function we wrote
  glfwSetErrorCallback(glfw_error_callback);


  
  if (!glfwInit()) {
    fprintf(stderr, "Error: could not start GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // Anti-aliasing
  glfwWindowHint(GLFW_SAMPLES, 4);

  // Window resolution and full-screen
  GLFWmonitor* mon = glfwGetPrimaryMonitor();
  const GLFWvidmode* vmode = glfwGetVideoMode(mon);
  
  GLFWwindow* window = glfwCreateWindow(g_gl_width, g_gl_height,
					"Extended GL Init",
					NULL, NULL);
  if(!window) {
    fprintf(stderr, "Error: could not open window GLFW3\n");
    glfwTerminate();
    return 1;
  }


  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);
  //log_gl_params();
  
  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported: %s\n", version);

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
  while(!glfwWindowShouldClose (window)) {
    //    _update_fps_counter(window);
      //wipe the drawing surface clear
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glViewport(0, 0, g_gl_width, g_gl_height);
      glUseProgram(shader_programme);
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      //update other events like input handling
      glfwPollEvents();
      if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
	  glfwSetWindowShouldClose(window, 1);
	}
      //put the stuff we've been drawing onto the display
      glfwSwapBuffers (window);
    }
  
  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
