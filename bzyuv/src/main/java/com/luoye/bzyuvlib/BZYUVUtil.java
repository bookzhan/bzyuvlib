package com.luoye.bzyuvlib;

import android.graphics.Bitmap;
import android.media.Image;
import android.os.Build;


import java.nio.ByteBuffer;

/**
 * Created by zhandalin on 2019-11-01 17:39.
 * description:
 */
public class BZYUVUtil {
    static {
        System.loadLibrary("bzyuvlib");
    }

    private byte[] outDataRGBA = null;
    private byte[] outDataBGRA = null;
    private byte[] outYUV420 = null;
    private int lastWidth = 0;
    private int lastHeight = 0;

    public byte[] yuv420pToRGBA(Image image, boolean flipHorizontal, int rotate) {
        if (null == image || Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return null;
        }
        if (null == outDataRGBA || image.getWidth() != lastWidth || image.getHeight() != lastHeight) {
            outDataRGBA = new byte[image.getWidth() * image.getHeight() * 4];
            lastWidth = image.getWidth();
            lastHeight = image.getHeight();
        }
        Image.Plane[] planes = image.getPlanes();
        BZYUVUtil.yuv420pToRGBA(planes[0].getBuffer(), planes[0].getRowStride(), planes[1].getBuffer(), planes[1].getPixelStride(), planes[1].getRowStride(), planes[2].getBuffer(), planes[2].getPixelStride(), planes[2].getRowStride(), outDataRGBA, image.getWidth(), image.getHeight(), flipHorizontal, rotate);
        return outDataRGBA;
    }

    public byte[] yuv420pToBGRA(Image image, boolean flipHorizontal, int rotate) {
        if (null == image || Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return null;
        }
        if (null == outDataBGRA || image.getWidth() != lastWidth || image.getHeight() != lastHeight) {
            outDataBGRA = new byte[image.getWidth() * image.getHeight() * 4];
            lastWidth = image.getWidth();
            lastHeight = image.getHeight();
        }
        Image.Plane[] planes = image.getPlanes();
        BZYUVUtil.yuv420pToBGRA(planes[0].getBuffer(), planes[0].getRowStride(), planes[1].getBuffer(), planes[1].getPixelStride(), planes[1].getRowStride(), planes[2].getBuffer(), planes[2].getPixelStride(), planes[2].getRowStride(), outDataBGRA, image.getWidth(), image.getHeight(), flipHorizontal, rotate);
        return outDataBGRA;
    }

    public byte[] preHandleYUV420(Image image, boolean flipHorizontal, int rotate) {
        if (null == image || Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return null;
        }
        if (null == outYUV420 || image.getWidth() != lastWidth || image.getHeight() != lastHeight) {
            outYUV420 = new byte[image.getWidth() * image.getHeight() * 3 / 2];
            lastWidth = image.getWidth();
            lastHeight = image.getHeight();
        }
        Image.Plane[] planes = image.getPlanes();
        BZYUVUtil.preHandleYUV420(planes[0].getBuffer(), planes[0].getRowStride(), planes[1].getBuffer(), planes[1].getPixelStride(), planes[1].getRowStride(), planes[2].getBuffer(), planes[2].getPixelStride(), planes[2].getRowStride(), outYUV420, image.getWidth(), image.getHeight(), flipHorizontal, rotate);
        return outYUV420;
    }

    public static native int preHandleYUV420(ByteBuffer byteBufferY, int yRowStride, ByteBuffer byteBufferU, int uPixelStride, int uRowStride, ByteBuffer byteBufferV, int vPixelStride, int vRowStride, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yuv420pToRGBA(ByteBuffer byteBufferY, int yRowStride, ByteBuffer byteBufferU, int uPixelStride, int uRowStride, ByteBuffer byteBufferV, int vPixelStride, int vRowStride, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yuv420pToBGRA(ByteBuffer byteBufferY, int yRowStride, ByteBuffer byteBufferU, int uPixelStride, int uRowStride, ByteBuffer byteBufferV, int vPixelStride, int vRowStride, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yv12ToRGBA(byte[] yv12, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yv12ToBGRA(byte[] yv12, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int nv21ToRGBA(byte[] nv21, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int nv21ToBGRA(byte[] nv21, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yuv420ToRGBA(byte[] yuv420, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yuv420ToBGRA(byte[] yuv420, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    /**
     * @param outDate YUVI420
     */
    public static native int preHandleNV21(byte[] nv21, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    /**
     * @param outDate YUVI420
     */
    public static native int preHandleYV12(byte[] yv12, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);


    public static native int cropNV21(byte[] src, byte[] dis, int srcWidth, int srcHeight, int startX, int startY, int disWidth, int disHeight);

    public static native int cropYUV420(byte[] src, byte[] dis, int srcWidth, int srcHeight, int startX, int startY, int disWidth, int disHeight);

    public static native int zoomYUV420(byte[] src, byte[] dis, int srcWidth, int srcHeight, int disWidth, int disHeight);

    public static native int bitmapToYUV420(Bitmap bitmap, byte[] dis);

}
