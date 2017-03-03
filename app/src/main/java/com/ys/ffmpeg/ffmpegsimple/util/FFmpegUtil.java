package com.ys.ffmpeg.ffmpegsimple.util;

/**
 *   FFmpeg util tool
 * Created by Yoosir 2016/10/10 0010.
 */
public class FFmpegUtil {

    public native int decode(String inputUrl,String outputUrl);

    public static native int playVideo(String url,Object surface);

    static{
        System.loadLibrary("avcodec-57");
//        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
//        System.loadLibrary("postproc");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("ysffmpeg");
    }
}
