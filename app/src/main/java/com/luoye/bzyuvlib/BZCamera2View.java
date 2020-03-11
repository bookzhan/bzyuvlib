package com.luoye.bzyuvlib;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Range;
import android.util.Size;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.TextureView;
import android.view.WindowManager;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.core.content.ContextCompat;


import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * Created by zhandalin on 2019-10-25 11:40.
 * description:
 */
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class BZCamera2View extends TextureView implements TextureView.SurfaceTextureListener, ImageReader.OnImageAvailableListener {
    private static final String TAG = "bz_BZCamera2View";
    private final float dp_1;
    private SurfaceTexture mSurfaceTexture;
    private int previewTargetSizeWidth = 640;
    private int previewTargetSizeHeight = 480;
    private boolean needCallBackData = false;
    private HandlerThread mCameraHandlerThread = null;
    private Camera2Handler mCameraHandler = null;
    private String mCurrentCameraID = "1";
    private CaptureRequest.Builder captureRequestBuilder = null;
    private CaptureRequest mPreviewRequest = null;
    private CameraDevice mCameraDevice = null;
    private long previewStartTime = 0;
    private long frameCount = 0;
    private CameraCaptureSession mCameraCaptureSession = null;
    private ImageReader mImageReader = null;
    private HandlerThread mYUVHandlerThread = null;
    private Handler mYUVHandler = null;
    private Range<Integer>[] fpsRanges;
    private OnPreviewBitmapListener onPreviewBitmapListener;
    private Bitmap bitmap = null;
    private long index = 0;
    private long spaceTime = 0;
    private BZYUVUtil bzyuvUtil;

    public BZCamera2View(Context context) {
        this(context, null);
    }

    public BZCamera2View(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BZCamera2View(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setSurfaceTextureListener(this);
        dp_1 = getResources().getDisplayMetrics().density;
        bzyuvUtil = new BZYUVUtil();
    }

    public void onResume() {
        startPreview();
    }

    public void startPreview() {
        if (null != mSurfaceTexture && isAvailable()) {
            startPreview(mSurfaceTexture);
        }
    }

    public void onPause() {
        stopPreview();
    }

    public synchronized void setNeedCallBackData(boolean needCallBackData) {
        this.needCallBackData = needCallBackData;
    }

    public void setPreviewTargetSize(int previewTargetSizeWidth, int previewTargetSizeHeight) {
        if (previewTargetSizeWidth <= 0 || previewTargetSizeHeight <= 0) {
            return;
        }
        this.previewTargetSizeWidth = previewTargetSizeWidth;
        this.previewTargetSizeHeight = previewTargetSizeHeight;
    }

    private synchronized void startPreview(final SurfaceTexture surfaceTexture) {
        if (null == surfaceTexture) {
            return;
        }
        //Granted Permission
        if (ContextCompat.checkSelfPermission(getContext(), Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        int width = getWidth();
        int height = getHeight();
        if (width <= 0 || height <= 0) {
            return;
        }
        stopPreview();
        mCameraHandlerThread = new HandlerThread("Camera2HandlerThread", Thread.MAX_PRIORITY);
        mCameraHandlerThread.start();
        mCameraHandler = new Camera2Handler(mCameraHandlerThread.getLooper());

        mYUVHandlerThread = new HandlerThread("YUVHandlerThread", Thread.MAX_PRIORITY);
        mYUVHandlerThread.start();
        mYUVHandler = new Handler(mYUVHandlerThread.getLooper());

        frameCount = 0;
        previewStartTime = 0;
        try {
            CameraManager manager = (CameraManager) getContext().getSystemService(Context.CAMERA_SERVICE);
            String fitCameraID = getFitCameraID(manager.getCameraIdList(), mCurrentCameraID);
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(fitCameraID);
            isHardwareSupported(characteristics);
            StreamConfigurationMap streamConfigurationMap = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            if (null != streamConfigurationMap) {
                Size[] sizes = streamConfigurationMap.getOutputSizes(ImageFormat.YUV_420_888);
                if (null != sizes && sizes.length > 0) {
                    for (Size size : sizes) {
                        Log.d(TAG, "previewSize width=" + size.getWidth() + " height=" + size.getHeight());
                    }
                }
            }
            fpsRanges = characteristics.get(CameraCharacteristics.CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES);
            Log.d(TAG, "SYNC_MAX_LATENCY_PER_FRAME_CONTROL: " + Arrays.toString(fpsRanges));

            manager.openCamera(fitCameraID, new CameraDevice.StateCallback() {
                @Override
                public void onOpened(@NonNull CameraDevice camera) {
                    try {
                        mCameraDevice = camera;
                        captureRequestBuilder = camera.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                        if (previewTargetSizeWidth >= 0 && previewTargetSizeHeight >= 0) {
                            mImageReader = ImageReader.newInstance(previewTargetSizeWidth, previewTargetSizeHeight, ImageFormat.YUV_420_888, 2);
                            surfaceTexture.setDefaultBufferSize(previewTargetSizeWidth, previewTargetSizeHeight);
                        } else {
                            mImageReader = ImageReader.newInstance(getWidth(), getHeight(), ImageFormat.YUV_420_888, 2);
                            surfaceTexture.setDefaultBufferSize(getWidth(), getHeight());
                        }
                        Surface surface = new Surface(surfaceTexture);
                        captureRequestBuilder.addTarget(surface);
                        captureRequestBuilder.addTarget(mImageReader.getSurface());
                        mImageReader.setOnImageAvailableListener(BZCamera2View.this, mYUVHandler);
                        ArrayList<Surface> surfaceArrayList = new ArrayList<>();
                        surfaceArrayList.add(surface);
                        surfaceArrayList.add(mImageReader.getSurface());
                        camera.createCaptureSession(surfaceArrayList, stateCallback, mCameraHandler);
                    } catch (Exception e) {
//                        Log.e(TAG, e);
                    }
                }

                @Override
                public void onDisconnected(@NonNull CameraDevice camera) {

                }

                @Override
                public void onError(@NonNull CameraDevice camera, int error) {

                }
            }, mCameraHandler);

        } catch (Exception e) {
//            Log.e(TAG, e);
        }
    }

    private int isHardwareSupported(CameraCharacteristics characteristics) {
        Integer deviceLevel = characteristics.get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL);
        if (deviceLevel == null) {
            Log.e(TAG, "can not get INFO_SUPPORTED_HARDWARE_LEVEL");
            return -1;
        }
        switch (deviceLevel) {
            case CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_FULL:
                Log.d(TAG, "hardware supported level:LEVEL_FULL");
                break;
            case CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY:
                Log.w(TAG, "hardware supported level:LEVEL_LEGACY");
                break;
            case CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_3:
                Log.w(TAG, "hardware supported level:LEVEL_3");
                break;
            case CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_LIMITED:
                Log.w(TAG, "hardware supported level:LEVEL_LIMITED");
                break;
            case CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_EXTERNAL:
                Log.d(TAG, "hardware supported level:LEVEL_EXTERNA");
                break;
        }
        return deviceLevel;
    }

    private CameraCaptureSession.StateCallback stateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            startRepeatingRequest(session);
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {

        }
    };

    private void startRepeatingRequest(CameraCaptureSession cameraCaptureSession) {
        if (null == cameraCaptureSession || null == captureRequestBuilder) {
            return;
        }
        mCameraCaptureSession = cameraCaptureSession;
        Log.d(TAG, "startRepeatingRequest");
        try {
            captureRequestBuilder.set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, new Range<>(30, 30));
            captureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            mPreviewRequest = captureRequestBuilder.build();
            cameraCaptureSession.setRepeatingRequest(mPreviewRequest, new CameraCaptureSession.CaptureCallback() {
                @Override
                public void onCaptureStarted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, long timestamp, long frameNumber) {
                    super.onCaptureStarted(session, request, timestamp, frameNumber);
                }
            }, mCameraHandler);
        } catch (Exception e) {
//            Log.e(TAG, e);
        }
    }

    public synchronized void stopPreview() {
        Log.d(TAG, "stopPreview");
        if (null != mCameraHandler) {
            mCameraHandler.post(new Runnable() {
                @Override
                public void run() {
                    try {
                        if (null != mCameraCaptureSession) {
                            mCameraCaptureSession.abortCaptures();
                            mCameraCaptureSession.close();
                            mCameraCaptureSession = null;
                        }
                        if (null != mCameraDevice) {
                            mCameraDevice.close();
                            mCameraDevice = null;
                        }
                    } catch (Throwable e) {
//                        Log.e(TAG, e);
                    }
                }
            });
            mCameraHandler = null;
        }
        if (null != mCameraHandlerThread) {
            try {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                    mCameraHandlerThread.quitSafely();
                } else {
                    mCameraHandlerThread.quit();
                }
                long startTime = System.currentTimeMillis();
                mCameraHandlerThread.join();
                Log.d(TAG, "mCameraHandlerThread.join() time consuming=" + (System.currentTimeMillis() - startTime));
            } catch (Exception e) {
//                Log.e(TAG, e);
            }
            mCameraHandlerThread = null;
        }
        if (null != mYUVHandlerThread) {
            try {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                    mYUVHandlerThread.quitSafely();
                } else {
                    mYUVHandlerThread.quit();
                }
                long startTime = System.currentTimeMillis();
                mYUVHandlerThread.join();
                Log.d(TAG, "mYUVHandlerThread.join() time consuming=" + (System.currentTimeMillis() - startTime));
            } catch (Exception e) {
//                Log.e(TAG, e);
            }
            mYUVHandlerThread = null;
        }
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mSurfaceTexture = surface;
        Log.d(TAG, "onSurfaceTextureAvailable");
        startPreview(surface);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        Log.d(TAG, "onSurfaceTextureSizeChanged width=" + width + " height=" + height);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

    }

//    private CameraPreviewListener cameraPreviewListener = new CameraPreviewListener() {
//        @Override
//        public void onPreviewSuccess(Camera camera, final int width, final int height) {
//            Log.d(TAG, "onPreviewSuccess width=" + width + " height=" + height);
//            post(new Runnable() {
//                @Override
//                public void run() {
//                    Matrix matrix = new Matrix();
//                    float finalWidth = getWidth();
//                    float finalHeight = finalWidth * height / width;
//                    if (finalHeight < getHeight()) {
//                        finalHeight = getHeight();
//                        finalWidth = finalHeight * width / height;
//                    }
//                    matrix.postScale(finalWidth / getWidth(), finalHeight / getHeight(), getWidth() / 2, getHeight() / 2);
//                    setTransform(matrix);
//                    if (null != onSetTransformListener) {
//                        onSetTransformListener.setTransform(matrix);
//                    }
//                }
//            });
//            if (null != mFlashMode) {
//                setFlashMode(mFlashMode);
//            }
//            setExposureCompensation(mExposureProgress);
//            if (null != mWhiteBalance) {
//                setWhiteBalance(mWhiteBalance);
//            }
//        }
//    };
//
//    public void setCameraStateListener(CameraStateListener cameraStateListener) {
//        this.mCameraStateListener = cameraStateListener;
//        if (null != mCameraHandler) {
//            mCameraHandler.setCameraStateListener(mCameraStateListener);
//        }
//    }

    public static int getDisplayOrientation(Context context) {
        WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        int rotation = windowManager.getDefaultDisplay().getRotation();
        short degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
        }

        return degrees;
    }

    public static String getFitCameraID(String[] cameraIdArray, String targetCameraID) {
        if (null == cameraIdArray || cameraIdArray.length <= 0 || null == targetCameraID) {
            return "1";
        }
        String finalCameraId = cameraIdArray[0];
        for (String cameraId : cameraIdArray) {
            if (targetCameraID.equals(cameraId)) {
                finalCameraId = cameraId;
                break;
            }
        }
        return finalCameraId;
    }

    public synchronized void setFlashMode(String flashMode) {
        if (null == mCameraHandler || null == flashMode) {
            return;
        }
//        mFlashMode = flashMode;
        Message message = new Message();
        message.obj = flashMode;
        message.what = MessageConstant.MSG_SET_FLASH_MODE;
        mCameraHandler.sendMessage(message);
    }

    public synchronized void setExposureCompensation(float progress) {
        if (null == mCameraHandler) {
            return;
        }
//        mExposureProgress = progress;
        Message message = new Message();
        message.obj = progress;
        message.what = MessageConstant.MSG_SET_EXPOSURE_COMPENSATION;
        mCameraHandler.sendMessage(message);
    }

    public synchronized void setWhiteBalance(String whiteBalance) {
        if (null == mCameraHandler || null == whiteBalance) {
            return;
        }
//        mWhiteBalance = whiteBalance;
        Message message = new Message();
        message.obj = whiteBalance;
        message.what = MessageConstant.MSG_SET_WHITE_BALANCE;
        mCameraHandler.sendMessage(message);
    }

    public synchronized void switchCamera() {
//        if (mCurrentCameraID == Camera.CameraInfo.CAMERA_FACING_FRONT) {
//            mCurrentCameraID = Camera.CameraInfo.CAMERA_FACING_BACK;
//        } else if (mCurrentCameraID == Camera.CameraInfo.CAMERA_FACING_BACK) {
//            mCurrentCameraID = Camera.CameraInfo.CAMERA_FACING_FRONT;
//        }
//        switchCamera2ID(mCurrentCameraID);
    }

    public synchronized void switchCamera2ID(int cameraId) {
//        mCurrentCameraID = cameraId;
        startPreview();
    }


    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
//        if (event.getAction() == MotionEvent.ACTION_DOWN) {
//            setFocusPoint(event.getX(), event.getY());
//        }
        return super.dispatchTouchEvent(event);
    }

//    private void setFocusPoint(float x, float y) {
//        if (null == mCameraHandler) {
//            return;
//        }
//        RectF rectSrc = new RectF(0, 0, getWidth(), getHeight());
//        RectF rectDst = new RectF();
//        Matrix matrix = new Matrix();
//        getTransform(matrix);
//        matrix.mapRect(rectDst, rectSrc);
//        if (!rectDst.contains(x, y)) {//Touch not camera area
//            return;
//        }
//        x -= rectDst.left;
//        y -= rectDst.top;
//        Log.d(TAG, "setFocusPoint dis x=" + x + " y=" + y);
//
//        FocusObj focusObj = new FocusObj();
//        focusObj.setFocusPointF(new PointF(x, y));
//        focusObj.setFocusRadius(dp_1 * MessageConstant.FOCUS_RADIUS_DP);
//        focusObj.setContentWidth(rectDst.width());
//        focusObj.setContentHeight(rectDst.height());
//
//        Message message = new Message();
//        message.what = MessageConstant.MSG_SET_FOCUS_POINT;
//        message.obj = focusObj;
//        mCameraHandler.sendMessage(message);
//    }

//    public void setOnSetTransformListener(OnSetTransformListener onSetTransformListener) {
////        this.onSetTransformListener = onSetTransformListener;
//    }

    @Override
    public void onImageAvailable(ImageReader reader) {
        Image image = reader.acquireLatestImage();
        if (image == null) {
            return;
        }
        int width = image.getWidth();
        int height = image.getHeight();

        if (null == bitmap) {
//            bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            bitmap = Bitmap.createBitmap(height, width, Bitmap.Config.ARGB_8888);
        }

        if (null != onPreviewBitmapListener) {
            long startTime = System.currentTimeMillis();
            byte[] bytes = bzyuvUtil.yuv420pToBGRA(image, true, 90);
            spaceTime += (System.currentTimeMillis() - startTime);
            bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(bytes));
            index++;
            Log.d(TAG, "平均 yuv转换 耗时=" + (spaceTime / index) + " bitmap.width=" + width + " height=" + height);
            onPreviewBitmapListener.onPreviewBitmapListener(bitmap);
        }
        image.close();

        if (previewStartTime <= 0) {
            previewStartTime = System.currentTimeMillis();
        }
        frameCount++;
        long time = System.currentTimeMillis() - previewStartTime;
        float fps = frameCount / (time / 1000.f);
        if (frameCount % 30 == 0) {
            Log.d(TAG, "onPreviewDataUpdate width=" + width + " height=" + height + " fps=" + fps);
        }
    }

    public void setOnPreviewBitmapListener(OnPreviewBitmapListener onPreviewBitmapListener) {
        this.onPreviewBitmapListener = onPreviewBitmapListener;
    }

    public interface OnPreviewBitmapListener {
        void onPreviewBitmapListener(Bitmap bitmap);
    }
}
