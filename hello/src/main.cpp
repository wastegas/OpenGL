#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cassert>
#include "logging.h"

int g_gl_width = 640;
int g_gl_height = 480;

static void glfw_error_callback(int error, const char* description) {
  gl_log_err("GLFW ERROR: code %i msg: %s\n", error, description);
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  g_gl_width = width;
  g_gl_height = height;

  /* update any perspectie matrices here */
}

static void log_gl_params() {
  GLenum params[] = {
		     GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		     GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		     GL_MAX_DRAW_BUFFERS,
		     GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		     GL_MAX_TEXTURE_IMAGE_UNITS,
		     GL_MAX_TEXTURE_SIZE,
		     GL_MAX_VARYING_FLOATS,
		     GL_MAX_VERTEX_ATTRIBS,
		     GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		     GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		     GL_MAX_VIEWPORT_DIMS,
		     GL_STEREO,
  };

  const char* names[] = {
		     "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
		     "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
		     "GL_MAX_DRAW_BUFFERS",
		     "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
		     "GL_MAX_TEXTURE_IMAGE_UNITS",
		     "GL_MAX_TEXTURE_SIZE",
		     "GL_MAX_VARYING_FLOATS",
		     "GL_MAX_VERTEX_ATTRIBS",
		     "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
		     "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
		     "GL_MAX_VIEWPORT_DIMS",
		     "GL_STEREO",
  };

  gl_log("GL Context Params:\n");
  char msg[256];
  // integers - only works if the order is 0-10 integer return types
  for (int i = 0; i < 10; i++) {
    int v = 0;
    glGetIntegerv (params[i], &v);
    gl_log("%s %i\n", names[i], v);
  }

  //others
  int v[2];
  v[0] = v[1] = 0;
  glGetIntegerv (params[10], v);
  gl_log("%s %i %i\n", names[10], v[0], v[1]);
  unsigned char s = 0;
  glGetBooleanv (params[11], &s);
  gl_log("%s %u\n", names[11], (unsigned int)s);
  gl_log("-----------------------------\n");
}

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
  log_gl_params();
  
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

  GLuint vbo = 0; // our vertex buffer
  glGenBuffers (1, &vbo); // set as the current buffer
  glBindBuffer (GL_ARRAY_BUFFER, vbo);
  // copy our points into the currently bound buffer
  glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);

  GLuint vao = 0; // our mesh aka vertext array
  glGenVertexArrays (1, &vao); // turn vao into a mesh
  glBindVertexArray(vao); // make it current mesh
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo); // put our buffer into the mesh
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  //Shader programs
  const char* vertex_shader =
    "#version 400\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4 (vp, 1.0);"
    "}";

  const char* fragment_shader =
    "#version 400\n"
    "out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
    "}";

  GLuint vs = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);

  GLuint shader_programme = glCreateProgram();
  glAttachShader (shader_programme, fs);
  glAttachShader (shader_programme, vs);
  glLinkProgram (shader_programme);

  // draw our triangle
  while(!glfwWindowShouldClose (window)) {
    _update_fps_counter(window);
      //wipe the drawing surface clear
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glViewport(0, 0, g_gl_width, g_gl_height);
      glUseProgram(shader_programme);
      glBindVertexArray(vao);
      //draw points 0-3 from the currently bound VAO with current in-use shader
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
