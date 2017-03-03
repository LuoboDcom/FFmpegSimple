//
// Created by Administrator on 2017/3/1 0001.
//
#include<com_ys_ffmpeg_ffmpegsimple_util_FFmpegUtil.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string.h>
#include <android/log.h>

//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char* fmt, va_list vl){
    FILE *fp=fopen("/storage/emulated/0/av_log.txt","a+");
    if(fp){
        vfprintf(fp,fmt,vl);
        fflush(fp);
        fclose(fp);
    }
}

JNIEXPORT jint JNICALL Java_com_ys_ffmpeg_ffmpegsimple_util_FFmpegUtil_decode
  (JNIEnv *env, jobject jcls, jstring input_jstr, jstring output_jstr){

        AVFormatContext *pFormatCtx;
        int  i,videoIndex;
        AVCodecContext  *pCodecCtx;
        AVCodec         *pCodec;
        AVFrame *pFrame,*pFrameYUV;
        uint8_t *out_buffer;
        AVPacket *packet;
        int y_size;
        int ret,got_picture;
        struct SwsContext *img_convert_ctx;
        FILE *fp_yuv;
        int frame_cnt;
        clock_t time_start,time_finish;
        double time_duration = 0.0;

        char input_str[500] = {0};
        char output_str[500] = {0};

        char info[1000] = {0};
        sprintf(input_str,"%s",(*env)->GetStringUTFChars(env,input_jstr,NULL));
        sprintf(output_str,"%s",(*env)->GetStringUTFChars(env,output_jstr,NULL));

        //FFmpeg av_log() callback
        av_log_set_callback(custom_log);

        av_register_all();
        avformat_network_init();
        pFormatCtx = avformat_alloc_context();

        if(avformat_open_input(&pFormatCtx,input_str,NULL,NULL)!= 0){
            //Couldn't open input stream .\n
            return -1;
        }

        if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
            //Couldn't find stream information.\n
            return -1;
        }

        videoIndex = -1;
        for(i=0 ; i < pFormatCtx->nb_streams;i++){
            if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
                videoIndex = i;
                break;
            }
        }

        if(videoIndex == -1){
            //Couldn't find a video stream.\n
            return -1;
        }
        pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
        pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec == NULL){
            // Couldn't find Codec.\n
            return -1;
        }
        if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0){
            //Couldn't open codec.\n
            return -1;
        }

        pFrame=av_frame_alloc();
        pFrameYUV = av_frame_alloc();
        out_buffer = (unsigned char*)av_malloc(
                av_image_get_buffer_size(AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,1));
        av_image_fill_arrays(pFrameYUV->data,
                             pFrameYUV->linesize,
                             out_buffer,
                             AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,
                             1);

        packet = (AVPacket*) av_malloc(sizeof(AVPacket));

        img_convert_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,
                    pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height,
                    AV_PIX_FMT_YUV420P,SWS_BICUBIC,NULL,NULL,NULL);
        sprintf(info,  "[Input        ]%s\n",input_str);
        sprintf(info,"%s[Output       ]%s\n",info,output_str);
        sprintf(info,"%s[Format       ]%s\n",info,pFormatCtx->iformat->name);
        sprintf(info,"%s[Codec        ]%s\n",info,pCodecCtx->codec->name);
        sprintf(info,"%s[Resolution   ]%dx%d\n",info,pCodecCtx->width,pCodecCtx->height);

        fp_yuv = fopen(output_str,"wb+");
        if(fp_yuv == NULL){
            printf("Couldn't open output file.\n");
            return -1;
        }

        frame_cnt = 0;
        time_start = clock();

        while(av_read_frame(pFormatCtx,packet) >= 0){
            if(packet->stream_index == videoIndex){
                ret = avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
                if(ret < 0){
                    //Decode Error.\n
                    return -1;
                }
                if(got_picture){
                    sws_scale(img_convert_ctx,(const uint8_t* const*)pFrame->data,
                              pFrame->linesize,0,pCodecCtx->height,
                              pFrameYUV->data,pFrameYUV->linesize);
                    y_size = pCodecCtx->width*pCodecCtx->height;
                    fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);     //Y
                    fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);   //U
                    fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);   //V

                    //Output info
                    char pictype_str[10] = {0};
                    switch(pFrame->pict_type){
                        case AV_PICTURE_TYPE_I:
                            sprintf(pictype_str,"I");
                            break;
                        case AV_PICTURE_TYPE_P:
                            sprintf(pictype_str,"P");
                            break;
                        case AV_PICTURE_TYPE_B:
                            sprintf(pictype_str,"B");
                            break;
                        default:
                            sprintf(pictype_str,"Other");
                            break;
                    }
                    //LOGI("Frame Index: %5d. Type:%s",frame_cnt,pictype_str);
                    frame_cnt++;
                }
            }
            av_free_packet(packet);
        }

        //flush decoder
        //FIX: Flush Frames remained in Codec
        while(1){
            ret = avcodec_decode_video2(pCodecCtx,pFrame,&got_picture,packet);
            if(ret < 0) break;
            if(!got_picture) break;
            sws_scale(img_convert_ctx,(const uint8_t* const*)pFrame->data,
                    pFrame->linesize,0,pCodecCtx->height,pFrameYUV->data,
                    pFrameYUV->linesize);
            int y_size=pCodecCtx->width*pCodecCtx->height;
            fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);     //Y
            fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);   //U
            fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);   //V

            //Output info
            char pictype_str[10] = {0};
            switch(pFrame->pict_type){
                case AV_PICTURE_TYPE_I:
                    sprintf(pictype_str,"I");
                    break;
                case AV_PICTURE_TYPE_P:
                    sprintf(pictype_str,"P");
                    break;
                case AV_PICTURE_TYPE_B:
                    sprintf(pictype_str,"B");
                    break;
                default:
                    sprintf(pictype_str,"Other");
                    break;
            }
            //LOGI("Frame Index: %5d. Type:%s",frame_cnt,pictype_str);
            frame_cnt++;
        }
        time_finish = clock();
        time_duration = (double)(time_finish - time_start);

        sprintf(info,"%s[Time       ]%fms\n",info,time_duration);
        sprintf(info,"%s[Count      ]%d\n",info,frame_cnt);

        sws_freeContext(img_convert_ctx);

        fclose(fp_yuv);

        av_frame_free(&pFrameYUV);
        av_frame_free(&pFrame);
        avcodec_close(pCodecCtx);
        avformat_close_input(&pFormatCtx);

        return 0;
  }

  // java中的jstring, 转化为c的一个字符数组
  char*   Jstring2CStr(JNIEnv*   env,   jstring   jstr)
  {
       char*   rtn   =   NULL;
       jclass   clsstring   =   (*env)->FindClass(env,"java/lang/String");
       jstring   strencode   =   (*env)->NewStringUTF(env,"UTF-8");
       jmethodID   mid   =   (*env)->GetMethodID(env,clsstring,   "getBytes",   "(Ljava/lang/String;)[B");
       jbyteArray   barr=   (jbyteArray)(*env)->CallObjectMethod(env,jstr,mid,strencode); // String .getByte("GB2312");
       jsize   alen   =   (*env)->GetArrayLength(env,barr);
       jbyte*   ba   =   (*env)->GetByteArrayElements(env,barr,JNI_FALSE);
       if(alen   >   0)
       {
        rtn   =   (char*)malloc(alen+1);         //new   char[alen+1]; "\0"
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
       }
       (*env)->ReleaseByteArrayElements(env,barr,ba,0);  //释放内存

       return rtn;
  }

JNIEXPORT jint JNICALL Java_com_ys_ffmpeg_ffmpegsimple_util_FFmpegUtil_playVideo
  (JNIEnv *env, jobject jcls, jstring jstr1, jobject surface){

        //LOGD("start playvideo... url");
        char* live_url;
        //"http://123.108.164.110/etv1sb/phd10497/playlist.m3u8";
        //http://125.227.22.115/sta/323033.m3u8
        live_url = Jstring2CStr(env,jstr1);;
        av_register_all();
        AVFormatContext * pFormatCtx = avformat_alloc_context();
        //Open video file
        if(avformat_open_input(&pFormatCtx,live_url,NULL,NULL) != 0){
            //LOGE("Couldn't open file:%s\n",live_url);
            return -1; // Couldn't open file
        }
        //Retrieve stream information
        if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
            //LOGE("Couldn't find stream information");
            return -1;
        }
        //Find the first video stream
        int videoStream = -1,i;
        for(i = 0; i < pFormatCtx->nb_streams;i++){
            if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
               videoStream < 0)
               {
                    videoStream = i;
               }
        }
        if(videoStream == -1){
            //LOGE("Didn't find a video stream");
            return -1;//Didn't find a video stream
        }
        //Get a pointer to the codec context for the video stream
        AVCodecContext * pCodecCtx = pFormatCtx->streams[videoStream]->codec;
        //Find the decoder for the video stream
        AVCodec * pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec==NULL){
            //LOGE("Codec not found");
            return -1;//Codec not found
        }
        if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0){
            //LOGE("Codec not open codec");
            return -1;//Could not open codec
        }
        //获取native window
        ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env,surface);
        //获取视频宽高
        int videoWidth = pCodecCtx->width;
        int videoHeight = pCodecCtx->height;
        //设置native window的buffer大小，可自动拉伸
        ANativeWindow_setBuffersGeometry(nativeWindow,videoWidth,videoHeight,WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_Buffer windowBuffer;
        if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0){
            //LOGE("Could not open codec");
            return -1;//Could not open codec
        }
        //Allocate video frame
        AVFrame* pFrame = av_frame_alloc();
        //用于渲染
        AVFrame* pFrameRGBA = av_frame_alloc();
        if(pFrameRGBA == NULL || pFrame == NULL){
            //LOGE("Could not allocate video frame");
            return -1;
        }
        //Determine required buffer size and allocate buffer
        //buffer中数据就是用于渲染的，且格式为RGBA
        int numBytes=av_image_get_buffer_size(
            AV_PIX_FMT_RGBA,
            pCodecCtx->width,
            pCodecCtx->height,
            1
        );
        uint8_t * buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
            av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                                 pCodecCtx->width, pCodecCtx->height, 1);

        //由于解码出来的帧格式不是RGBA的，在渲染之前需要进行格式转换
        struct SwsContext *sws_ctx = sws_getContext(
            pCodecCtx->width,
            pCodecCtx->height,
            pCodecCtx->pix_fmt,
            pCodecCtx->width,
            pCodecCtx->height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL);

        int frameFinished;
        AVPacket packet;
        while(av_read_frame(pFormatCtx,&packet)>= 0){
            //IS this a packet from the video stream ?
            if(packet.stream_index == videoStream){
                //Decode video frame
                avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished,&packet);
                //并不是decode 一次就可解码出一帧
                if(frameFinished){
                    //lock native window buffer
                    ANativeWindow_lock(nativeWindow,&windowBuffer,0);
                    //格式转换
                    sws_scale(sws_ctx,
                               (uint8_t const * const *)pFrame->data,
                               pFrame->linesize,0,pCodecCtx->height,
                               pFrameRGBA->data,pFrameRGBA->linesize
                    );
                    //获取stride
                    uint8_t * dst = windowBuffer.bits;
                    int dstStride= windowBuffer.stride * 4;
                    uint8_t * src = (uint8_t*)(pFrameRGBA->data[0]);
                    int srcStride = pFrameRGBA->linesize[0];
                    //由于window的stride和帧的stride不同，因此需要逐行复制
                    int h;
                    for(h = 0; h < videoHeight;h++){
                        memcpy(dst + h * dstStride,src + h * srcStride,srcStride);
                    }
                    ANativeWindow_unlockAndPost(nativeWindow);
                }
            }
            av_packet_unref(&packet);
        }
        av_free(buffer);
        av_free(pFrameRGBA);
        //Free the YUV frame
        av_free(pFrame);
        //Close the codecs
        avcodec_close(pCodecCtx);
        //Close the video file
        avformat_close_input(&pFormatCtx);
        return 0;
  }