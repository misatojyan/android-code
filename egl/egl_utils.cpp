//
// Created by yuki on 17-11-1.
//

#include <assert.h>

#include <iostream>
#include <memory>

#include <EGL/egl.h>

#include "log.h"

#if defined(TAG)
#undef TAG
#endif

#define TAG "eglUtils"

// begin namespace eglUtils
namespace eglUtils {

EGLDisplay display = EGL_NO_DISPLAY;
EGLSurface surface = EGL_NO_SURFACE;
EGLContext context = EGL_NO_CONTEXT;

EGLint width = 0;
EGLint height = 0;
EGLint format = 0;

#define PE(function) printError(__FILE__, __LINE__, #function)
#define CASE(error, msg) case error: msg = #error; break;

static void printError(const char *file, const int line, const char *function)
{
    EGLint errorCode = eglGetError();
    char *msg = "";
    switch (errorCode) {
    CASE(EGL_SUCCESS, msg)
    CASE(EGL_NOT_INITIALIZED, msg)
    CASE(EGL_BAD_ACCESS, msg)
    CASE(EGL_BAD_ALLOC, msg)
    CASE(EGL_BAD_ATTRIBUTE, msg)
    CASE(EGL_BAD_CONTEXT, msg)
    CASE(EGL_BAD_CONFIG, msg)
    CASE(EGL_BAD_CURRENT_SURFACE, msg)
    CASE(EGL_BAD_DISPLAY, msg)
    CASE(EGL_BAD_SURFACE, msg)
    CASE(EGL_BAD_MATCH, msg)
    CASE(EGL_BAD_PARAMETER, msg)
    CASE(EGL_BAD_NATIVE_PIXMAP, msg)
    CASE(EGL_BAD_NATIVE_WINDOW, msg)
    CASE(EGL_CONTEXT_LOST, msg)
    default:
        msg = "unknown";
        break;
    }
    LOGE(TAG, "%s@<%d@%s> failed: %#x, %s", function, line, file, errorCode, msg);
}

EGLBoolean initEGL(ANativeWindow *window)
{
    if (NULL == window) {
        LOGE(TAG, "window is NULL");
        return EGL_FALSE;
    }

    // Get default display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == display) {
        LOGE(TAG, "eglGetDisplay failed");
        return EGL_FALSE;
    }

    // Initialize EGL display connection
    EGLint majorVer, minorVer;
    EGLBoolean status = eglInitialize(display, &majorVer, &minorVer);
    if (EGL_TRUE != status) {
        PE(eglInitialize);
        return EGL_FALSE;
    }
    LOGE(TAG, "majorVer: %d, minorVer: %d", majorVer, minorVer);


    // Get a list of EGL frame buffer configurations that match specified attributes
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint numConfigs;
    EGLConfig config;
    /**
     * Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    status = eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);
    if (EGL_TRUE != status) {
        PE(eglChooseConfig);
        return EGL_FALSE;
    }
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    status = eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    if (EGL_TRUE != status) {
        PE(eglChooseConfig);
        return EGL_FALSE;
    }
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
    }

    /*if (EGL_TRUE != status || !numConfigs) {
        LOGE(TAG, "eglChooseConfig failed, error code: %#x", eglGetError());
        return EGL_FALSE;
    }*/

    // Create a new EGL window surface
    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (EGL_NO_SURFACE == surface) {
        PE(eglCreateWindowSurface);
        return EGL_FALSE;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    // Create a new EGL rendering context
    const EGLint attrs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrs);
    if (EGL_NO_CONTEXT == context) {
        PE(eglCreateContext);
        return EGL_FALSE;
    }

    // Attach an EGL rendering context to EGL surfaces
    status = eglMakeCurrent(display, surface, surface, context);
    if (EGL_TRUE != status) {
        PE(eglMakeCurrent);
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

// end namespace eglUtils
}
