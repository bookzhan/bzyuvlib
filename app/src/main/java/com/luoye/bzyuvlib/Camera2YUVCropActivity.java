package com.luoye.bzyuvlib;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.media.Image;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzcamera.BZCamera2View;

import java.io.FileOutputStream;
import java.nio.ByteBuffer;

@TargetApi(android.os.Build.VERSION_CODES.LOLLIPOP)
public class Camera2YUVCropActivity extends AppCompatActivity {
    private static final String TAG = "bz_Camera2YUVCrop";
    private BZCamera2View bz_camera2_view;
    private ImageView iv_preview;
    private BZYUVUtil bzyuvUtil;
    private Bitmap bitmap;
    private long spaceTime;
    private int index;
    private int cropStartX = 100;
    private int cropStartY = 100;
    private int cropWidth = 240;
    private int cropHeight = 320;
    private byte[] argbByteBuffer = null;
    private byte[] cropYuvBuffer = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera2_y_u_v_crop);
        iv_preview = findViewById(R.id.iv_preview);
        bz_camera2_view = findViewById(R.id.bz_camera2_view);
        bz_camera2_view.setCheckCameraCapacity(false);
        bzyuvUtil = new BZYUVUtil();
        bz_camera2_view.setOnStatusChangeListener(new BZCamera2View.OnStatusChangeListener() {

            private FileOutputStream fileOutputStream;

            @Override
            public void onPreviewSuccess(CameraDevice mCameraDevice, int width, int height) {

            }

            @Override
            public void onImageAvailable(Image image, int displayOrientation, float fps) {
                int height = image.getHeight();
                int width = image.getWidth();
                if (displayOrientation == 270 || displayOrientation == 90) {
                    width = image.getHeight();
                    height = image.getWidth();
                }
                if (width < cropWidth + cropStartX || height < cropHeight + cropStartY) {
                    Log.e(TAG, "width < cropWidth + cropStartX || height < cropHeight + cropStartY");
                    return;
                }
                if (null == argbByteBuffer) {
                    argbByteBuffer = new byte[cropWidth * cropHeight * 4];
                }
                if (null == cropYuvBuffer) {
                    cropYuvBuffer = new byte[cropWidth * cropHeight * 3 / 2];
                }
                long startTime = System.currentTimeMillis();
                byte[] yuv420 = bzyuvUtil.preHandleYUV420(image, bz_camera2_view.getCurrentCameraLensFacing() == CameraCharacteristics.LENS_FACING_FRONT, displayOrientation);
                if (null == fileOutputStream) {
                    try {
                        fileOutputStream = new FileOutputStream("/sdcard/bzmedia/kk.yuv");
                        fileOutputStream.write(yuv420, 0, yuv420.length);
                        fileOutputStream.flush();
                        fileOutputStream.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                BZYUVUtil.cropYUV420(yuv420, cropYuvBuffer, width, height, cropStartX, cropStartY, cropWidth, cropHeight);

                BZYUVUtil.yuv420ToRGBA(cropYuvBuffer, argbByteBuffer, cropWidth, cropHeight, false, 0);

                spaceTime += (System.currentTimeMillis() - startTime);
                index++;
                Log.d(TAG, "time cost=" + (spaceTime / index) + " bitmap.width=" + width + " height=" + height);
                if (null == bitmap) {
                    bitmap = Bitmap.createBitmap(cropWidth, cropHeight, Bitmap.Config.ARGB_8888);
                }
                bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(argbByteBuffer));
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
