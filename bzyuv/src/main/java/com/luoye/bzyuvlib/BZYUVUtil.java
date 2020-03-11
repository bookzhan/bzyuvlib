package com.luoye.bzyuvlib;

import java.nio.ByteBuffer;

/**
 * Created by zhandalin on 2019-11-01 17:39.
 * description:
 */
public class BZYUVUtil {
    static {
        System.loadLibrary("bzyuvlib");
    }

    public static native int yuv420pToRGBA(ByteBuffer byteBufferY, ByteBuffer byteBufferU, int uPixelStride, ByteBuffer byteBufferV, int vPixelStride, ByteBuffer argb, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yuv420pToBGRA(ByteBuffer byteBufferY, ByteBuffer byteBufferU, int uPixelStride, ByteBuffer byteBufferV, int vPixelStride, ByteBuffer argb, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yv12ToARGB(byte[] yv12, ByteBuffer argbByteBuffer, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yv12ToBGRA(byte[] yv12, ByteBuffer argbByteBuffer, int width, int height, boolean flipHorizontal, int rotate);
}
