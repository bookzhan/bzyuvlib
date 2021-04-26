#include <jni.h>
#include <string>
#include "BZLogUtil.h"
#include "include/libyuv.h"
#include "bz_time.h"
#include <android/bitmap.h>

enum Pix_Format {
    RGBA, BGRA, YUV420, GREY
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
    } else if (pixFormat == Pix_Format::GREY) {
        memcpy(outData, finalYData, ySize);
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
                            jbyteArray out_data, jint width, jint height,
                            jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == byte_buffer_y || nullptr == byte_buffer_u || nullptr == byte_buffer_v ||
        u_pixel_stride <= 0 || v_pixel_stride <= 0) {
        BZLogUtil::logE(
                "nullptr == byte_buffer_y || nullptr == byte_buffer_u || nullptr == byte_buffer_v ||u_pixel_stride <= 0 || v_pixel_stride <= 0");
        return -1;
    }
    if (nullptr == out_data || width <= 0 || height <= 0) {
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
    auto *p_out_data = env->GetByteArrayElements(out_data, JNI_FALSE);

    int ySize = width * height;
    int yuvSize = ySize * 3 / 2;
    unsigned char *buffer = static_cast<unsigned char *>(malloc(yuvSize));
    libyuv::Android420ToI420(reinterpret_cast<const uint8_t *>(pYData), yRowStride,
                             reinterpret_cast<const uint8_t *>(pUData), uRowStride,
                             reinterpret_cast<const uint8_t *>(pVData), vRowStride,
                             u_pixel_stride,
                             buffer, width,
                             buffer + ySize, width / 2,
                             buffer + ySize + ySize / 4, width / 2, width, height);

    int ret = handle_conversion(buffer,
                                buffer + ySize,
                                buffer + ySize + ySize / 4,
                                reinterpret_cast<unsigned char *>(p_out_data), width,
                                height, flip_horizontal, rotate, pixFormat);;
    free(buffer);
    env->ReleaseByteArrayElements(out_data, p_out_data, JNI_FALSE);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420pToRGBA(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                jint y_row_stride, jobject byte_buffer_u,
                                                jint u_pixel_stride, jint u_row_stride,
                                                jobject byte_buffer_v, jint v_pixel_stride,
                                                jint v_row_stride, jbyteArray out_data,
                                                jint width, jint height, jboolean flip_horizontal,
                                                jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_data, width,
                                   height, flip_horizontal, rotate, Pix_Format::RGBA);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420pToBGRA(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                jint y_row_stride, jobject byte_buffer_u,
                                                jint u_pixel_stride, jint u_row_stride,
                                                jobject byte_buffer_v, jint v_pixel_stride,
                                                jint v_row_stride, jbyteArray out_data,
                                                jint width, jint height, jboolean flip_horizontal,
                                                jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_data, width,
                                   height, flip_horizontal, rotate, Pix_Format::BGRA);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_preHandleYUV420(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                                  jint y_row_stride, jobject byte_buffer_u,
                                                  jint u_pixel_stride, jint u_row_stride,
                                                  jobject byte_buffer_v, jint v_pixel_stride,
                                                  jint v_row_stride, jbyteArray out_data,
                                                  jint width, jint height,
                                                  jboolean flip_horizontal, jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_data, width,
                                   height, flip_horizontal, rotate, Pix_Format::YUV420);
}


int pretreatmentYV12Data(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                         jbyteArray out_data, jint width, jint height,
                         jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == yv12_ || nullptr == out_data) {
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
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_data, JNI_FALSE);
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
    env->ReleaseByteArrayElements(out_data, p_argb_byte_buffer, 0);
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToRGBA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_data, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentYV12Data(env, clazz, yv12_, out_data, width, height, flip_horizontal, rotate,
                                RGBA);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToBGRA(JNIEnv *env, jclass clazz, jbyteArray yv12_,
                                             jbyteArray out_data, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {

    return pretreatmentYV12Data(env, clazz, yv12_, out_data, width, height, flip_horizontal, rotate,
                                BGRA);
}


int pretreatmentNV21Data(JNIEnv *env, jclass clazz, jbyteArray nv21_,
                         jbyteArray out_data, jint width, jint height,
                         jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == nv21_ || nullptr == out_data) {
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
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_data, JNI_FALSE);
    if (nullptr == p_argb_byte_buffer) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int ySize = width * height;
    int yuvSize = ySize * 3 / 2;
    unsigned char *buffer = static_cast<unsigned char *>(malloc(yuvSize));

    int ret = libyuv::NV21ToI420(reinterpret_cast<const uint8_t *>(data_nv21), width,
                                 reinterpret_cast<const uint8_t *>(data_nv21 + ySize), width,
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
    env->ReleaseByteArrayElements(out_data, p_argb_byte_buffer, 0);
    return ret;
}

int pretreatmentYUV420Data(JNIEnv *env, jclass clazz, jbyteArray yuv420_,
                           jbyteArray out_data, jint width, jint height,
                           jboolean flip_horizontal, jint rotate, Pix_Format pixFormat) {
    if (nullptr == yuv420_ || nullptr == out_data) {
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
    auto *p_argb_byte_buffer = env->GetByteArrayElements(out_data, JNI_FALSE);
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
    env->ReleaseByteArrayElements(out_data, p_argb_byte_buffer, 0);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420ToRGBA(JNIEnv *env, jclass clazz, jbyteArray yuv420,
                                               jbyteArray out_data, jint width, jint height,
                                               jboolean flip_horizontal, jint rotate) {
    return pretreatmentYUV420Data(env, clazz, yuv420, out_data, width, height, flip_horizontal,
                                  rotate,
                                  RGBA);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420ToBGRA(JNIEnv *env, jclass clazz, jbyteArray yuv420,
                                               jbyteArray out_data, jint width, jint height,
                                               jboolean flip_horizontal, jint rotate) {
    return pretreatmentYUV420Data(env, clazz, yuv420, out_data, width, height, flip_horizontal,
                                  rotate,
                                  BGRA);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_nv21ToRGBA(JNIEnv *env, jclass clazz, jbyteArray nv21,
                                             jbyteArray out_data, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_data, width, height, flip_horizontal, rotate,
                                RGBA);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_nv21ToBGRA(JNIEnv *env, jclass clazz, jbyteArray nv21,
                                             jbyteArray out_data, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_data, width, height, flip_horizontal, rotate,
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
                                                jbyteArray out_data, jint width, jint height,
                                                jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_data, width, height, flip_horizontal, rotate,
                                Pix_Format::YUV420);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_preHandleYV12(JNIEnv *env, jclass clazz, jbyteArray yv12,
                                                jbyteArray out_data, jint width, jint height,
                                                jboolean flip_horizontal, jint rotate) {
    return pretreatmentYV12Data(env, clazz, yv12, out_data, width, height, flip_horizontal, rotate,
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
    int ret = libyuv::I420Scale(reinterpret_cast<const uint8_t *>(data_yuv), src_width,
                                reinterpret_cast<const uint8_t *>(data_yuv + ySrcSize),
                                src_width >> 1,
                                reinterpret_cast<const uint8_t *>(data_yuv + ySrcSize +
                                                                  ySrcSize / 4),
                                src_width >> 1,
                                src_width, src_height,
                                reinterpret_cast<uint8_t *>(yuv_dis), dis_width,
                                reinterpret_cast<uint8_t *>(yuv_dis + yDisSize), dis_width >> 1,
                                reinterpret_cast<uint8_t *>(yuv_dis + yDisSize + yDisSize / 4),
                                dis_width >> 1,
                                dis_width, dis_height, libyuv::FilterMode::kFilterNone);

    env->ReleaseByteArrayElements(src_, data_yuv, 0);
    env->ReleaseByteArrayElements(dis_, yuv_dis, 0);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_bitmapToYUV420(JNIEnv *env, jclass clazz, jobject bitmap,
                                                 jbyteArray dis_) {
    if (nullptr == bitmap || nullptr == dis_) {
        return -1;
    }
    auto *yuv_dis = env->GetByteArrayElements(dis_, JNI_FALSE);
    if (nullptr == yuv_dis) {
        BZLogUtil::logE("Get p_byte_buffer return null");
        return -1;
    }
    int ret = 0;
    void *pixels_color = NULL;

    AndroidBitmapInfo info;
    ret = AndroidBitmap_getInfo(env, bitmap, &info);
    if (ret < 0) {
        BZLogUtil::logE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return ret;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels_color)) < 0) {
        BZLogUtil::logE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return ret;
    }
    int dis_width = info.width;
    int dis_height = info.height;
    int64_t yDisSize = dis_width * dis_height;
    if (info.format == AndroidBitmapFormat::ANDROID_BITMAP_FORMAT_RGBA_8888) {
        ret = libyuv::ABGRToI420(reinterpret_cast<const uint8_t *>(pixels_color), dis_width * 4,
                                 reinterpret_cast<uint8_t *>(yuv_dis), dis_width,
                                 reinterpret_cast<uint8_t *>(yuv_dis + yDisSize), dis_width >> 1,
                                 reinterpret_cast<uint8_t *>(yuv_dis + yDisSize + yDisSize / 4),
                                 dis_width >> 1,
                                 dis_width, dis_height);
    } else if (info.format == AndroidBitmapFormat::ANDROID_BITMAP_FORMAT_RGB_565) {
        ret = libyuv::RGB565ToI420(reinterpret_cast<const uint8_t *>(pixels_color), dis_width * 2,
                                   reinterpret_cast<uint8_t *>(yuv_dis), dis_width,
                                   reinterpret_cast<uint8_t *>(yuv_dis + yDisSize), dis_width >> 1,
                                   reinterpret_cast<uint8_t *>(yuv_dis + yDisSize + yDisSize / 4),
                                   dis_width >> 1,
                                   dis_width, dis_height);
    } else {
        BZLogUtil::logE("The format is not supported");
    }


    AndroidBitmap_unlockPixels(env, bitmap);
    env->ReleaseByteArrayElements(dis_, yuv_dis, 0);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuv420ToGray(JNIEnv *env, jclass clazz, jobject byte_buffer_y,
                                               jint y_row_stride, jobject byte_buffer_u,
                                               jint u_pixel_stride, jint u_row_stride,
                                               jobject byte_buffer_v, jint v_pixel_stride,
                                               jint v_row_stride, jbyteArray out_data, jint width,
                                               jint height, jboolean flip_horizontal, jint rotate) {
    return pretreatmentYuv420pData(env, clazz, byte_buffer_y, y_row_stride, byte_buffer_u,
                                   u_pixel_stride, u_row_stride,
                                   byte_buffer_v, v_pixel_stride, v_row_stride, out_data, width,
                                   height, flip_horizontal, rotate, Pix_Format::GREY);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yv12ToGrey(JNIEnv *env, jclass clazz, jbyteArray yv12,
                                             jbyteArray out_data, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentYV12Data(env, clazz, yv12, out_data, width, height, flip_horizontal, rotate,
                                Pix_Format::GREY);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_nv21ToGrey(JNIEnv *env, jclass clazz, jbyteArray nv21,
                                             jbyteArray out_data, jint width, jint height,
                                             jboolean flip_horizontal, jint rotate) {
    return pretreatmentNV21Data(env, clazz, nv21, out_data, width, height, flip_horizontal, rotate,
                                Pix_Format::GREY);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_greyToRGBA(JNIEnv *env, jclass clazz, jbyteArray grey_,
                                             jbyteArray out_data_, jint width, jint height) {
    if (nullptr == grey_ || nullptr == out_data_) {
        BZLogUtil::logE("greyToRGBA nullptr == grey_ || nullptr == out_data_");
        return -1;
    }
    if (width * height != env->GetArrayLength(grey_)) {
        BZLogUtil::logE("width * height * 4!=grey.length");
        return -1;
    }
    if (width * height * 4 != env->GetArrayLength(out_data_)) {
        BZLogUtil::logE("width * height * 4!=out_data.length");
        return -1;
    }
    auto *grey = env->GetByteArrayElements(grey_, JNI_FALSE);
    auto *out_data = env->GetByteArrayElements(out_data_, JNI_FALSE);


    auto *p_buffer = reinterpret_cast<unsigned char *>(out_data);
    memset(p_buffer, 255, width * height * 4);
    auto *p_grey = reinterpret_cast<unsigned char *>(grey);
    for (int i = 0; i < width * height; ++i) {
        memset(p_buffer, *p_grey, 3);
        p_buffer += 4;
        p_grey++;
    }
    env->ReleaseByteArrayElements(grey_, grey, 0);
    env->ReleaseByteArrayElements(out_data_, out_data, 0);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_translationSingleChannel(JNIEnv *env, jclass clazz,
                                                           jbyteArray single_channel_data_,
                                                           jbyteArray out_data_, jint width,
                                                           jint height, jint translation_x,
                                                           jint translation_y) {
    if (nullptr == single_channel_data_ || nullptr == out_data_) {
        BZLogUtil::logE("greyToRGBA nullptr == grey_ || nullptr == out_data_");
        return -1;
    }
    if (std::abs(translation_x) > width || std::abs(translation_y) > height) {
        BZLogUtil::logE("std::abs(translation_x)>width||std::abs(translation_y)>height");
        return -1;
    }
    auto *single_channel_data = env->GetByteArrayElements(single_channel_data_, JNI_FALSE);
    auto *out_data = env->GetByteArrayElements(out_data_, JNI_FALSE);

    auto *p_single_channel_data = reinterpret_cast<unsigned char *>(single_channel_data);
    auto *temp_buffer = static_cast<unsigned char *>(malloc(width * height));
    memset(temp_buffer, 0, width * height);
    unsigned char *p_buffer = temp_buffer;

    if (translation_y < 0) {
        p_single_channel_data -= width * translation_y;
        for (int i = -translation_y; i < height; ++i) {
            if (translation_x < 0) {
                memcpy(p_buffer, p_single_channel_data - translation_x, width + translation_x);
            } else {
                memcpy(p_buffer + translation_x, p_single_channel_data, width - translation_x);
            }
            p_single_channel_data += width;
            p_buffer += width;
        }
    } else {
        p_buffer += width * translation_y;
        for (int i = 0; i < height - translation_y; ++i) {
            if (translation_x < 0) {
                memcpy(p_buffer, p_single_channel_data - translation_x, width + translation_x);
            } else {
                memcpy(p_buffer + translation_x, p_single_channel_data, width - translation_x);
            }
            p_single_channel_data += width;
            p_buffer += width;
        }
    }

    memcpy(out_data, temp_buffer, width * height);
    free(temp_buffer);

    env->ReleaseByteArrayElements(single_channel_data_, single_channel_data, 0);
    env->ReleaseByteArrayElements(out_data_, out_data, 0);
    return 0;
}

int handleRGBA(unsigned char *buffer,
               int stride, unsigned char *out_data, int width,
               int height, bool flip_horizontal, int rotate) {
    unsigned char *final_data = buffer;
    size_t buffer_size = static_cast<size_t>(width * height * 4);

    unsigned char *mirror_buffer = nullptr;
    if (flip_horizontal) {
        mirror_buffer = static_cast<unsigned char *>(malloc(buffer_size));
        memset(mirror_buffer, 0, buffer_size);
        libyuv::ARGBMirror(final_data, stride,
                           mirror_buffer, stride, width, height);
        final_data = mirror_buffer;
    }
    unsigned char *rotate_buffer = nullptr;
    if (rotate != 0) {
        rotate_buffer = static_cast<unsigned char *>(malloc(buffer_size));
        memset(rotate_buffer, 0, buffer_size);
        libyuv::RotationMode rotation_mode = libyuv::RotationMode::kRotate0;
        int stride_dis = stride;
        if (rotate == 90) {
            rotation_mode = libyuv::RotationMode::kRotate90;
            stride_dis = height * 4;
        } else if (rotate == 180) {
            rotation_mode = libyuv::RotationMode::kRotate180;
        } else if (rotate == 270) {
            rotation_mode = libyuv::RotationMode::kRotate270;
            stride_dis = height * 4;
        }
        libyuv::ARGBRotate(final_data, stride, rotate_buffer, stride_dis, width, height,
                           rotation_mode);
        final_data = rotate_buffer;
    }
    memcpy(out_data, final_data, buffer_size);

    if (nullptr != mirror_buffer) {
        free(mirror_buffer);
    }
    if (nullptr != rotate_buffer) {
        free(rotate_buffer);
    }
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_handleRGBA(JNIEnv *env, jclass clazz, jbyteArray buffer_,
                                             jint stride, jbyteArray out_data_, jint width,
                                             jint height, jboolean flip_horizontal, jint rotate) {
    auto *buffer = env->GetByteArrayElements(buffer_, JNI_FALSE);
    auto *out_data = env->GetByteArrayElements(out_data_, JNI_FALSE);
    int ret = handleRGBA(reinterpret_cast<unsigned char *>(buffer), stride,
                         reinterpret_cast<unsigned char *>(out_data), width, height,
                         flip_horizontal, rotate);

    env->ReleaseByteArrayElements(buffer_, buffer, 0);
    env->ReleaseByteArrayElements(out_data_, out_data, 0);
    return ret;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_handleRGBA4ByteBuffer(JNIEnv *env, jclass clazz,
                                                        jobject byte_buffer, jint stride,
                                                        jbyteArray out_data_, jint width,
                                                        jint height, jboolean flip_horizontal,
                                                        jint rotate) {
    auto *pData = (jbyte *) env->GetDirectBufferAddress(byte_buffer);
    auto *out_data = env->GetByteArrayElements(out_data_, JNI_FALSE);
    int ret = handleRGBA(reinterpret_cast<unsigned char *>(pData), stride,
                         reinterpret_cast<unsigned char *>(out_data), width, height,
                         flip_horizontal, rotate);
    env->ReleaseByteArrayElements(out_data_, out_data, 0);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuvI420ToNV21(JNIEnv *env, jclass clazz, jbyteArray yuv_i420_,
                                                jbyteArray out_data_, jint width, jint height) {
    auto *yuv_i420 = env->GetByteArrayElements(yuv_i420_, JNI_FALSE);
    auto *out_data = env->GetByteArrayElements(out_data_, JNI_FALSE);

    int yuv_size = width * height;

    int ret = libyuv::I420ToNV21(reinterpret_cast<const uint8_t *>(yuv_i420), width,
                                 reinterpret_cast<const uint8_t *>(yuv_i420 + yuv_size), width / 2,
                                 reinterpret_cast<const uint8_t *>(yuv_i420 + yuv_size * 5 / 4),
                                 width / 2,
                                 reinterpret_cast<uint8_t *>(out_data), width,
                                 reinterpret_cast<uint8_t *>(out_data + yuv_size), width,
                                 width, height);
    env->ReleaseByteArrayElements(yuv_i420_, yuv_i420, 0);
    env->ReleaseByteArrayElements(out_data_, out_data, 0);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzyuvlib_BZYUVUtil_yuvI420ToNV12(JNIEnv *env, jclass clazz, jbyteArray yuv_i420_,
                                                jbyteArray out_data_, jint width, jint height) {
    auto *yuv_i420 = env->GetByteArrayElements(yuv_i420_, JNI_FALSE);
    auto *out_data = env->GetByteArrayElements(out_data_, JNI_FALSE);

    int yuv_size = width * height;

    int ret = libyuv::I420ToNV12(reinterpret_cast<const uint8_t *>(yuv_i420), width,
                                 reinterpret_cast<const uint8_t *>(yuv_i420 + yuv_size), width / 2,
                                 reinterpret_cast<const uint8_t *>(yuv_i420 + yuv_size * 5 / 4),
                                 width / 2,
                                 reinterpret_cast<uint8_t *>(out_data), width,
                                 reinterpret_cast<uint8_t *>(out_data + yuv_size), width,
                                 width, height);
    env->ReleaseByteArrayElements(yuv_i420_, yuv_i420, 0);
    env->ReleaseByteArrayElements(out_data_, out_data, 0);
    return ret;
}