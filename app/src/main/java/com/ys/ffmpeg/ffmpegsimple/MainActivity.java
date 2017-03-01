package com.ys.ffmpeg.ffmpegsimple;

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
        tv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String text = fmpegUtil.decode("","");
                tv.setText(text);
            }
        });


    }
}
