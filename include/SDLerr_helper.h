#pragma once

#include <stdlib.h>
#include <stdio.h>

#define _LOG_SDL_ERROR(file, line) fprintf(stderr, "\x1b[31mERROR\x1b[m at %s:%d, \"%s\" (<- SDL errmsg)\n", file, line, SDL_GetError())

#ifdef DEBUG
#define FAIL_ON_RET(expr, ret, line, file) \
do {                                       \
	if (!expr) {                           \
		_LOG_SDL_ERROR(file, line);        \
		return ret;                        \
	}                                      \
} while (false)

#define ASSIGN_OR_FAIL_ON_RET(var, expr, ret, line, file) \
var = expr;                                               \
do {                                                      \
	if (!var) {                                           \
		_LOG_SDL_ERROR(file, line);                       \
		return ret;                                       \
	}                                                     \
} while (false)
#define DECLARE_OR_FAIL_ON_RET(type, varname, expr, ret, line, file) \
type varname = expr;                                                 \
do {                                                                 \
	if (!varname) {                                                  \
		_LOG_SDL_ERROR(file, line);                                  \
		return ret;                                                  \
	}                                                                \
} while (false)
#else
# define FAIL_ON_RET(expr, msg, ret, line) (expr)
# define ASSIGN_OR_FAIL_ON_RET(var, expr, errmsg, ret, line) var = expr
# define DECLARE_OR_FAIL_ON_RET(type, varname, expr, errmsg, EXIT_FAILURE, line) type varname = expr
#endif

#define FAILON(condition)               FAIL_ON_RET(condition, -1, __LINE__, __FILE__)
#define ASSIORFAIL(var, expr)           ASSIGN_OR_FAIL_ON_RET(var, expr, EXIT_FAILURE, __LINE__, __FILE__)
#define DECLORFAIL(type, varname, expr) DECLARE_OR_FAIL_ON_RET(type, varname, expr, EXIT_FAILURE, __LINE__, __FILE__)

