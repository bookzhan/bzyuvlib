//
/**
 * Created by zhandalin on 2018-09-10 11:26.
 * 说明:
 */
//

#include "BZLogUtil.h"

bool BZLogUtil::enableLog = true;

void BZLogUtil::logV(const char *fmt, ...) {
    if (enableLog) {
        va_list ap;
        char buf[LOG_BUF_SIZE];
        va_start(ap, fmt);
        vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
        va_end(ap);
        __android_log_write(ANDROID_LOG_VERBOSE, BZLOG_TAG, buf);
    }
}

void BZLogUtil::logD(const char *fmt, ...) {
    if (enableLog) {
        va_list ap;
        char buf[LOG_BUF_SIZE];
        va_start(ap, fmt);
        vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
        va_end(ap);
        __android_log_write(ANDROID_LOG_DEBUG, BZLOG_TAG, buf);
    }
}

void BZLogUtil::logE(const char *fmt, ...) {
    if (enableLog) {
        va_list ap;
        char buf[LOG_BUF_SIZE];
        va_start(ap, fmt);
        vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
        va_end(ap);
        __android_log_write(ANDROID_LOG_ERROR, BZLOG_TAG, buf);
    }
}

void BZLogUtil::logW(const char *fmt, ...) {
    if (enableLog) {
        va_list ap;
        char buf[LOG_BUF_SIZE];
        va_start(ap, fmt);
        vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
        va_end(ap);
        __android_log_write(ANDROID_LOG_WARN, BZLOG_TAG, buf);
    }
}
