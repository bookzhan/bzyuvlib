package com.luoye.bzyuvlib;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;


public class Camera2Activity extends AppCompatActivity {
    private BZCamera2View bz_camera2_view;
    private ImageView iv_preview;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera2);
        iv_preview = findViewById(R.id.iv_preview);
        bz_camera2_view = findViewById(R.id.bz_camera2_view);
        bz_camera2_view.setOnPreviewBitmapListener(new BZCamera2View.OnPreviewBitmapListener() {
            @Override
            public void onPreviewBitmapListener(final Bitmap bitmap) {
                iv_preview.post(new Runnable() {
                    @Override
                    public void run() {
                        iv_preview.setImageBitmap(bitmap);
                    }
                });
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        bz_camera2_view.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        bz_camera2_view.onPause();
    }
}
