#ifndef _LOGGING_H
#define _LOGGING_H

#include <time.h>
#include <stdarg.h>
#include <cstdio>
#define GL_LOG_FILE "gl.log"

bool restart_gl_log();
bool gl_log(const char*, ...);
bool gl_log_err(const char*, ...);

#endif
