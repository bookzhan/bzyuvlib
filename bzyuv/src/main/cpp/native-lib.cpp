#include <jni.h>
#include <string>
#include "BZLogUtil.h"
#include "include/libyuv.h"
#include "bz_time.h"

enum Pix_Format {
    RGBA, BGRA, YUV420
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
    } else if (pixFormat == Pix_Format::YUV420) {
        memcpy(outData, finalYData, ySize);
        memcpy(outData + ySize, finalUData, ySize / 4);
        memcpy(outData + ySize + ySize / 4, finalVData, ySize / 4);
    }
    if (nullptr != mirror_i420_data) {
        free(mirror_i420_data);
    }
    if (nullptr != rotate_i420_data) {
        free(rotate_i420_data);
    }
    return 0;
}

int pretreatmentYuv420pData(JNIEnv *env, jclass clazz, jobject byte_buffer_y, int yRowStride,
                            jobject byte_buffer_u, jint u_pixel_stride, int uRowStride,
                            jobject byte_buffer_v, jint v_pixel_stride, int vRowStride,
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
    auto *p_out_date = env->GetByteArrayElements(out_date, JNI_FALSE);

    int ySize = width * height;
    int yuvSize = ySize * 3 / 2;
    jlong uBufferCapacity = env->GetDirectBufferCapacity(byte_buffer_u);
    jlong vBufferCapacity = env->GetDirectBufferCapacity(byte_buffer_v);


    int ret = 0;
    if (u_pixel_stride == v_pixel_stride && u_pixel_stride == 1) {//YV12
        ret = handle_conversion(reinterpret_cast<unsigned char *>(pYData),
                                reinterpret_cast<unsigned char *>(pVData),
                                reinterpret_cast<unsigned char *>(pUData),
                                reinterpret_cast<unsigned char *>(p_out_date), width,
                                height, flip_horizontal, rotate, pixFormat);
    } else {//NV21
        unsigned long temp = pUData - pVData;
        unsigned char *buffer = static_cast<unsigned char *>(malloc(yuvSize));
        //Continuous memory storage
        if (temp == 1) {
            ret = libyuv::NV21ToI420(reinterpret_cast<const uint8 *>(pYData), yRowStride,
                                     reinterpret_cast<const uint8 *>(pVData),
                                     vRowStride,
                                     buffer, width,
                                     buffer + ySize, width / 2,
                                     buffer + ySize + ySize / 4,
                                     width / 2, width,
                                     height);
        } else if (temp == -1) {
            ret = libyuv::NV21ToI420(reinterpret_cast<const uint8 *>(pYData), yRowStride,
                                     reinterpret_cast<const uint8 *>(pUData), vRowStride,
                                     buffer, width,
                                     buffer + ySize, width / 2,
                                     buffer + ySize + ySize / 4, width / 2, width,
                                     height);
        } else if (uBufferCapacity == vBufferCapacity && u_pixel_stride == v_pixel_stride &&
                   u_pixel_stride == 2) {
            ret = libyuv::NV21ToI420(reinterpret_cast<const uint8 *>(pYData), yRowStride,
                                     reinterpret_cast<const uint8 *>(pVData), vRowStride,
                                     buffer, width,
                                     buffer + ySize, width / 2,
                                     buffer + ySize + ySize / 4,
                                     width / 2, width,
                                     height);
        } else {
            memcpy(buffer, pYData, ySize);
            unsigned char *tempUP = buffer + ySize;
            unsigned char *tempVP = buffer + ySize + ySize / 4;
            if (uBufferCapacity == vBufferCapacity && u_pixel_stride == v_pixel_stride) {
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
        }
        if (ret < 0) {
            BZLogUtil::logE("yuv ToI420 fail");
            return ret;
        }
        ret = handle_conversion(buffer, buffer + ySize + ySize / 4, buffer + ySize,
                                reinterpret_cast<unsigned char *>(p_out_date), width,
                                height, flip_horizontal, rotate, pixFormat);
        free(buffer);
    }
    env->ReleaseByteArrayElements(out_date, p_out_date, JNI_FALSE);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420pToRGBA(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                jint y_row_stride, jobject byte_buffer_u,
                                                jint u_pixel_stride, jint u_row_stride,
                                                jobject byte_buffer_v, jint v_pixel_stride,
                                                jint v_row_stride, jbyteArray out_date,
                                                jint width, jint height, jboolean flip_horizontal,
                                                jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_date, width,
                                   height, flip_horizontal, rotate, Pix_Format::RGBA);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420pToBGRA(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                jint y_row_stride, jobject byte_buffer_u,
                                                jint u_pixel_stride, jint u_row_stride,
                                                jobject byte_buffer_v, jint v_pixel_stride,
                                                jint v_row_stride, jbyteArray out_date,
                                                jint width, jint height, jboolean flip_horizontal,
                                                jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_date, width,
                                   height, flip_horizontal, rotate, Pix_Format::BGRA);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_preHandleYUV420p(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                   jint y_row_stride, jobject byte_buffer_u,
                                                   jint u_pixel_stride, jint u_row_stride,
                                                   jobject byte_buffer_v, jint v_pixel_stride,
                                                   jint v_row_stride, jbyteArray out_date,
                                                   jint width, jint height,
                                                   jboolean flip_horizontal, jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_date, width,
                                   height, flip_horizontal, rotate, Pix_Format::YUV420);
}

int pretreatmentYU12Data(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                         jbyteArray out_date, jint width, jint height,
                         jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
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
                                height, flip_horizontal, rotate, pixFormat);
    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    env->ReleaseByteArrayElements(yv12_, data_yv12, 0);
    env->ReleaseByteArrayElements(out_date, p_argb_byte_buffer, 0);
    return ret;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToRGBA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentYU12Data(env, clazz, yv12_, out_date, width, height, flip_horizontal, rotate,
                                RGBA);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToBGRA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {

    return pretreatmentYU12Data(env, clazz, yv12_, out_date, width, height, flip_horizontal, rotate,
                                BGRA);
}

int pretreatmentNV21Data(JNIEnv *env, jclass clazz, jbyteArray nv21_,
                         jbyteArray out_date, jint width, jint height,
                         jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == nv21_ || nullptr == out_date) {
        BZLogUtil::logE("nullptr == nv21_ || nullptr == byte_buffer_");
        return -1;
    }
    if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
        BZLogUtil::logE("rotate != 9 && rotate != 90 && rotate != 180 && rotate != 270");
        return -1;
    }
    jbyte *data_nv21 = env->GetByteArrayElements(nv21_, nullptr);
    if (nullptr == data_nv21) {
        BZLogUtil::logE("nullptr == data_nv21");
        return -1;
    }
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_date, JNI_FALSE);
    if (nullptr == p_argb_byte_buffer) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int ySize = width * height;
    int yuvSize = ySize * 3 / 2;
    unsigned char *buffer = static_cast<unsigned char *>(malloc(yuvSize));

    int ret = libyuv::NV21ToI420(reinterpret_cast<const uint8 *>(data_nv21), width,
                                 reinterpret_cast<const uint8 *>(data_nv21 + ySize), width,
                                 buffer, width,
                                 buffer + ySize, width / 2,
                                 buffer + ySize + ySize / 4,
                                 width / 2, width,
                                 height);
    if (ret < 0) {
        BZLogUtil::logE("NV21ToI420 fail");
    }
    ret = handle_conversion(buffer,
                            buffer + ySize + ySize / 4,
                            buffer + ySize,
                            reinterpret_cast<unsigned char *>(p_argb_byte_buffer), width,
                            height, flip_horizontal, rotate, pixFormat);
    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    free(buffer);
    env->ReleaseByteArrayElements(nv21_, data_nv21, 0);
    env->ReleaseByteArrayElements(out_date, p_argb_byte_buffer, 0);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_nv21ToRGBA(JNIEnv *env, jclass clazz, jbyteArray nv21,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_date, width, height, flip_horizontal, rotate,
                                RGBA);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_nv21ToBGRA(JNIEnv *env, jclass clazz, jbyteArray nv21,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_date, width, height, flip_horizontal, rotate,
                                BGRA);
}
