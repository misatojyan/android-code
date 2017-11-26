//
// Created by yuki on 17-11-1.
//

#ifndef _EGL_UTIL_H_
#define _EGL_UTIL_H_

#include <EGL/egl.h>

// begin namespace eglUtil
namespace eglUtils {

extern EGLDisplay display;
extern EGLSurface surface;
extern EGLContext context;

EGLBoolean initEGL(ANativeWindow *);

// end namespace eglUtil
}

#endif //_EGL_UTIL_H_
