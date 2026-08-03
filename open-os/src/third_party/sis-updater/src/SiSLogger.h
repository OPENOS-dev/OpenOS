// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

#ifndef SRC_SISLOGGER_H_
#define SRC_SISLOGGER_H_

#include <stdarg.h>
#include <stdio.h>

// FOR_ANDROID_LOG & SDCARD_FILE_LOG is define in Android.mk (LOCAL_CFLAGS)
// #define FOR_ANDROID_LOG
// #define SDCARD_FILE_LOG
#define LOG_TIME

#ifdef FOR_ANDROID_LOG
#define LOG_TAG "SiSTouchjni native.cpp"
#include <android/log.h>
#endif

const int NULL_LOG_FLAG = 0x0;
const int ANDROID_LOG_FLAG = 0x1;
const int FILE_LOG_FLAG = 0x2;
const int ANDROID_AND_FILE_LOG_FLAG = 0x3;

int setLogInterface(int logInterface);
int getLogInterface();

void initialLogger();
void releaseLogger();

int LOGI(const char* format, ...);
int LOGE(const char* format, ...);

#ifdef LOG_TIME
int getCurrentTimeString(char* buf);
#endif

__attribute__((__format__(__printf__, 1, 0)))
int vLOGItoNull(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGItoLogcat(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGItoFile(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGItoLogcatAndFile(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGEtoNull(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGEtoLogcat(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGEtoFile(const char* format, va_list args);

__attribute__((__format__(__printf__, 1, 0)))
int vLOGEtoLogcatAndFile(const char* format, va_list args);

int LOGItoNull(const char* format, ...);
int LOGItoLogcat(const char* format, ...);
int LOGItoFile(const char* format, ...);
int LOGItoLogcatAndFile(const char* format, ...);

int LOGEtoNull(const char* format, ...);
int LOGEtoLogcat(const char* format, ...);
int LOGEtoFile(const char* format, ...);
int LOGEtoLogcatAndFile(const char* format, ...);

#endif  // SRC_SISLOGGER_H_
