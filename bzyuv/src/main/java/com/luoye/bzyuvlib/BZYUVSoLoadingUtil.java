package com.luoye.bzyuvlib;

import android.util.Log;

/**
 * Created by bookzhan on 2024−03-11 22:43.
 * description:
 */
public class BZYUVSoLoadingUtil {
    private static SoLoadingListener mSoLoadingListener;

    public static void setSoLoadingListener(SoLoadingListener soLoadingListener) {
        mSoLoadingListener = soLoadingListener;
    }

    public static void loadLibrary(String soName) {
        try {
            System.loadLibrary(soName);
        } catch (Throwable throwable) {
            Log.e(BZYUVSoLoadingUtil.class.getSimpleName(), "loadSo fail soName=" + soName, throwable);
            if (null != mSoLoadingListener) {
                mSoLoadingListener.onSoLoadingFail(soName);
            }
        }
    }

    public interface SoLoadingListener {
        void onSoLoadingFail(String soName);
    }
}
