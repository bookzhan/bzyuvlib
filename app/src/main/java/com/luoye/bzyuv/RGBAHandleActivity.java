package com.luoye.bzyuv;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.luoye.bzyuvlib.BZYUVUtil;

import java.nio.ByteBuffer;

public class RGBAHandleActivity extends AppCompatActivity {
    private final static String TAG = "bz_RGBAHandle";
    private ImageView iv_rgba_dis;
    private Bitmap bitmapSrc;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_r_g_b_a_handle);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        ImageView iv_rgba_src = findViewById(R.id.iv_rgba_src);
        iv_rgba_dis = findViewById(R.id.iv_rgba_dis);

        bitmapSrc = BitmapFactory.decodeResource(getResources(), R.drawable.timg_2);
        iv_rgba_src.setImageBitmap(bitmapSrc);
    }

    public void testRgba(View view) {
        Log.d(TAG,"Width="+bitmapSrc.getWidth()+" Height="+bitmapSrc.getHeight());
        byte[] srcBuffer = new byte[bitmapSrc.getWidth() * bitmapSrc.getHeight() * 4];
        bitmapSrc.copyPixelsToBuffer(ByteBuffer.wrap(srcBuffer));

        byte[] outBuffer = new byte[bitmapSrc.getWidth() * bitmapSrc.getHeight() * 4];

        int rotate = 90;
        long startTime = System.currentTimeMillis();
        BZYUVUtil.handleRGBA(srcBuffer, bitmapSrc.getWidth() * 4, outBuffer, bitmapSrc.getWidth(), bitmapSrc.getHeight(), true, rotate);
        Log.d(TAG, "time cost=" + (System.currentTimeMillis() - startTime));
        Bitmap bitmapDis;
        if (rotate == 90 || rotate == 270) {
            bitmapDis = Bitmap.createBitmap(bitmapSrc.getHeight(), bitmapSrc.getWidth(), Bitmap.Config.ARGB_8888);
        } else {
            bitmapDis = Bitmap.createBitmap(bitmapSrc.getWidth(), bitmapSrc.getHeight(), Bitmap.Config.ARGB_8888);
        }
        bitmapDis.copyPixelsFromBuffer(ByteBuffer.wrap(outBuffer));
        iv_rgba_dis.setImageBitmap(bitmapDis);
    }
}
