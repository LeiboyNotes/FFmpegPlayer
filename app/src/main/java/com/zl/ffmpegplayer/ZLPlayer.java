package com.zl.ffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class ZLPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }


    //播放地址
    private String dataSource;
    private SurfaceHolder surfaceHolder;

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放准备工作
     */
    public void prepare() {
        prepareNative(dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        startNative();
    }

    /**
     * 供native反射用
     * 表示播放器已经准备好了
     */
    public void onPrepared() {
        if (preparedListener != null) {

            preparedListener.onPrepared();
        }
    }

    /**
     * 供native反射用
     * 表示播放错误
     */
    public void onError(int errorCode) {
        if (onErrorListener != null) {
            onErrorListener.onError(errorCode);
        }
    }


    OnPreparedListener preparedListener;
    OnErrorListener onErrorListener;

    void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    void setPreparedListener(OnPreparedListener listener) {
        this.preparedListener = listener;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (surfaceHolder != null) {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }


    /**
     * 画布创建回调
     *
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布刷新
     *
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }


    interface OnPreparedListener {
        void onPrepared();
    }

    interface OnErrorListener {
        void onError(int errorCode);
    }

    private native void prepareNative(String dataSource);
    private native void setSurfaceNative(Surface surface);


    private native void startNative();

}
