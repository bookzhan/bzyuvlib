/**
 * Created by zhandalin on 2017-04-21 11:26.
 * 说明:获取时间
 */

#ifndef BZFFMPEG_BZ_TIME_H
#define BZFFMPEG_BZ_TIME_H

#include <cstdint>
#include <sys/time.h>


//返回当前时间的微妙值
int64_t getMicrosecondTime();

//返回当前时间的毫秒值
int64_t getCurrentTime();

#endif //BZFFMPEG_BZ_TIME_H
