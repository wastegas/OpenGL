#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <cassert>
#include <math.h>
#include "math_funcs.h"
#include "gl_utils.h"
#include "logging.h"

int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow* g_window;

int main()
{
  assert(restart_gl_log());
  // starg GL context and O/S window using GLFW helper library
  gl_log("starting GLFW\n%s\n", glfwGetVersionString());
  // register the error call-back function we wrote
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
  // copy our points into the currently bound buffer
  glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);

  GLuint colours_vbo = 0; // our vertex buffer
  glGenBuffers (1, &colours_vbo); // set as the current buffer
  glBindBuffer (GL_ARRAY_BUFFER, colours_vbo);
  // copy our points into the currently bound buffer
  glBufferData (GL_ARRAY_BUFFER, sizeof (points), colours, GL_STATIC_DRAW);

  
  GLuint vao = 0; // our mesh aka vertext array
  glGenVertexArrays (1, &vao); // turn vao into a mesh
  glBindVertexArray(vao); // make it current mesh
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);


  char vertex_shader[1024 * 256];
  char fragment_shader[1024 * 256];
  parse_file_into_str("test4_vs.glsl", vertex_shader, 1024 * 256);
  parse_file_into_str("test4_fs.glsl", fragment_shader, 1024 * 256);
  
  
  const GLchar* p = NULL;
  
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

  float matrix[] = {
		    1.0f, 0.0f, 0.0f, 0.0f, // first column
		    0.0f, 1.0f, 0.0f, 0.0f, // second column
		    0.0f, 0.0f, 1.0f, 0.0f, // third column
		    0.5f, 0.0f, 0.0f, 1.0f  // fourth column
  };


  int view_mat_location = glGetUniformLocation(shader_programme, "view");
  glUseProgram(shader_programme);
  glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view_mat.m);
  int proj_mat_location = glGetUniformLocation(shader_programme, "proj");
  glUseProgram(shader_programme);
  glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, proj_mat);

  // virtual camera section
  float cam_pos[] = {0.0f, 0.0f, 2.0f};
  float cam_yaw = 0.0f; // y-rotation degrees
  mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1],
					 -cam_pos[2]));
  mat4 R = rotaty_y_deg(identity_mat4(), -cam_yaw);
  mat4 view_mat = R * T;

  // input variables
  float near = 0.1f; // clipping plane
  float far = 100.0f; // clipping plane
  float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 deg to rad
  float aspect = (float)width / (float)height; // aspect ratio
  // matrix components
  float range = tan (fov * 0.5f) * near;
  float Sx = (2.0f * near) / (range * aspect + range * aspect);
  float Sy = near / range;
  float Sz = -(far + near) / (far - near);
  float Pz = -(2.0f * far * near) / (far - near);

  float proj_mat[] = {
		      Sx, 0.0f, 0.0f, 0.0f,
		      0.0f, Sy, 0.0f, 0.0f,
		      0.0f, 0.0f, Sz, -1.0f,
		      0.0f, 0.0f, Pz, 0.0f
  };
  

  
  glEnable (GL_CULL_FACE); // cull face
  glCullFace (GL_BACK); // cull back face
  glFrontFace (GL_CW); // GL_CCW for counter clock-wise

  float speed = 1.0f; // move at 1 unit per sec
  float last_position = 0.0f;
  // camera speed
  float cam_speed = 1.0f; // 1 unit per second
  float cam_yaw_speed = 10.0f; // 10 degrees per second

  // draw our triangle
  while(!glfwWindowShouldClose (g_window)) {
    // add a timer for amimation
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapse_seconds          = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;
    
    _update_fps_counter(g_window);
    //wipe the drawing surface clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, g_gl_width, g_gl_height);


    // reverse direction when goint to far left or right
    if (fabs(last_position) > 1.0f) {
      speed = -speed;
    }
    
    
    // update the matrix
    matrix[12] = elapse_seconds * speed + last_position;
    last_position = matrix[12];
    glUseProgram(shader_programme);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, matrix);

    glBindVertexArray(vao);
    //draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays(GL_TRIANGLES, 0, 3);
    //update other events like input handling
    glfwPollEvents();
    bool cam_moved = false;
    if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
      cam_pos[0] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
      cam_pos[0] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_PAGE_UP ) ) {
      cam_pos[1] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_PAGE_DOWN ) ) {
      cam_pos[1] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
      cam_pos[2] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
      cam_pos[2] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
      cam_yaw += cam_yaw_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
      cam_yaw -= cam_yaw_speed * elapsed_seconds;
      cam_moved = true;
    }

    if ( cam_moved ) {
      mat4 T = translate( identity_mat4(),
			  vec3( -cam_pos[0],
				-cam_pos[1],
				-cam_pos[2] ) ); // cam translation
      mat4 R = rotate_y_deg( identity_mat4(),
			     -cam_yaw );     //
      mat4 view_mat = R * T;
      glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
    }

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
