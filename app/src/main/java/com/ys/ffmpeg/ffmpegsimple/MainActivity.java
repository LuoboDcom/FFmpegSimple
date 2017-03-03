package com.ys.ffmpeg.ffmpegsimple;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.ys.ffmpeg.ffmpegsimple.util.FFmpegUtil;

public class MainActivity extends AppCompatActivity {

    TextView tv;
    FFmpegUtil fmpegUtil;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tv = (TextView) findViewById(R.id.text);
        fmpegUtil = new FFmpegUtil();

        final String inPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/VID_20160501_140013.mp4";
        final String outPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/VID.yuv";

        tv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int text = fmpegUtil.decode(inPath,outPath);
                tv.setText("result="+text);
            }
        });
    }
}
