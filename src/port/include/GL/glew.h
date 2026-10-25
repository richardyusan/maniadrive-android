/*
 * ManiaDrive Android port - GLEW replacement.
 *
 * Raydium only calls glewInit() and then uses the ARB/EXT entry points
 * directly. gl4es exports all of them, so this header only has to pull in
 * the OpenGL headers (with prototypes) and provide a dummy glewInit().
 */
#ifndef MANIA_PORT_GLEW_H
#define MANIA_PORT_GLEW_H

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <GL/gl.h>
#include <GL/glext.h>

#define GLEW_OK 0
#define GLEW_VERSION_1_3 1

static inline GLenum glewInit(void) { return GLEW_OK; }

#endif
