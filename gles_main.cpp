#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <gbm.h>
#include <xf86drmMode.h>
#include <xf86drm.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <GLES/glext.h>

#include <GLES3/gl32.h>
#include <GLES3/gl3platform.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <thread>
#include "gles_drm.h"
#include "gles_base.h"
#include "Shader.h"
#include "v4l2.h"

using namespace std;

static int _terminate = 0;

static void sigint_handler(int arg)
{
    _terminate = 1;
}

int video_display(int cpuid, int video_index, int crtc_index, int plane_index, uint32_t display_x, uint32_t display_y, uint32_t display_w, uint32_t display_h) {
    std::cout << "Hello, World!" << std::endl;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuid, &mask);

    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        printf("set thread affinity failed");
        return(FALSE);
    }
    printf("Bind CameraCapture process to CPU %d\n", cpuid);
    /***************** base paramters *****************/
    struct kms kms;
    struct display displays ;
    /***************** v4l2 paramters *****************/
    BUF *buffer = (BUF *)malloc(1*(sizeof (BUF)));

    char video[25];
    snprintf(video, sizeof(video), "/dev/video%d", video_index);

    //char video[] = "/dev/video10" ;
    unsigned char *srcBuffer = (unsigned char*)malloc(sizeof(unsigned char) * BUFFER_SIZE_src);  // yuyv
    //unsigned char *dstBuffer = (unsigned char*)malloc(sizeof(unsigned char) * BUFFER_SIZE_det);  // RGB

    /***************** v4l2 function *****************/
    int ret = v4l2(video, buffer) ;
    if (ret == 0){
        printf("v4l2 run failed .\n");
        return 0 ;
    }

    /***************** init egl gbm *****************/
    init_egl_gbm(&kms, &displays);

    /***************** opengles function *****************/
    Shader ourShader(1);
    
    GLuint VAO = bind_array() ;

    GLuint textures[3];
    init_texture(textures);

    /** if loop display rended image , begin here . **/

    //glViewport(0, 0, 0, 0);  // set a area to display

    eglSwapBuffers(displays.eglDisplay, displays.eglSurface); // ready to display , whrite image to gbmSurface .
    displays.bo = gbm_surface_lock_front_buffer(displays.gbmSurface); // get buffer_object

    uint32_t handle = gbm_bo_get_handle(displays.bo).u32;  // get handle of buffer_object
    uint32_t stride = gbm_bo_get_stride(displays.bo);
    int width = gbm_bo_get_width(displays.bo);
    int height = gbm_bo_get_height(displays.bo);
    //kms.fb_id = fb->id ;
    printf("handle=%d, stride=%d rect=%dx%d\n", handle, stride, width, height); // here is frame's parameters : handle, stride, width, height .

    uint32_t handles[4], strides[4], offsets[4];

    handles[0] = handle;
    strides[0] = stride;
    offsets[0] = 0;
    ret = drmModeAddFB2(kms.fd, width, height, GBM_FORMAT_RGB888,
            handles, strides, offsets,
            &kms.fb_id, 0);
    if (ret) {
        LOGE("failed to create fb");
        //goto rm_fb;
        return -1;
    }

    uint32_t test_crtc = kms.crtcs[crtc_index] ;
    drmModeCrtcPtr saved_crtc = drmModeGetCrtc(kms.fd, test_crtc);
    if (saved_crtc == NULL) {
        LOGE("failed to crtc: %m");
        return -1;
    }

    signal(SIGINT, sigint_handler);
    while(!_terminate)
    {
        struct gbm_bo *next_bo = NULL;
        srcBuffer = get_img(buffer, srcBuffer);
        //yuyv2bgr24(srcBuffer, dstBuffer);

        glClearColor(1.0, 1.0, 1.0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ourShader.Use();
        glBindVertexArray(VAO);
        draw(ourShader.Program, textures, srcBuffer);
        glBindVertexArray(0);
        eglSwapBuffers(displays.eglDisplay, displays.eglSurface); // ready to display , whrite image to gbmSurface .
        next_bo = gbm_surface_lock_front_buffer(displays.gbmSurface); // get buffer_object

        ret = drmModeSetPlane(kms.fd, kms.planes[plane_index], // display FrameBuffer
            test_crtc, kms.fb_id, 0, display_x, display_y, display_w, display_h,
            0, 0, width << 16, height << 16);
        if (ret) {
            printf("failed to set plane 0 %d\n", ret);
            return 0;
        }

        gbm_surface_release_buffer(displays.gbmSurface, displays.bo);
        displays.bo = next_bo;
    }

    ret = drmModeSetCrtc(kms.fd, saved_crtc->crtc_id, saved_crtc->buffer_id,
                         saved_crtc->x, saved_crtc->y,
                         &kms.connector->connector_id, 1, &saved_crtc->mode);
    ourShader.Use_end();
    if (ret) {
        LOGE("failed to restore crtc: %m");
    }
    close_v4l2(buffer);
    free(srcBuffer);
    return 0;
}

#if 1
int main()
{
    signal(SIGINT, sigint_handler);
    array<thread, 2> threads;
    threads = {thread(video_display, 0, 10, 0, 0, 128, 0, 1280, 960),
               thread(video_display, 1, 12, 0, 2, 128, 1088, 1280, 960),
              };
    for (int i = 0; i < 2; i++){
        threads[i].join();
    }
    return(TRUE);
}
#else
int main()
{
    signal(SIGINT, sigint_handler);

    video_display(0, 10, 0, 0, 128, 0, 1280, 960);

    return(TRUE);
}
#endif
