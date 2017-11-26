//
// Created by yuki on 17-11-1.
//

#include <EGL/egl.h>

#include "log.h"

#if defined(TAG)
#undef TAG
#endif

#define TAG "eglUtils"

// begin namespace eglUtil
namespace eglUtils {

EGLDisplay display = EGL_NO_DISPLAY;
EGLSurface surface = EGL_NO_SURFACE;
EGLContext context = EGL_NO_CONTEXT;

EGLint width = 0;
EGLint height = 0;
EGLint format = 0;

EGLBoolean initEGL(ANativeWindow *window)
{
    if (NULL == window) {
        LOGE(TAG, "window is NULL");
        return EGL_FALSE;
    }

    // Get default display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == display) {
        LOGE(TAG, "eglGetDisplay failed, error code: %#x", eglGetError());
        return EGL_FALSE;
    }

    // Initialize EGL display connection
    EGLint majorVer, minorVer;
    EGLBoolean status = eglInitialize(display, &majorVer, &minorVer);
    if (EGL_TRUE != status) {
        EGLint errorCode = eglGetError();
        char *msg = "";
        switch (errorCode) {
        case EGL_BAD_DISPLAY:
            msg = "EGL_BAD_DISPLAY";
            break;
        case EGL_NOT_INITIALIZED:
            msg = "EGL_NOT_INITIALIZED";
            break;
        default:
            msg = "unknown";
            break;
        }
        LOGE(TAG, "eglInitialize failed: %#x, %s", errorCode, msg);

        return EGL_FALSE;
    }
    LOGE(TAG, "majorVer: %d, minorVer: %d", majorVer, minorVer);

    // find out how many configurations are supported
    /*
     * no code here
     */

    // Get a list of EGL frame buffer configurations that match specified attributes
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 5,
            EGL_GREEN_SIZE, 6,
            EGL_RED_SIZE, 5,
            EGL_NONE
    };
    EGLint numConfigs;
    EGLConfig config;
    status = eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    if (EGL_TRUE != status || !numConfigs) {
        LOGE(TAG, "eglChooseConfig failed, error code: %#x", eglGetError());
        return EGL_FALSE;
    }

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
   /* std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }*/

    // Create a new EGL window surface
    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (EGL_NO_SURFACE == surface) {
        EGLint errorCode = eglGetError();
        char *msg = "";
        switch (errorCode) {
        case EGL_BAD_DISPLAY:
            msg = "EGL_BAD_DISPLAY";
            break;
        case EGL_NOT_INITIALIZED:
            msg = "EGL_NOT_INITIALIZED";
            break;
        case EGL_BAD_CONFIG:
            msg = "EGL_BAD_CONFIG";
            break;
        case EGL_BAD_NATIVE_WINDOW:
            msg = "EGL_BAD_NATIVE_WINDOW";
            break;
        case EGL_BAD_ATTRIBUTE:
            msg = "EGL_BAD_ATTRIBUTE";
            break;
        case EGL_BAD_ALLOC:
            msg = "EGL_BAD_ALLOC";
            break;
        case EGL_BAD_MATCH:
            msg = "EGL_BAD_MATCH)";
            break;
        default:
            msg = "unknown";
            break;
        }
        LOGE(TAG, "eglCreateWindowSurface failed: %#x, %s", errorCode, msg);
        return EGL_FALSE;
    }

    // Create a new EGL rendering context
    const EGLint attrs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrs);
    // context = eglCreateContext(display, config, NULL, NULL);
    if (EGL_NO_CONTEXT == context) {
        EGLint errorCode = eglGetError();
        char *msg = "";
        switch (errorCode) {
        case EGL_BAD_MATCH:
            msg = "EGL_BAD_MATCH";
            break;
        case EGL_BAD_DISPLAY:
            msg = "EGL_BAD_DISPLAY";
            break;
        case EGL_NOT_INITIALIZED:
            msg = "EGL_NOT_INITIALIZED";
            break;
        case EGL_BAD_CONFIG:
            msg = "EGL_BAD_CONFIG";
            break;
        case EGL_BAD_CONTEXT:
            msg = "EGL_BAD_CONTEXT";
            break;
        case EGL_BAD_ATTRIBUTE:
            msg = "EGL_BAD_ATTRIBUTE";
            break;
        case EGL_BAD_ALLOC:
            msg = "EGL_BAD_ALLOC";
            break;
        default:
            msg = "unknown";
            break;
        }
        LOGE(TAG, "eglCreateContext failed: %#x, %s", errorCode, msg);
        return EGL_FALSE;
    }

    // Attach an EGL rendering context to EGL surfaces
    status = eglMakeCurrent(display, surface, surface, context);
    if (EGL_TRUE != status) {
        EGLint errorCode = eglGetError();
        char *msg = "";
        switch (errorCode) {
        case EGL_BAD_DISPLAY:
            msg = "EGL_BAD_DISPLAY";
            break;
        case EGL_NOT_INITIALIZED:
            msg = "EGL_NOT_INITIALIZED";
            break;
        case EGL_BAD_SURFACE:
            msg = "EGL_BAD_SURFACE";
            break;
        case EGL_BAD_CONTEXT:
            msg = "EGL_BAD_CONTEXT";
            break;
        case EGL_BAD_MATCH:
            msg = "EGL_BAD_MATCH";
            break;
        case EGL_BAD_ACCESS:
            msg = "EGL_BAD_ACCESS";
            break;
        case EGL_BAD_NATIVE_PIXMAP:
            msg = "EGL_BAD_NATIVE_PIXMAP";
            break;
        case EGL_BAD_NATIVE_WINDOW:
            msg = "EGL_BAD_NATIVE_WINDOW";
            break;
        case EGL_BAD_CURRENT_SURFACE:
            msg = "EGL_BAD_CURRENT_SURFACE";
            break;
        case EGL_BAD_ALLOC:
            msg = "EGL_BAD_ALLOC";
            break;
        case EGL_CONTEXT_LOST:
            msg = "EGL_CONTEXT_LOST";
            break;
        default:
            msg = "unknown";
            break;
        }
        LOGE(TAG, "eglMakeCurrent failed: %#x, %s", errorCode, msg);

        return EGL_FALSE;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    LOGE(TAG, "width: %d, height: %d", width, height);

    // If interval is set to a value of 0, buffer swaps are not synchronized to a video frame,
    // and the swap happens as soon as the render is complete.
    // eglSwapInterval(display, 0);

    return EGL_TRUE;
}

// end namespace eglUtil
}