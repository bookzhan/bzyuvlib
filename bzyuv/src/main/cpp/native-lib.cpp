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
        //UV needs to be exchanged,Influenced by Big-Endian and Little-endian?
        libyuv::I420ToARGB(finalYData, targetWidth,
                           finalVData, targetWidth / 2,
                           finalUData, targetWidth / 2,
                           outData, targetWidth * 4,
                           targetWidth, targetHeight);
    } else if (pixFormat == Pix_Format::BGRA) {
        //UV needs to be exchanged,Influenced by Big-Endian and Little-endian?
        libyuv::I420ToABGR(finalYData, targetWidth,
                           finalVData, targetWidth / 2,
                           finalUData, targetWidth / 2,
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
        long temp = pUData - pVData;
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
        ret = handle_conversion(buffer, buffer + ySize, buffer + ySize + ySize / 4,
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
Java_com_luoye_bzyuvlib_BZYUVUtil_preHandleYUV420(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
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


int pretreatmentYV12Data(JNIEnv *env, jclass clazz, jbyteArray yv12_,
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
                                reinterpret_cast<unsigned char *>(data_yv12 + ySize + ySize / 4),
                                reinterpret_cast<unsigned char *>(data_yv12 + ySize),
                                reinterpret_cast<unsigned char *>(p_argb_byte_buffer), width,
                                height, flip_horizontal, rotate, pixFormat);

    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    env->ReleaseByteArrayElements(yv12_, data_yv12, 0);
    env->ReleaseByteArrayElements(out_date, p_argb_byte_buffer, 0);
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToRGBA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentYV12Data(env, clazz, yv12_, out_date, width, height, flip_horizontal, rotate,
                                RGBA);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToBGRA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_date, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {

    return pretreatmentYV12Data(env, clazz, yv12_, out_date, width, height, flip_horizontal, rotate,
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
                            buffer + ySize,
                            buffer + ySize + ySize / 4,
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

int pretreatmentYUV420Data(JNIEnv *env, jclass clazz, jbyteArray yuv420_,
                           jbyteArray out_date, jint width, jint height,
                           jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == yuv420_ || nullptr == out_date) {
        BZLogUtil::logE("nullptr == nv21_ || nullptr == byte_buffer_");
        return -1;
    }
    if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
        BZLogUtil::logE("rotate != 9 && rotate != 90 && rotate != 180 && rotate != 270");
        return -1;
    }
    jbyte *data_yuv420 = env->GetByteArrayElements(yuv420_, nullptr);
    if (nullptr == data_yuv420) {
        BZLogUtil::logE("nullptr == data_yuv420");
        return -1;
    }
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_date, JNI_FALSE);
    if (nullptr == p_argb_byte_buffer) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int ySize = width * height;
    int ret = handle_conversion(reinterpret_cast<unsigned char *>(data_yuv420),
                                reinterpret_cast<unsigned char *>(data_yuv420 + ySize),
                                reinterpret_cast<unsigned char *>(data_yuv420 + ySize + ySize / 4),
                                reinterpret_cast<unsigned char *>(p_argb_byte_buffer), width,
                                height, flip_horizontal, rotate, pixFormat);
    if (ret < 0) {
        BZLogUtil::logE("handle_conversion fail");
    }
    env->ReleaseByteArrayElements(yuv420_, data_yuv420, 0);
    env->ReleaseByteArrayElements(out_date, p_argb_byte_buffer, 0);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420ToRGBA(JNIEnv *env, jclass clazz, jbyteArray yuv420,
                                               jbyteArray out_date, jint width, jint height,
                                               jboolean flip_horizontal, jint rotate) {
    return pretreatmentYUV420Data(env, clazz, yuv420, out_date, width, height, flip_horizontal,
                                  rotate,
                                  RGBA);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420ToBGRA(JNIEnv *env, jclass clazz, jbyteArray yuv420,
                                               jbyteArray out_date, jint width, jint height,
                                               jboolean flip_horizontal, jint rotate) {
    return pretreatmentYUV420Data(env, clazz, yuv420, out_date, width, height, flip_horizontal,
                                  rotate,
                                  BGRA);
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
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_cropNV21(JNIEnv *env, jclass type, jbyteArray src_,
                                           jbyteArray dis_, jint srcWidth, jint srcHeight,
                                           jint startX, jint startY, jint disWidth,
                                           jint disHeight) {
    if (NULL == src_ || NULL == dis_ || startX < 0 || startY < 0 ||
        startX + disWidth > srcWidth || startY + disHeight > srcHeight) {
        BZLogUtil::logE(
                "cropNV21 param is error NULL == src_ || NULL == dis_ || startX < 0 || startY < 0 ||startX + disWidth > srcWidth || startY + disHeight > srcHeight");
        return -1;
    }
    jsize srcLength = env->GetArrayLength(src_);
    if (srcLength < srcWidth * srcHeight * 3 / 2) {
        BZLogUtil::logE("srcLength < srcWidth * srcHeight * 3 / 2");
        return -1;
    }
    jsize disLength = env->GetArrayLength(dis_);
    if (disLength < disWidth * disHeight * 3 / 2) {
        BZLogUtil::logE("disLength < disWidth * disHeight * 3 / 2");
        return -1;
    }


    jbyte *src = env->GetByteArrayElements(src_, NULL);
    jbyte *dis = env->GetByteArrayElements(dis_, NULL);

    if (nullptr == src || nullptr == dis) {
        BZLogUtil::logE("nullptr == src || nullptr == dis");
        return -1;
    }

    //Align
    startY = startY / 2 * 2;
    startX = startX / 2 * 2;
    disWidth = disWidth / 2 * 2;
    disHeight = disHeight / 2 * 2;

    //The x and y values ​​are reversed
//    startY = srcHeight - disHeight - startY;
//    startX = srcWidth - disWidth - startX;


    signed char *srcStartP = src + srcWidth * startY;
    signed char *disStartP = dis;

    //copyY
    for (int i = 0; i < disHeight; ++i) {
        memcpy(disStartP, srcStartP + startX, (size_t) disWidth);
        disStartP += disWidth;
        srcStartP += srcWidth;
    }
    //This is where the UV starts
    srcStartP = src + srcWidth * srcHeight + srcWidth * startY / 2;
    //copyUV
    for (int i = 0; i < disHeight / 2; ++i) {
        memcpy(disStartP, srcStartP + startX, (size_t) disWidth);
        disStartP += disWidth;
        srcStartP += srcWidth;
    }

    env->ReleaseByteArrayElements(src_, src, 0);
    env->ReleaseByteArrayElements(dis_, dis, 0);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_cropYUV420(JNIEnv *env, jclass type, jbyteArray src_,
                                             jbyteArray dis_, jint srcWidth, jint srcHeight,
                                             jint startX, jint startY, jint disWidth,
                                             jint disHeight) {
    if (NULL == src_ || NULL == dis_ || startX < 0 || startY < 0 ||
        startX + disWidth > srcWidth || startY + disHeight > srcHeight) {
        BZLogUtil::logE(
                "cropYV12 param is error NULL == src_ || NULL == dis_ || startX < 0 || startY < 0 ||startX + disWidth > srcWidth || startY + disHeight > srcHeight");
        return -1;
    }
    jsize srcLength = env->GetArrayLength(src_);
    if (srcLength < srcWidth * srcHeight * 3 / 2) {
        BZLogUtil::logE("srcLength < srcWidth * srcHeight * 3 / 2");
        return -1;
    }
    jsize disLength = env->GetArrayLength(dis_);
    if (disLength < disWidth * disHeight * 3 / 2) {
        BZLogUtil::logE("disLength < disWidth * disHeight * 3 / 2");
        return -1;
    }


    jbyte *src = env->GetByteArrayElements(src_, NULL);
    jbyte *dis = env->GetByteArrayElements(dis_, NULL);

    if (nullptr == src || nullptr == dis) {
        BZLogUtil::logE("nullptr == src || nullptr == dis");
        return -1;
    }

    //Align
    startY = startY / 2 * 2;
    startX = startX / 2 * 2;
    disWidth = disWidth / 2 * 2;
    disHeight = disHeight / 2 * 2;

    //The x and y values ​​are reversed
//    startY = srcHeight - disHeight - startY;
//    startX = srcWidth - disWidth - startX;


    signed char *srcStartP = src + srcWidth * startY;
    signed char *disStartP = dis;

    //copyY
    for (int i = 0; i < disHeight; ++i) {
        memcpy(disStartP, srcStartP + startX, (size_t) disWidth);
        disStartP += disWidth;
        srcStartP += srcWidth;
    }
    //copy Y or V
    srcStartP = src + srcWidth * srcHeight + srcWidth * startY / 4;
    for (int i = 0; i < disHeight / 2; ++i) {
        memcpy(disStartP, srcStartP + startX / 2, (size_t) disWidth);
        disStartP += disWidth / 2;
        srcStartP += srcWidth / 2;
    }
    //copy Y or V
    srcStartP = src + srcWidth * srcHeight * 5 / 4 + srcWidth * startY / 4;
    for (int i = 0; i < disHeight / 2; ++i) {
        memcpy(disStartP, srcStartP + startX / 2, (size_t) disWidth);
        disStartP += disWidth / 2;
        srcStartP += srcWidth / 2;
    }

    env->ReleaseByteArrayElements(src_, src, 0);
    env->ReleaseByteArrayElements(dis_, dis, 0);
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_preHandleNV21(JNIEnv *env, jclass clazz, jbyteArray nv21,
                                                jbyteArray out_date, jint width, jint height,
                                                jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_date, width, height, flip_horizontal, rotate,
                                Pix_Format::YUV420);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_preHandleYV12(JNIEnv *env, jclass clazz, jbyteArray yv12,
                                                jbyteArray out_date, jint width, jint height,
                                                jboolean flip_horizontal, jint rotate) {
    return pretreatmentYV12Data(env, clazz, yv12, out_date, width, height, flip_horizontal, rotate,
                                Pix_Format::YUV420);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_zoomYUV420(JNIEnv *env, jclass clazz, jbyteArray src_,
                                             jbyteArray dis_, jint src_width, jint src_height,
                                             jint dis_width, jint dis_height) {
    if (src_width <= 0 || src_height <= 0 || dis_width <= 0 || dis_height <= 0) {
        return -1;
    }
    jbyte *data_yuv = env->GetByteArrayElements(src_, nullptr);
    if (nullptr == data_yuv) {
        BZLogUtil::logE("nullptr == data_yuv");
        return -1;
    }
    auto *yuv_dis = env->GetByteArrayElements(dis_, JNI_FALSE);
    if (nullptr == yuv_dis) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int64_t ySrcSize = src_width * src_height;
    int64_t yDisSize = dis_width * dis_height;
    int ret = libyuv::I420Scale(reinterpret_cast<const uint8 *>(data_yuv), src_width,
                                reinterpret_cast<const uint8 *>(data_yuv + ySrcSize),
                                src_width >> 1,
                                reinterpret_cast<const uint8 *>(data_yuv + ySrcSize + ySrcSize / 4),
                                src_width >> 1,
                                src_width, src_height,
                                reinterpret_cast<uint8 *>(yuv_dis), dis_width,
                                reinterpret_cast<uint8 *>(yuv_dis + yDisSize), dis_width >> 1,
                                reinterpret_cast<uint8 *>(yuv_dis + yDisSize + yDisSize / 4),
                                dis_width >> 1,
                                dis_width, dis_height, libyuv::FilterMode::kFilterNone);

    return ret;
}