package com.luoye.bzyuvlib;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzcamera.listener.OnCameraStateListener;

import java.nio.ByteBuffer;

public class Camera1YUVCropActivity extends AppCompatActivity {
    private static final String TAG = "bz_Camera1YUVCrop";

    private BZCameraView bz_camera_view;
    private ImageView bz_image_view;
    private byte[] yuvBuffer = null;
    private byte[] argbByteBuffer = null;
    private byte[] cropYuvBuffer = null;
    private int index = 0;
    private Bitmap bitmap = null;
    private long totalTime = 0;
    private int cropStartX = 100;
    private int cropStartY = 100;
    private int cropWidth = 240;
    private int cropHeight = 320;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera1_y_u_v_crop);
        bz_camera_view = findViewById(R.id.bz_camera_view);
        bz_camera_view.setPreviewTargetSize(480, 640);
        bz_image_view = findViewById(R.id.bz_image_view);


//        bz_image_view.setScaleX(-1);
//        bz_image_view.setScaleY(-1);
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
                if (width < cropWidth + cropStartX || height < cropHeight + cropStartY) {
                    Log.e(TAG, "width < cropWidth + cropStartX || height < cropHeight + cropStartY");
                    return;
                }
                long startTime = System.currentTimeMillis();
                if (null == yuvBuffer) {
                    yuvBuffer = new byte[width * height * 3 / 2];
                }
                if (null == argbByteBuffer) {
                    argbByteBuffer = new byte[cropWidth * cropHeight * 4];
                }
                if (null == cropYuvBuffer) {
                    cropYuvBuffer = new byte[cropWidth * cropHeight * 3 / 2];
                }
                //Correct angle and mirror image to facilitate crop coordinate calculation
                BZYUVUtil.preHandleNV21(data, yuvBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);
                if (displayOrientation == 270 || displayOrientation == 90) {
                    BZYUVUtil.cropYUV420(yuvBuffer, cropYuvBuffer, height, width, cropStartX, cropStartY, cropWidth, cropHeight);
                } else {
                    BZYUVUtil.cropYUV420(yuvBuffer, cropYuvBuffer, width, height, cropStartX, cropStartY, cropWidth, cropHeight);
                }
                BZYUVUtil.yuv420ToRGBA(cropYuvBuffer, argbByteBuffer, cropWidth, cropHeight, false, 0);
                index++;
                totalTime += (System.currentTimeMillis() - startTime);
                Log.d(TAG, "time cost=" + (totalTime / index));

                if (null == bitmap) {
                    bitmap = Bitmap.createBitmap(cropWidth, cropHeight, Bitmap.Config.ARGB_8888);
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
