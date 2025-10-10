package com.luoye.bzyuv;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.bzcommon.utils.BZPermissionUtil;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "bz_MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        BZPermissionUtil.requestPermissionIfNot(this, Manifest.permission.CAMERA, BZPermissionUtil.CODE_REQ_PERMISSION);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    private boolean noPermission() {
        if (!BZPermissionUtil.requestPermissionIfNot(this, Manifest.permission.CAMERA, BZPermissionUtil.CODE_REQ_PERMISSION)) {
            Toast.makeText(this, "Please give App sufficient permissions", Toast.LENGTH_LONG).show();
            return true;
        }
        return false;
    }

    public void Camera2Activity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, Camera2Activity.class));
    }

    public void CameraActivity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, CameraActivity.class));
    }

    public void Camera1YUVCropActivity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, Camera1YUVCropActivity.class));
    }

    public void Camera2YUVCropActivity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, Camera2YUVCropActivity.class));
    }

    public void zoomYUV(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, ZoomYUVActivity.class));
    }

    public void Bitmap2YUVActivity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, Bitmap2YUVActivity.class));
    }

    public void GreyImageTestActivity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, GreyImageTestActivity.class));
    }

    public void RGBAHandleActivity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, RGBAHandleActivity.class));
    }

    public void YUV420ToNV21Activity(View view) {
        if (noPermission()) {
            return;
        }
        startActivity(new Intent(this, YUV420ToNV21Activity.class));
    }
}
