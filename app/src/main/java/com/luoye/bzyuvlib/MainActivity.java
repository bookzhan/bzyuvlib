package com.luoye.bzyuvlib;

import android.Manifest;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {


    private static final String TAG = "bz_MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onResume() {
        super.onResume();
        requestPermission();
    }

    private boolean requestPermission() {
        ArrayList<String> permissionList = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN && !PermissionUtil.isPermissionGranted(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {
            permissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        }
        if (!PermissionUtil.isPermissionGranted(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            permissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        }
        if (!PermissionUtil.isPermissionGranted(this, Manifest.permission.CAMERA)) {
            permissionList.add(Manifest.permission.CAMERA);
        }
        if (!PermissionUtil.isPermissionGranted(this, Manifest.permission.RECORD_AUDIO)) {
            permissionList.add(Manifest.permission.RECORD_AUDIO);
        }

        String[] permissionStrings = new String[permissionList.size()];
        permissionList.toArray(permissionStrings);

        if (permissionList.size() > 0) {
            PermissionUtil.requestPermission(this, permissionStrings, PermissionUtil.CODE_REQ_PERMISSION);
            return false;
        } else {
            Log.d(TAG, "All the required permissions are available");
            return true;
        }
    }

    public void Camera2Activity(View view) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            startActivity(new Intent(this, Camera2Activity.class));
        }
    }

    public void CameraActivity(View view) {
        startActivity(new Intent(this, CameraActivity.class));
    }

    public void Camera1YUVCropActivity(View view) {
        startActivity(new Intent(this, Camera1YUVCropActivity.class));
    }

    public void Camera2YUVCropActivity(View view) {
        startActivity(new Intent(this, Camera2YUVCropActivity.class));
    }

    public void zoomYUV(View view) {
        startActivity(new Intent(this, ZoomYUVActivity.class));
    }

    public void Bitmap2YUVActivity(View view) {
        startActivity(new Intent(this, Bitmap2YUVActivity.class));
    }

    public void GreyImageTestActivity(View view) {
        startActivity(new Intent(this, GreyImageTestActivity.class));
    }

    public void RGBAHandleActivity(View view) {
        startActivity(new Intent(this, RGBAHandleActivity.class));
    }

    public void YUV420ToNV21Activity(View view) {
        startActivity(new Intent(this, YUV420ToNV21Activity.class));
    }
}
