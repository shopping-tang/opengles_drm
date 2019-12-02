#include <gles_drm.h>


int setup_kms(struct kms *kms)
{
    drmModeRes *resources;
    drmModeConnector *connector;
    //drmModeEncoder *encoder;
    drmModePlaneRes *plane_res;
    int i;
    resources = drmModeGetResources(kms->fd); // drmModeRes *resources: 描 述 了 计 算 机 所 有 的 显 卡 信 息： connector， encoder， crtc， modes等  , get their count .
    if (!resources) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return 0;
    }

    drmSetClientCap(kms->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    for (i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(kms->fd, resources->connectors[i]); // get drm_mode 先 获 取  connecotr 的 具 体 资 源  .
        if (connector == NULL)
            continue;

        if (connector->connection == DRM_MODE_CONNECTED &&  // find connected connector ,if find it ,we break loop
            connector->count_modes > 0)
            break;

        drmModeFreeConnector(connector); // if all conditions don't be accepted , connector will be free
    }

    if (i == resources->count_connectors) {
        fprintf(stderr, "No currently active connector found.\n"); // if code in L59 don't run ,then i will equal to count_connectors .
        return 0;
    }

    /*for (i = 0; i < resources->count_encoders; i++) {
        encoder = drmModeGetEncoder(kms->fd, resources->encoders[i]); //  获 取  encoder 的 具 体 资 源  .

        if (encoder == NULL)
            continue;

        if (encoder->encoder_id == connector->encoder_id) // if here is a encoder match with connector , encoder->encoder_id will equal to connector->encoder_id
            break;

        drmModeFreeEncoder(encoder); // if all conditions don't be accepted , encoder will be free
    } */

    plane_res = drmModeGetPlaneResources(kms->fd);

    kms->connector = connector;
    //kms->encoder = encoder;
    kms->mode = connector->modes[0]; // display's basic data
    kms->planes = plane_res->planes;
    kms->crtcs = resources->crtcs ;

    return 1;
}

int init_egl_gbm(struct kms *kms, struct display *displays)
{
    // 1. Get Display
    EGLConfig eglConfig;
    EGLContext eglContext;

    EGLint verMajor, verMinor, n;
    EGLint numConfigs;

    struct gbm_device *gbm;

    static const char drm_device_name[] = "/dev/dri/card0";

    kms->fd = open(drm_device_name, O_RDWR);
    if (kms->fd < 0) {
        /* Probably permissions error */
        LOGE("couldn't open %s, skipping", drm_device_name);
        return -1;
    }

    gbm = gbm_create_device(kms->fd); // Create EGL Context using GBM ? create native_display
    if (gbm == NULL) {
        LOGE("couldn't create gbm device");
        close(kms->fd);
        return -1;
    }

    displays->eglDisplay = eglGetDisplay(gbm); // not EGL_DEFAULT_DISPLAY ! 打 开 一 个 EGL显 示 器 连 接  .
    if( displays->eglDisplay == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS ) {
        LOGE("getdisplay error !");
        return eglGetError();
    }

    // 2. Initialize EGL . verMajor : EGL main version ; verMinor : EGL minor version
    if ( eglInitialize(displays->eglDisplay, &verMajor, &verMinor) == EGL_FALSE || eglGetError() != EGL_SUCCESS ) { //初 始 化  EGL 内 部 数 据 , 返 回  EGL 主 次 版 本 号 .
        LOGE("egl init error ! %d", eglGetError());
        return eglGetError();
    }

    const char *ver = eglQueryString(displays->eglDisplay, EGL_VERSION); // query and get EGL_version

    if ( eglGetConfigs( displays->eglDisplay, NULL, 0, &numConfigs) == EGL_FALSE || eglGetError() != EGL_SUCCESS ) {  // 查 询 获 取 窗 口 配 置 信 息  .
        std::cerr << "getdisplay error !" << std::endl;
        return eglGetError();
    }

    LOGD("* EGL_VERSION = %s (have %d configs)", ver, numConfigs);

    if (!setup_kms(kms)) {
        LOGE("setup kms failed !");
        return -1;
    }
    eglBindAPI(EGL_OPENGL_ES_API); // 设 置 当 前 渲 染  API  Specifies the client API to bind, one of EGL_OPENGL_API, EGL_OPENGL_ES_API, or EGL_OPENVG_API.

    // 3. Choose Config ,Config实 际 指 的 是  FrameBuffer 的 参 数  .
    if (!eglChooseConfig(displays->eglDisplay, attribs, &eglConfig, 1, &n) || n != 1) { // 选 出 最 匹 配 要 求 的 窗 口. .
        LOGE("failed to choose argb config");
        return eglGetError();
    }

    // 4. Create GBM Surface = native_window .
    displays->gbmSurface = gbm_surface_create(gbm, kms->mode.hdisplay, kms->mode.vdisplay,
                            GBM_FORMAT_RGB888,
                            GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    fprintf(stderr, "%s %d gbmSurface=%p\n",__func__,__LINE__, displays->gbmSurface);
    displays->eglSurface = eglCreateWindowSurface(displays->eglDisplay, eglConfig, displays->gbmSurface, NULL); // 创 建 一 个 可 实 际 显 示 的  EGL Surface, Surface 实 际 上 就 是 一 个  FrameBuffer，也 就 是 渲 染 目 的 地 .


    // 5. Create Context
    static const EGLint context_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    eglContext = eglCreateContext(displays->eglDisplay, eglConfig, EGL_NO_CONTEXT, context_attribs); // Context就 代 表 这 个 状 态 机 , 有 当 前 的 颜 色 、纹 理 坐 标 、变 换 矩 阵 、绚 染 模 式 等 一 大 堆 状 态  .
    if (eglContext == NULL) {
        LOGE("failed to create context");
        return eglGetError();
    }
    // //eglMakeCurrent后 生 成 的  surface 就 可 以 利 用  opengl 画 图 了  .
    if (!eglMakeCurrent(displays->eglDisplay, displays->eglSurface, displays->eglSurface, eglContext)) { // 指 定 某 个  eglContext 为 当 前 上 下 文 ，关 联 特 定 的  eglContext 和  eglSurface.
        LOGE("failed to make context current");
        return eglGetError();
    }
    return 1 ;
}

