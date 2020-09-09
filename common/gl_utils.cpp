#include "gl_utils.h"

/* log glfw errors */
void glfw_error_callback(int error, const char* description) {
  gl_log_err("GLFW ERROR: code %i msg: %s\n", error, description);
}

/* monitor window size */
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  g_gl_width = width;
  g_gl_height = height;

  /* update any perspectie matrices here */
}

/* log current gl params */
void log_gl_params() {
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

/* convert GL type to string */
const char* GL_type_to_string(GLenum type) {
  switch(type) {
  case GL_BOOL: return "bool";
  case GL_INT: return "int";
  case GL_FLOAT: return "float";
  case GL_FLOAT_VEC2: return "vec2";
  case GL_FLOAT_VEC3: return "vec3";
  case GL_FLOAT_VEC4: return "vec4";
  case GL_FLOAT_MAT2: return "mat2";
  case GL_FLOAT_MAT3: return "mat3";
  case GL_FLOAT_MAT4: return "mat4";
  case GL_SAMPLER_2D: return "sampler2D";
  case GL_SAMPLER_3D: return "sampler3D";
  case GL_SAMPLER_CUBE: return "samplerCube";
  case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
  default: break;
  }
  return "other";
}

/* Print the shader info log */
void _print_shader_info_log(GLuint shader_index) {
  int max_length = 2048;
  int actual_length = 0;
  char log[2048];
  glGetShaderInfoLog(shader_index, max_length, &actual_length,
		     log);
  printf("shader info log for GL index %u\n%s\n",
	 shader_index, log);
}

/* Print the program info log */
void _print_programme_info_log(GLuint programme) {
  int max_length = 2048;
  int actual_length = 0;
  char log[2048];
  glGetProgramInfoLog(programme, max_length, &actual_length, log);
  printf("program info log for GL index %u:\n%s", programme, log);
}

/* Print all information */
void print_all(GLuint programme) {
  printf("-------------------\nshader programme %i info:\n",
	 programme);
  int params = -1;
  glGetProgramiv (programme, GL_LINK_STATUS, &params);
  printf("GL_LINK_STATUS = %i\n", params);

  glGetProgramiv (programme, GL_ATTACHED_SHADERS, &params);
  printf("GL_ATTACHED_SHADERS = %i\n", params);

  glGetProgramiv (programme, GL_ACTIVE_ATTRIBUTES, &params);
  printf("GL_ACTIVE_ATTRIBUTES = %i\n", params);
  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveAttrib (programme, i, max_length, &actual_length,
		       &size, &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
	char long_name[64];
	sprintf(long_name, "%s[%i]", name, j);
	int location = glGetAttribLocation(programme, long_name);
	printf(" %i) type: %s name:%s location:%i\n", i,
	       GL_type_to_string(type), long_name, location);
      }
    } else {
      int location = glGetAttribLocation (programme, name);
      printf(" %i) type:%s name:%s location:%i\n", i,
	     GL_type_to_string(type), name, location);
    }
  }

  glGetProgramiv (programme, GL_ACTIVE_UNIFORMS, &params);
  printf ("GL_ACTIVE_UNIFORMS = %i\n", params);
  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveUniform (programme, i, max_length, &actual_length,
			&size, &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
	char long_name[64];
	sprintf(long_name, "%s[%i]", name, j);
	int location = glGetUniformLocation (programme, long_name);
	printf(" %i) type:%s name:%s location:%i\n", i,
	       GL_type_to_string (type), long_name, location);
      }
    } else {
      int location = glGetUniformLocation (programme, name);
      printf (" %i) type:%s name:%s location:%i\n", i,
	      GL_type_to_string (type), name, location);
    }
  }

  _print_programme_info_log (programme);
}


/* validate shader */
bool is_valid (GLuint programme) {
  glValidateProgram (programme);
  int params = -1;
  glGetProgramiv (programme, GL_VALIDATE_STATUS, &params);
  printf("program %i GL_VALIDATE_STATUS = %i\n", programme,
	 params);
  if (GL_TRUE != params) {
    _print_programme_info_log (programme);
    return false;
  }
  return true;
}

/* load plain text shader to a character array */
bool parse_file_into_str(const char* file_name, char* shader_str,
			 int max_len) {
  FILE* file = fopen(file_name, "r");
  if(!file) {
    gl_log_err("ERROR: opening file for reading: %s\n", file_name);
    return false;
  }

  size_t cnt = fread(shader_str, 1, max_len - 1, file);
  if ((int)cnt >= max_len -1) {
    gl_log_err("WARNING: file %s too big - truncated.\n", file_name);
  }
  if (ferror(file)) {
    gl_log_err("ERROR: reading shader file %s\n", file_name);
    fclose(file);
    return false;
  }
  // append \0 to end of string
  shader_str[cnt] = 0;
  fclose(file);
  return true;
}

void _update_fps_counter(GLFWwindow* window) {
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

bool start_gl() {

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

  /* Window resolution and full-screen
  GLFWmonitor* mon = glfwGetPrimaryMonitor();
  const GLFWvidmode* vmode = glfwGetVideoMode(mon);
  
  GLFWwindow* window = glfwCreateWindow(g_gl_width, g_gl_height,
					"Extended GL Init",
					NULL, NULL);
  */

  g_window = glfwCreateWindow(g_gl_width, g_gl_height,
			      "Extended GL init",
			      NULL, NULL);
  
  if(!g_window) {
    fprintf(stderr, "Error: could not open window GLFW3\n");
    glfwTerminate();
    return 1;
  }


  glfwSetWindowSizeCallback(g_window, glfw_window_size_callback);
  glfwMakeContextCurrent(g_window);
  //log_gl_params();
  
  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported: %s\n", version);
  gl_log("renderer: %s version: %s\n", renderer, version);
  
  return true;
}
