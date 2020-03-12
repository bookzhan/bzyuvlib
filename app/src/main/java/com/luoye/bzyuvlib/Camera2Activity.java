package com.luoye.bzyuvlib;

import android.graphics.Bitmap;
import android.hardware.camera2.CameraDevice;
import android.media.Image;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzcamera.BZCamera2View;

import java.nio.ByteBuffer;


public class Camera2Activity extends AppCompatActivity {
    private static final String TAG = "bz_Camera2Activity";
    private BZCamera2View bz_camera2_view;
    private ImageView iv_preview;
    private BZYUVUtil bzyuvUtil;
    private Bitmap bitmap;
    private long spaceTime;
    private int index;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera2);
        iv_preview = findViewById(R.id.iv_preview);
        bz_camera2_view = findViewById(R.id.bz_camera2_view);
        bz_camera2_view.setCheckCameraCapacity(false);
        bzyuvUtil = new BZYUVUtil();
        bz_camera2_view.setOnStatusChangeListener(new BZCamera2View.OnStatusChangeListener() {
            @Override
            public void onPreviewSuccess(CameraDevice mCameraDevice, int width, int height) {

            }

            @Override
            public void onImageAvailable(Image image, int displayOrientation, float fps) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                    long startTime = System.currentTimeMillis();
                    byte[] bytes = bzyuvUtil.yuv420pToRGBA(image, true, displayOrientation);
                    int height = image.getHeight();
                    int width = image.getWidth();
                    spaceTime += (System.currentTimeMillis() - startTime);
                    index++;
                    Log.d(TAG, "平均 yuv转换 耗时=" + (spaceTime / index) + " bitmap.width=" + width + " height=" + height);
                    if (null == bitmap) {
                        if (displayOrientation == 270 || displayOrientation == 90) {
                            bitmap = Bitmap.createBitmap(height, width, Bitmap.Config.ARGB_8888);
                        } else {
                            bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                        }
                    }
                    bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(bytes));
                    iv_preview.post(new Runnable() {
                        @Override
                        public void run() {
                            iv_preview.setImageBitmap(bitmap);
                        }
                    });
                }
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
