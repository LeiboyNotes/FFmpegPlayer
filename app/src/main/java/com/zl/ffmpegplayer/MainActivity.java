package com.zl.ffmpegplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "MainActivity";

    //读写权限
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE};
    //请求状态码
    private static int REQUEST_PERMISSION_CODE = 1;
    private SurfaceView surfaceView;
    private ZLPlayer player;
    private SeekBar seekBar;
    private boolean isTouch;
    private boolean isSeek;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = findViewById(R.id.surfaceView);
        seekBar = findViewById(R.id.seekbar);

        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP) {

            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED) {
                ActivityCompat.requestPermissions(this, PERMISSIONS_STORAGE, REQUEST_PERMISSION_CODE);
            } else {
                player = new ZLPlayer();
                player.setSurfaceView(surfaceView);
                player.setDataSource(new File(
                        Environment.getExternalStorageDirectory() + File.separator + "/demo.mp4").getAbsolutePath());
                player.setPreparedListener(new ZLPlayer.OnPreparedListener() {

                    @Override
                    public void onPrepared() {
                        int duration = player.getDuration();
                        //如果是直播duration为0    不为0显示seekbar
                        if (duration != 0) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    seekBar.setVisibility(View.VISIBLE);
                                }
                            });

                        }
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(MainActivity.this, "开始播放！", Toast.LENGTH_SHORT).show();
                            }
                        });
                        player.start();
                        //播放   调用到native中
                        //start  play
                    }
                });
                player.setOnErrorListener(new ZLPlayer.OnErrorListener() {
                    @Override
                    public void onError(final int errorCode) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(MainActivity.this, "出错了 ，错误码：" + errorCode, Toast.LENGTH_SHORT).show();

                            }
                        });
                    }
                });
                player.setOnProgressListener(new ZLPlayer.OnProgressListener() {
                    @Override
                    public void onProgress(final int progress) {
                        //progress当前播放进度
                        Log.e(TAG, "progress:" + progress);

                        if(!isTouch){

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                int duration = player.getDuration();
                                if (duration != 0) {
                                    if (isSeek) {
                                        isSeek = false;
                                        return;
                                    }
                                    Log.e(TAG, "duration:" + duration);
                                    seekBar.setProgress(progress * 100 / duration);
                                }
                            }
                        });
                        }
                    }
                });
                seekBar.setOnSeekBarChangeListener(this);
            }
        }


    }


    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION_CODE) {
            for (int i = 0; i < permissions.length; i++) {
                Log.i("MainActivity", "申请的权限为：" + permissions[i] + ",申请结果：" + grantResults[i]);
            }
        }
    }


    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

        isSeek = true;
        isTouch = false;
        //获取seekbard的当前进度（百分比）
        int seekBarProgress = seekBar.getProgress();
        //将seekbar的进度转换成真实的播放进度
        int duration = player.getDuration();
        int playProgress = seekBarProgress*duration/100;
        //将播放进度传递给底层ffmpeg
        player.seekTo(playProgress);

    }
}
