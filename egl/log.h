//
// Created by yuki on 17-11-1.
//

#ifndef _LOG_H_
#define _LOG_H_

#include <android/log.h>

#if defined(LOGE)
#undef LOGE
#endif

#if defined(DEBUG)
#define LOGE(tag, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__))
#else
#define LOGE(tag, ...)
#endif

#endif //_LOG_H_
