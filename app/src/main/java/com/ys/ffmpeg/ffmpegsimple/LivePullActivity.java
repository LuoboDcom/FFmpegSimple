package com.ys.ffmpeg.ffmpegsimple;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.ys.ffmpeg.ffmpegsimple.util.FFmpegUtil;

public class LivePullActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live_pull);

        SurfaceView liveView = (SurfaceView) findViewById(R.id.live_view);
        SurfaceHolder surfaceHolder = liveView.getHolder();

        final String liveUrl = "http://zv.3gv.ifeng.com/live/zhongwen800k.m3u8";
        surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(final SurfaceHolder holder) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        FFmpegUtil.playVideo(liveUrl,holder.getSurface());
                    }
                }).start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }
}
