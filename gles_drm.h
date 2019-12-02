#ifndef __GLES_DRM_H__
#define __GLES_DRM_H__

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <gbm.h>
#include <GLES/egl.h>
#include <gbm.h>
#include "v4l2.h"

#define LOGD(...) fprintf(stdout, __VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, __VA_ARGS__); printf("\n")

struct kms {
    drmModeConnector *connector;
    //drmModeEncoder *encoder;
    drmModeModeInfo mode;
    uint32_t *crtcs;
    uint32_t *planes;
    uint32_t fb_id;
    int fd;
};

struct framebuffer {
    int fd;
    uint32_t width, height;
    uint32_t id;
};

const EGLint attribs[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
};

struct display {
       EGLSurface eglSurface;
       EGLDisplay eglDisplay;
       struct gbm_surface *gbmSurface;
       struct gbm_bo *bo;
       struct gbm_bo *next_bo;
};

extern "C" {
    int setup_kms(struct kms *kms);
    int init_egl_gbm(struct kms *kms, struct display *displays);
}

#endif
