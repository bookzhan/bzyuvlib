package com.luoye.bzyuvlib;

import android.media.Image;
import android.os.Build;

import androidx.annotation.RequiresApi;

import java.nio.ByteBuffer;

/**
 * Created by zhandalin on 2019-11-01 17:39.
 * description:
 */
public class BZYUVUtil {
    static {
        System.loadLibrary("tbb");
        System.loadLibrary("bzyuvlib");
    }

    private byte[] outDataRGBA = null;
    private byte[] outDataBGRA = null;

    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public byte[] yuv420pToRGBA(Image image, boolean flipHorizontal, int rotate) {
        if (null == image) {
            return null;
        }
        if (null == outDataRGBA) {
            outDataRGBA = new byte[image.getWidth() * image.getHeight() * 4];
        }
        Image.Plane[] planes = image.getPlanes();
        BZYUVUtil.yuv420pToRGBA(planes[0].getBuffer(), planes[1].getBuffer(), planes[1].getPixelStride(), planes[2].getBuffer(), planes[2].getPixelStride(), outDataRGBA, image.getWidth(), image.getHeight(), flipHorizontal, rotate);
        return outDataRGBA;
    }

    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public byte[] yuv420pToBGRA(Image image, boolean flipHorizontal, int rotate) {
        if (null == image) {
            return null;
        }
        if (null == outDataBGRA) {
            outDataBGRA = new byte[image.getWidth() * image.getHeight() * 4];
        }
        Image.Plane[] planes = image.getPlanes();
        BZYUVUtil.yuv420pToBGRA(planes[0].getBuffer(), planes[1].getBuffer(), planes[1].getPixelStride(), planes[2].getBuffer(), planes[2].getPixelStride(), outDataBGRA, image.getWidth(), image.getHeight(), flipHorizontal, rotate);
        return outDataBGRA;
    }

    public static native int test();

    public static native int yuv420pToRGBA(ByteBuffer byteBufferY, ByteBuffer byteBufferU, int uPixelStride, ByteBuffer byteBufferV, int vPixelStride, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yuv420pToBGRA(ByteBuffer byteBufferY, ByteBuffer byteBufferU, int uPixelStride, ByteBuffer byteBufferV, int vPixelStride, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yv12ToARGB(byte[] yv12, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);

    public static native int yv12ToBGRA(byte[] yv12, byte[] outDate, int width, int height, boolean flipHorizontal, int rotate);
}
