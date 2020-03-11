//
// Created by luoye on 2017/3/16.
//

#ifndef BZFFMPEG_BZLOG_H
#define BZFFMPEG_BZLOG_H

#include <stdio.h>
#include <android/log.h>

#define LOG_BUF_SIZE 1024
#define BZLOG_TAG "bz_"

class BZLogUtil {
public:
    static bool enableLog;

    static void logV(const char *fmt, ...);

    static void logD(const char *fmt, ...);

    static void logW(const char *fmt, ...);

    static void logE(const char *fmt, ...);
};

#endif //BZFFMPEG_BZLOG_H
