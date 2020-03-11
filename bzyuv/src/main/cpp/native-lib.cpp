#include <jni.h>
#include <string>
#include "BZLogUtil.h"
#include "include/libyuv.h"
#include "bz_time.h"
#include "tbb/parallel_for.h"

enum Pix_Format {
    RGBA, BGRA
};

int
handle_conversion(unsigned char *yData, unsigned char *uData, unsigned char *vData,
                  unsigned char *outData, int width, int height, bool flip_horizontal,
                  int rotate, Pix_Format pixFormat = RGBA) {
    if (nullptr == yData || nullptr == uData || nullptr == vData || nullptr == outData ||
        width <= 0 || height <= 0) {
        return -1;
    }
    if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
        BZLogUtil::logE("rotate != 9 && rotate != 90 && rotate != 180 && rotate != 270");
        return -1;
    }
    int ySize = width * height;
    int yuvSize = ySize * 3 / 2;
    unsigned char *finalYData = yData;
    unsigned char *finalUData = uData;
    unsigned char *finalVData = vData;

    unsigned char *mirror_i420_data = nullptr;
    if (flip_horizontal) {
        mirror_i420_data = static_cast<unsigned char *>(malloc(static_cast<size_t>(yuvSize)));
        memset(mirror_i420_data, 0, static_cast<size_t>(yuvSize));
        libyuv::I420Mirror(finalYData, width,
                           finalUData, width >> 1,
                           finalVData, width >> 1,
                           mirror_i420_data, width,
                           mirror_i420_data + ySize, width >> 1,
                           mirror_i420_data + ySize + ySize / 4, width >> 1,
                           width, height);
        finalYData = mirror_i420_data;
        finalUData = mirror_i420_data + ySize;
        finalVData = mirror_i420_data + ySize + ySize / 4;
    }

    int targetWidth = width;
    int targetHeight = height;
    if (rotate == 90 || rotate == 270) {
        targetWidth = height;
        targetHeight = width;
    }
    unsigned char *rotate_i420_data = nullptr;
    if (rotate != 0) {
        rotate_i420_data = static_cast<unsigned char *>(malloc(static_cast<size_t>(yuvSize)));
        memset(rotate_i420_data, 0, static_cast<size_t>(yuvSize));

        libyuv::I420Rotate(finalYData, width,
                           finalUData, width >> 1,
                           finalVData, width >> 1,
                           rotate_i420_data, targetWidth,
                           rotate_i420_data + ySize, targetWidth >> 1,
                           rotate_i420_data + ySize + ySize / 4,
                           targetWidth >> 1,
                           width, height,
                           (libyuv::RotationMode) rotate);

        finalYData = rotate_i420_data;
        finalUData = rotate_i420_data + ySize;
        finalVData = rotate_i420_data + ySize + ySize / 4;
    }
    if (pixFormat == Pix_Format::RGBA) {
        libyuv::I420ToARGB(finalYData, targetWidth,
                           finalUData, targetWidth / 2,
                           finalVData, targetWidth / 2,
                           outData, targetWidth * 4,
                           targetWidth, targetHeight);
    } else if (pixFormat == Pix_Format::BGRA) {
        libyuv::I420ToABGR(finalYData, targetWidth,
                           finalUData, targetWidth / 2,
                           finalVData, targetWidth / 2,
                           outData, targetWidth * 4,
                           targetWidth, targetHeight);
    }
    if (nullptr != mirror_i420_data) {
        free(mirror_i420_data);
    }
    if (nullptr != rotate_i420_data) {
        free(rotate_i420_data);
    }
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToARGB(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    if (nullptr == yv12_ || nullptr == out_date) {
        BZLogUtil::logE("nullptr == nv21_ || nullptr == byte_buffer_");
        return -1;
    }
    if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
        BZLogUtil::logE("rotate != 9 && rotate != 90 && rotate != 180 && rotate != 270");
        return -1;
    }
    jbyte *data_yv12 = env->GetByteArrayElements(yv12_, JNI_FALSE);
    if (nullptr == data_yv12) {
        BZLogUtil::logE("nullptr == data_nv21");
        return -1;
    }
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_date, JNI_FALSE);
    if (nullptr == p_argb_byte_buffer) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int ySize = width * height;
    int ret = handle_conversion(reinterpret_cast<unsigned char *>(data_yv12),
                                reinterpret_cast<unsigned char *>(data_yv12 + ySize),
                                reinterpret_cast<unsigned char *>(data_yv12 + ySize + ySize / 4),
                                reinterpret_cast<unsigned char *>(p_argb_byte_buffer), width,
                                height, flip_horizontal, rotate);
    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    env->ReleaseByteArrayElements(yv12_, data_yv12, JNI_FALSE);
    env->ReleaseByteArrayElements(out_date, p_argb_byte_buffer, JNI_FALSE);
    return ret;
}

int pretreatmentYuv420pData(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                            jobject byte_buffer_u, jint u_pixel_stride,
                            jobject byte_buffer_v, jint v_pixel_stride,
                            jbyteArray out_date, jint width, jint height,
                            jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == byte_buffer_y || nullptr == byte_buffer_u || nullptr == byte_buffer_v ||
        u_pixel_stride <= 0 || v_pixel_stride <= 0) {
        BZLogUtil::logE(
                "nullptr == byte_buffer_y || nullptr == byte_buffer_u || nullptr == byte_buffer_v ||u_pixel_stride <= 0 || v_pixel_stride <= 0");
        return -1;
    }
    if (nullptr == out_date || width <= 0 || height <= 0) {
        BZLogUtil::logE("nullptr == rgba || width <= 0 || height <= 0");
        return -1;
    }
    auto *pYData = (jbyte *) env->GetDirectBufferAddress(byte_buffer_y);
    if (!pYData) {
        BZLogUtil::logE("Get YData return null");
        return -1;
    }
    auto *pUData = (jbyte *) env->GetDirectBufferAddress(byte_buffer_u);
    if (!pUData) {
        BZLogUtil::logE("Get UData return null");
        return -1;
    }
    auto *pVData = (jbyte *) env->GetDirectBufferAddress(byte_buffer_v);
    if (!pVData) {
        BZLogUtil::logE("Get VData return null");
        return -1;
    }
    auto *p_argb_Data = env->GetByteArrayElements(out_date, JNI_FALSE);

    int ySize = width * height;
    int yuvSize = ySize * 3 / 2;


    jlong uBufferCapacity = env->GetDirectBufferCapacity(byte_buffer_u);
    jlong vBufferCapacity = env->GetDirectBufferCapacity(byte_buffer_v);
    int ret = 0;
    if (u_pixel_stride == v_pixel_stride && u_pixel_stride == 1) {
        ret = handle_conversion(reinterpret_cast<unsigned char *>(pYData),
                                reinterpret_cast<unsigned char *>(pVData),
                                reinterpret_cast<unsigned char *>(pUData),
                                reinterpret_cast<unsigned char *>(p_argb_Data), width,
                                height, flip_horizontal, rotate, pixFormat);
    } else {
        unsigned char *srcData = static_cast<unsigned char *>(malloc(yuvSize));
        memset(srcData, 0, yuvSize);
        memcpy(srcData, pYData, ySize);
        unsigned char *tempUP = srcData + ySize;
        unsigned char *tempVP = srcData + ySize + ySize / 4;
        if (uBufferCapacity == vBufferCapacity && u_pixel_stride == v_pixel_stride) {
//            tbb::parallel_for(tbb::blocked_range<size_t>(0, uBufferCapacity),
//                              [=](const tbb::blocked_range<size_t> &r) {
//                                  for (size_t i = r.begin(); i <= r.end(); i += u_pixel_stride) {
//                                      *(tempUP + i / 2) = *(pUData + i);
//                                      *(tempVP + i / 2) = *(pVData + i);
//                                  }
//                              });
            for (int i = 0; i < uBufferCapacity; i += u_pixel_stride) {
                *tempUP = *(pUData + i);
                *tempVP = *(pVData + i);
                tempUP++;
                tempVP++;
            }
        } else {
            for (int i = 0; i < uBufferCapacity; i += u_pixel_stride) {
                *tempUP = *(pUData + i);
                tempUP++;
            }
            for (int i = 0; i < vBufferCapacity; i += v_pixel_stride) {
                *tempVP = *(pVData + i);
                tempVP++;
            }
        }
        int ret = handle_conversion(srcData, srcData + ySize + ySize / 4, srcData + ySize,
                                    reinterpret_cast<unsigned char *>(p_argb_Data), width,
                                    height, flip_horizontal, rotate, pixFormat);
        free(srcData);
    }
    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    env->ReleaseByteArrayElements(out_date, p_argb_Data, JNI_FALSE);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420pToRGBA(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                jobject byte_buffer_u, jint u_pixel_stride,
                                                jobject byte_buffer_v, jint v_pixel_stride,
                                                jbyteArray out_date, jint width, jint height,
                                                jboolean flip_horizontal, jint rotate) {

    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, byte_buffer_u, u_pixel_stride,
                                   byte_buffer_v, v_pixel_stride, out_date, width, height,
                                   flip_horizontal, rotate, Pix_Format::RGBA);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420pToBGRA(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                jobject byte_buffer_u, jint u_pixel_stride,
                                                jobject byte_buffer_v, jint v_pixel_stride,
                                                jbyteArray out_date, jint width, jint height,
                                                jboolean flip_horizontal, jint rotate) {

    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, byte_buffer_u, u_pixel_stride,
                                   byte_buffer_v, v_pixel_stride, out_date, width, height,
                                   flip_horizontal, rotate, Pix_Format::BGRA);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToBGRA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    if (nullptr == yv12_ || nullptr == out_date) {
        BZLogUtil::logE("nullptr == nv21_ || nullptr == byte_buffer_");
        return -1;
    }
    if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
        BZLogUtil::logE("rotate != 9 && rotate != 90 && rotate != 180 && rotate != 270");
        return -1;
    }
    jbyte *data_yv12 = env->GetByteArrayElements(yv12_, nullptr);
    if (nullptr == data_yv12) {
        BZLogUtil::logE("nullptr == data_nv21");
        return -1;
    }
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_date, JNI_FALSE);
    if (nullptr == p_argb_byte_buffer) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int ySize = width * height;
    int ret = handle_conversion(reinterpret_cast<unsigned char *>(data_yv12),
                                reinterpret_cast<unsigned char *>(data_yv12 + ySize),
                                reinterpret_cast<unsigned char *>(data_yv12 + ySize + ySize / 4),
                                reinterpret_cast<unsigned char *>(p_argb_byte_buffer), width,
                                height, flip_horizontal, rotate, BGRA);
    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    env->ReleaseByteArrayElements(yv12_, data_yv12, 0);
    env->ReleaseByteArrayElements(out_date, p_argb_byte_buffer, 0);
    return ret;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_test(JNIEnv *env, jclass clazz) {
    int64_t startTime = getCurrentTime();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, 10000),
                      [=](const tbb::blocked_range<size_t> &r) {
                          for (size_t i = r.begin(); i != r.end(); ++i) {
                              BZLogUtil::logV("test index=%d", i);
                          }
                      });
    BZLogUtil::logE("test 耗时=%lld", getCurrentTime() - startTime);
    return 0;
}