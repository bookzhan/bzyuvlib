package com.luoye.bzyuvlib;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;


/**
 * Created by admin on 2016/2/17.
 * 动态权限请求的相关工具类
 */
public class PermissionUtil {
    private static final String TAG = "PermissionUtil";
    public static final int CODE_REQ_PERMISSION = 1100;//权限请求
    public static final int CODE_REQ_AUDIO_PERMISSION = 601;
    public static final int CODE_REQ_CAMERA_PERMISSION = 602;

    /**
     * 权限请求
     *
     * @param activity
     * @return
     */
    public static void requestPermission(Activity activity, String[] permissionArr, int requestCode) {
        if (permissionArr != null) {
            ActivityCompat.requestPermissions(activity, permissionArr, requestCode);
        }

    }

    public static void requestPermission(Activity activity, String permissionArr, int requestCode) {
        if (permissionArr != null) {
            ActivityCompat.requestPermissions(activity, new String[]{permissionArr}, requestCode);
        }
    }

    public static void requestPermissionIFNot(Activity activity, String permissionArr, int requestCode) {
        if (permissionArr != null && !isPermissionGranted(activity, permissionArr)) {
            requestPermission(activity, permissionArr, requestCode);
        }
    }

    /**
     * 判断是否拥有该权限
     */
    public static boolean isPermissionGranted(Context context, String permission) {
        if (ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            return false;
        }
    }

}
