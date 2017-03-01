package com.ys.ffmpeg.ffmpegsimple.util;

/**
 *   FFmpeg util tool
 * Created by Yoosir 2016/10/10 0010.
 */
public class FFmpegUtil {

    public native String decode(String inputUrl,String outputUrl);

    static{
        System.loadLibrary("avcodec");
        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("postproc");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("ysffmpeg.so");
    }
}
