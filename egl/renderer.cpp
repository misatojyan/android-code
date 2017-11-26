#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/native_window_jni.h>

#include "egl_utils.h"
#include "log.h"

#if defined(TAG)
#undef TAG
#endif

#define TAG "native-egl"

#define RAND(x) (rand() % (x))
#define R(c) ((c) & 0xFF)
#define G(c) (((c) >> 8) & 0xFF)
#define B(c) (((c) >> 16) & 0xFF)

static ANativeWindow *window = NULL;

#ifdef __cplusplus
extern "C" {
#endif

static GLuint loadShader(GLenum type, const GLchar *shaderCode)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);
    return shader;
}

static GLuint loadProgram(const GLchar *vertexShaderCode, const GLchar *fragmentShaderCode)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, loadShader(GL_VERTEX_SHADER, vertexShaderCode));
    glAttachShader(program, loadShader(GL_FRAGMENT_SHADER, fragmentShaderCode));
    glLinkProgram(program);
    return program;
}

// number of coordinates per vertex in this array
static const int COORDS_PER_VERTEX = 3;
static float triangleCoords[] = {   // in counterclockwise order:
        0.0f,  0.622008459f, 0.0f, // top
        -0.5f, -0.311004243f, 0.0f, // bottom left
        0.5f, -0.311004243f, 0.0f  // bottom right
};
// Set color with red, green, blue and alpha (opacity) values
float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

static char *vertexShaderCode = " \
attribute vec4 vPosition; \
void main() { \
    gl_Position = vPosition; \
}";

static char *fragmentShaderCode = " \
precision mediump float; \
uniform vec4 vColor; \
void main() { \
    gl_FragColor = vColor; \
}";

static int flag;
static int flag2;

pthread_mutex_t mutex;
pthread_cond_t cond;

void *__gl_proc__(void *)
{
    EGLBoolean status = eglUtils::initEGL(window);
    if (EGL_FALSE == status) {
        LOGE(TAG, "initEGL failed, error code %#x", eglGetError());
        return NULL;
    }

    srand(time(NULL));
    flag = 1;

    GLuint program = loadProgram(vertexShaderCode, fragmentShaderCode);
    glUseProgram(program);
    GLint positionHandle = glGetAttribLocation(program, "vPosition");
    GLint colorHandle = glGetUniformLocation(program, "vColor");

    int vertexStride = COORDS_PER_VERTEX * 4;
    int vertexCount = sizeof(triangleCoords) / COORDS_PER_VERTEX;

    while (flag) {
        float r = RAND(255) / 255.0f;
        float g = RAND(255) / 255.0f;
        float b = RAND(255) / 255.0f;
        LOGE(TAG, "r = %f, g = %f, b = %f", r, g, b);

        color[0] = r;
        color[1] = g;
        color[2] = b;

        // Just fill the screen with a color.
        // glClearColor(r, g, b, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);

        glEnableVertexAttribArray(positionHandle);
        /**
         * GLES20.glVertexAttribPointer(mPositionHandle, COORDS_PER_VERTEX,
                                 GLES20.GL_FLOAT, false,
                                 vertexStride, vertexBuffer);
         */
        glVertexAttribPointer(positionHandle, COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE, vertexStride, triangleCoords);
        glUniform4fv(colorHandle, 1, color);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glDisableVertexAttribArray(positionHandle);

        glFlush();
        EGLBoolean status = eglSwapBuffers(eglUtils::display, eglUtils::surface);
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
            LOGE(TAG, "eglSwapBuffers failed, error code %#x, %s", errorCode, msg);
        }

        /*pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);*/

//        sleep(1);
    }
}

void *__init_egl__(void *)
{
    EGLBoolean status = eglUtils::initEGL(window);
    if (EGL_FALSE == status) {
        LOGE(TAG, "initEGL failed, error code %#x", eglGetError());
        return NULL;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);


    pthread_t t;
    pthread_create(&t, NULL, __gl_proc__, NULL);
    // pthread_join(t, NULL);

    flag2 = 1;
    while (flag2) {
        LOGE(TAG, "lock");
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
        LOGE(TAG, "unlock");

        EGLBoolean status = eglSwapBuffers(eglUtils::display, eglUtils::surface);
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
            LOGE(TAG, "eglSwapBuffers failed, error code %#x, %s", errorCode, msg);
        }
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
    pthread_t t;
    pthread_create(&t, NULL, __gl_proc__, NULL);
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
    flag2 = 0;
}

#ifdef __cplusplus
}
#endif
