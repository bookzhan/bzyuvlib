package com.luoye.bzyuvlib;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzcamera.listener.CameraStateListener;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class CameraActivity extends AppCompatActivity {

    private BZCameraView bz_camera_view;
    private ImageView bz_image_view;
    private ByteBuffer argbByteBuffer = null;
    private int index = 0;
    private Bitmap bitmap = null;
    private long totalTime = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);
        bz_camera_view = findViewById(R.id.bz_camera_view);
        bz_camera_view.setPreviewTargetSize(480, 640);
        bz_image_view = findViewById(R.id.bz_image_view);


//        bz_image_view.setScaleX(-1);
//        bz_image_view.setScaleY(-1);
        bz_camera_view.setPreviewFormat(ImageFormat.YV12);
        bz_camera_view.setNeedCallBackData(true);
        bz_camera_view.setCameraStateListener(new CameraStateListener() {
            @Override
            public void onPreviewSuccess(Camera camera, int width, int height) {

            }

            @Override
            public void onPreviewFail(String message) {

            }

            @Override
            public void onPreviewDataUpdate(byte[] data, int width, int height, int displayOrientation, int cameraId) {
                if (null == argbByteBuffer) {
                    argbByteBuffer = ByteBuffer.allocateDirect(width * height * 4);
                    argbByteBuffer.order(ByteOrder.nativeOrder());
                    argbByteBuffer.position(0);
                }
                index++;
                long startTime = System.currentTimeMillis();
                BZYUVUtil.yv12ToARGB(data, argbByteBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);
                totalTime += (System.currentTimeMillis() - startTime);
                Log.d("onPreviewDataUpdate", "耗时=" + (totalTime / index));

                if (null == bitmap) {
                    if (displayOrientation == 270 || displayOrientation == 90) {
                        bitmap = Bitmap.createBitmap(height, width, Bitmap.Config.ARGB_8888);
                    } else {
                        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                    }
                }
                bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(argbByteBuffer.array()));
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
