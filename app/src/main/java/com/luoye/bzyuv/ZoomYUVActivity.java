package com.luoye.bzyuv;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzcamera.listener.OnCameraStateListener;
import com.luoye.bzyuvlib.BZYUVUtil;

import java.nio.ByteBuffer;

public class ZoomYUVActivity extends AppCompatActivity {
    private static final String TAG = "bz_ZoomYUVActivity";

    private BZCameraView bz_camera_view;
    private ImageView bz_image_view;
    private byte[] argbByteBuffer = null;
    private int index = 0;
    private Bitmap bitmap = null;
    private long totalTime = 0;
    private byte[] yuvBuffer = null;
    private byte[] zoomYuvBuffer = null;
    private int zoomWidth = 240;
    private int zoomHeight = 320;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_zoom_yuv);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        bz_camera_view = findViewById(R.id.bz_camera_view);
        bz_camera_view.setPreviewTargetSize(480, 640);
        bz_image_view = findViewById(R.id.bz_image_view);


        bz_camera_view.setPreviewFormat(ImageFormat.NV21);
        bz_camera_view.setNeedCallBackData(true);
        bz_camera_view.setOnCameraStateListener(new OnCameraStateListener() {
            @Override
            public void onPreviewSuccess(Camera camera, int width, int height) {

            }

            @Override
            public void onPreviewFail(String message) {

            }

            @Override
            public void onPreviewDataUpdate(byte[] data, int width, int height, int displayOrientation, int cameraId) {
                if (null == yuvBuffer) {
                    yuvBuffer = new byte[width * height * 3 / 2];
                }
                if (null == zoomYuvBuffer) {
                    zoomYuvBuffer = new byte[zoomWidth * zoomHeight * 3 / 2];
                }
                if (null == argbByteBuffer) {
                    argbByteBuffer = new byte[zoomWidth * zoomHeight * 4];
                }

                index++;
                long startTime = System.currentTimeMillis();
                BZYUVUtil.preHandleNV21(data, yuvBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);

                int tempWidth = width;
                int tempHeight = height;
                if (displayOrientation == 270 || displayOrientation == 90) {
                    tempWidth = height;
                    tempHeight = width;
                }
                BZYUVUtil.zoomYUV420(yuvBuffer, zoomYuvBuffer, tempWidth, tempHeight, zoomWidth, zoomHeight);


                BZYUVUtil.yuv420ToRGBA(zoomYuvBuffer, argbByteBuffer, zoomWidth, zoomHeight, false, 0);
                totalTime += (System.currentTimeMillis() - startTime);
                Log.d(TAG, "time cost=" + (totalTime / index));
                if (null == bitmap) {
                    bitmap = Bitmap.createBitmap(zoomWidth, zoomHeight, Bitmap.Config.ARGB_8888);
                }
                bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(argbByteBuffer));
                bz_image_view.post(new Runnable() {
                    @Override
                    public void run() {
                        bz_image_view.setImageBitmap(bitmap);
                    }
                });
            }

            @Override
            public void onCameraClose() {

            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        bz_camera_view.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        bz_camera_view.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d("onDestroy", "yuvFileOutputStream.close");
    }
}
