package com.luoye.bzyuv;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import android.graphics.Bitmap;
import android.hardware.camera2.CameraDevice;
import android.media.Image;
import android.os.Bundle;
import android.widget.ImageView;

import com.luoye.bzcamera.BZCamera2View;
import com.luoye.bzyuvlib.BZYUVUtil;

import java.nio.ByteBuffer;

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
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_grey_image_test);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
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