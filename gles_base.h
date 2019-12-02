#ifndef __GLES_BASE_H__
#define __GLES_BASE_H__

#include <GLES/gl.h>
#include <GLES/egl.h>
#include <GLES/glext.h>

#include <GLES3/gl32.h>
#include <GLES3/gl3platform.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include "v4l2.h"

extern "C"
{
    void init_texture(GLuint *texture);
    void draw(GLuint Program, GLuint *textures, unsigned char *data);
    GLuint bind_array();
}

#endif
