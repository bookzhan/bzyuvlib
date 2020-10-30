package com.luoye.bzyuvlib;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.hardware.camera2.CameraDevice;
import android.media.Image;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import com.luoye.bzcamera.BZCamera2View;

import java.nio.ByteBuffer;

@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class GreyImageTestActivity extends AppCompatActivity {

    private BZCamera2View bz_camera2_view;
    private ImageView image_view;
    private BZYUVUtil bzyuvUtil;
    private byte[] outDataRGBA = null;
    private Bitmap bitmap = null;
    private byte[] greyBuffer = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_grey_image_test);
        bzyuvUtil = new BZYUVUtil();
        image_view = findViewById(R.id.image_view);
        bz_camera2_view = findViewById(R.id.bz_camera2_view);
        bz_camera2_view.setPreviewTargetSize(480, 640);
        bz_camera2_view.setDisplayOrientation(90);
        bz_camera2_view.switchCamera();
        bz_camera2_view.setOnStatusChangeListener(new BZCamera2View.OnStatusChangeListener() {
            @Override
            public void onPreviewSuccess(CameraDevice mCameraDevice, int width, int height) {

            }

            @Override
            public void onImageAvailable(Image image, int displayOrientation, float fps) {
                if (null == outDataRGBA) {
                    outDataRGBA = new byte[image.getWidth() * image.getHeight() * 4];
                }
                if (null == greyBuffer) {
                    greyBuffer = new byte[image.getWidth() * image.getHeight()];
                }
                int width = image.getWidth();
                int height = image.getHeight();
                if (displayOrientation == 90 || displayOrientation == 270) {
                    width = image.getHeight();
                    height = image.getWidth();
                }
                byte[] toGrey = bzyuvUtil.yuv420pToGrey(image, true, displayOrientation);
                BZYUVUtil.translationSingleChannel(toGrey, greyBuffer, width, height, -60,60);

                BZYUVUtil.greyToRGBA(greyBuffer, outDataRGBA, width, height);
                if (null == bitmap) {
                    bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                }
                bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(outDataRGBA));
                image_view.post(new Runnable() {
                    @Override
                    public void run() {
                        image_view.setImageBitmap(bitmap);
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