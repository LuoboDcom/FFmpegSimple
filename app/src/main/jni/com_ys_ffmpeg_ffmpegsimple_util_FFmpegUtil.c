//
// Created by Administrator on 2017/3/1 0001.
//
#include<com_ys_ffmpeg_ffmpegsimple_util_FFmpegUtil.h>
//#include "libavcodec/avcodec.h"
#include <string.h>

JNIEXPORT jstring JNICALL Java_com_ys_ffmpeg_ffmpegsimple_util_FFmpegUtil_decode
  (JNIEnv *env, jobject jcls, jstring jstr1, jstring jstr2){

        char info[10000] = { 0 };
            //sprintf(info, "%s\n", avcodec_configuration());
            sprintf(info, "%s\n", "hello world");
            return (*env)->NewStringUTF(env, info);
  }
