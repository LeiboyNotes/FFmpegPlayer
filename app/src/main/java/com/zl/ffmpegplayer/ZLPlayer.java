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
    public void onProgress(int progress) {
        if (onProgressListener != null) {
            onProgressListener.onProgress(progress);
        }
    }




    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setPreparedListener(OnPreparedListener onPreparedListener) {
        this.preparedListener = onPreparedListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
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

    /**
     * 资源释放
     */
    public void release() {
        surfaceHolder.removeCallback(this);
        releaseNative();
    }

    /**
     * 停止播放
     */
    public void stop() {
        stopNative();
    }

    /**
     * 获取总播放时长
     * @return
     */
    public int getDuration(){
        return getDurationNative();
    }

    /**
     * 播放进度条调整
     * @param playProgress
     */
    public void seekTo(final int playProgress) {

        new Thread(){
            @Override
            public void run() {
                seekToNative(playProgress);
            }
        }.start();

    }

    interface OnPreparedListener {
        void onPrepared();
    }

    interface OnErrorListener {
        void onError(int errorCode);
    }
    interface OnProgressListener {
        void onProgress(int progress);
    }

    private OnPreparedListener preparedListener;
    private OnErrorListener onErrorListener;
    private OnProgressListener onProgressListener;


    private native void prepareNative(String dataSource);
    private native void setSurfaceNative(Surface surface);

    private native void startNative();
    private native void stopNative();
    private native void releaseNative();
    private native int getDurationNative();
    private native void seekToNative(int playProgress);



}
