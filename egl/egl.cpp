#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#include <android/native_window_jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-egl", __VA_ARGS__))

#define RAND(x) (rand() % (x))

static int flag;

static EGLDisplay display = EGL_NO_DISPLAY;
static EGLSurface surface = EGL_NO_SURFACE;
static EGLContext context = EGL_NO_CONTEXT;
static ANativeWindow *window = NULL;

static EGLint width = 0;
static EGLint height = 0;
static EGLint format = 0;

static EGLBoolean initEGL()
{
    // Get default display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == display || EGL_SUCCESS != eglGetError()) {
        LOGE("eglGetDisplay failed");
        return EGL_FALSE;
    }

    // Initialize EGL display connection
    EGLint majorVer, minorVer;
    EGLBoolean status = eglInitialize(display, &majorVer, &minorVer);
    if (EGL_TRUE != status || EGL_SUCCESS != eglGetError()) {
        LOGE("eglInitialize failed");
        return EGL_FALSE;
    }
    LOGE("majorVer: %d, minorVer: %d", majorVer, minorVer);

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
        LOGE("eglChooseConfig failed");
        return EGL_FALSE;
    }

    // Create a new EGL window surface
    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (EGL_NO_SURFACE == surface) {
        LOGE("eglCreateWindowSurface failed");
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
        LOGE("eglCreateContext failed");
        return EGL_FALSE;
    }

    // Attach an EGL rendering context to EGL surfaces
    status = eglMakeCurrent(display, surface, surface, context);
    if (EGL_TRUE != status) {
        LOGE("eglMakeCurrent failed");
        return EGL_FALSE;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    LOGE("width: %d, height: %d", width, height);

    // If interval is set to a value of 0, buffer swaps are not synchronized to a video frame,
    // and the swap happens as soon as the render is complete.
    // eglSwapInterval(display, 0);

    return EGL_TRUE;
}


void *__gl_proc__(void *)
{
    srand(time(NULL));

    if (EGL_NO_DISPLAY == display) {
        LOGE("display is %#x, EGL_NO_DISPLAY", display);
    }

    if (EGL_NO_SURFACE == surface) {
        LOGE("surface is %#x, EGL_NO_SURFACE", surface);
    }

    if (EGL_NO_CONTEXT == context) {
        LOGE("context is %#x, EGL_NO_CONTEXT", context);
    }

    if (NULL == window) {
        LOGE("window is NULL");
    }

    flag = 1;
    while (flag) {
        float r = RAND(255) / 255.0f;
        float g = RAND(255) / 255.0f;
        float b = RAND(255) / 255.0f;
        LOGE("r: %f, g: %f, b: %f", r, g, b);

        // Just fill the screen with a color.
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        EGLBoolean status = eglSwapBuffers(display, surface);
        if (EGL_FALSE == status) {
            int errorCode = eglGetError();
            char *msg;
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
            case EGL_CONTEXT_LOST:
                msg = "EGL_CONTEXT_LOST";
                break;
            default:
                msg = "unknown";
                break;
            }
            LOGE("eglSwapBuffers failed, error code %#x, %s", errorCode, msg);
        }

        sleep(1);
    }
}

/*
 * Class:     org_yuki_demo_opengl_Renderer
 * Method:    onSurfaceCreated
 * Signature: (Landroid/view/Surface;)V
 */
JNIEXPORT void JNICALL Java_org_yuki_demo_opengl_Renderer_onSurfaceCreated
        (JNIEnv *env, jobject thiz, jobject surface)
{
    window = ANativeWindow_fromSurface(env, surface);
    EGLBoolean status = initEGL();

    if (EGL_TRUE == status) {
        pthread_t t;
        pthread_create(&t, NULL, __gl_proc__, NULL);
    }
}


/*
 * Class:     org_yuki_demo_opengl_Renderer
 * Method:    onSurfaceChanged
 * Signature: (Landroid/view/Surface;III)V
 */
JNIEXPORT void JNICALL Java_org_yuki_demo_opengl_Renderer_onSurfaceChanged
        (JNIEnv *env, jobject thiz, jobject surface, jint format, jint width, jint height)
{

}

/*
 * Class:     org_yuki_demo_opengl_Renderer
 * Method:    onSurfaceDestroyed
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_yuki_demo_opengl_Renderer_onSurfaceDestroyed
        (JNIEnv *, jobject)
{
    flag = 0;
}

#ifdef __cplusplus
}
#endif
