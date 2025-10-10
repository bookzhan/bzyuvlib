package com.luoye.bzyuv;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;


public class PermissionUtil {
    private static final String TAG = "PermissionUtil";
    public static final int CODE_REQ_PERMISSION = 1100;
    public static final int CODE_REQ_AUDIO_PERMISSION = 601;
    public static final int CODE_REQ_CAMERA_PERMISSION = 602;


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

    public static boolean isPermissionGranted(Context context, String permission) {
        if (ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            return false;
        }
    }

}
