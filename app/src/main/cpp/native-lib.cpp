#include <jni.h>
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include <libavutil/time.h>
}


#include <android/native_window_jni.h>
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_INFO,"David",__VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_cxy_ffmpage_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
//    std::string hello = "Hello from C++";
    std::string configInfo = avcodec_configuration();
    return env->NewStringUTF(configInfo.c_str());
}

static AVFormatContext *avFormatContext;
static AVCodecContext *avCodecContext;
static SwsContext *swsContext;
AVCodec *vCode;
ANativeWindow *nativeWindow; //渲染对象
//缓存队列，用来编解码
static AVFrame *avFrame, *rgbFrame;
static AVPacket *avPacket;
uint8_t *outbuffer;
ANativeWindow_Buffer windowBuffer;

//解码播放
extern "C" JNIEXPORT jint JNICALL
Java_com_cxy_ffmpage_MainActivity_play(JNIEnv *env, jobject thiz, jstring url_, jobject surface) {
    /**
     *  1.转换字符串，surface对象转换成windows
     *  2.注册ffmpage所有组件
     *  3.打开视频文件
     *  4.找到视频轨
     *  5.初始化解码器
     *  6.创建解码所需的buffer，packet(压缩数据)，deCodeFrame（解压缩数据），RgbFrame（经过转换的数据）
     *  7.创建转换器 将原始数据转换为surface分辨率
     *  8.开始解码，rgbFrame渲染到surface上面
     */
    const char *url = env->GetStringUTFChars(url_, 0);
    nativeWindow = ANativeWindow_fromSurface(env, surface);

    //注册所有组件
    avcodec_register_all();
    avFormatContext = avformat_alloc_context();
    //打开视频文件
    if (avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        LOGD("Couldn't open input stream.\n");
        return -1;
    }
    LOGD("打开视频成功.\n");

    //查找文件的流信息  mp4  失败  的视频
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGD("Couldn't find stream information.\n");
        return -1;
    }

    //找到音频轨
    int videoindex = -1;
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
        }
    }

    if (videoindex == -1) {
        LOGD("Couldn't find a video stream.\n");
        return -1;
    }
    LOGD("找到视频轨.\n");

    //找到编码器
    avCodecContext = avFormatContext->streams[videoindex]->codec;
    vCode = avcodec_find_decoder(avCodecContext->codec_id);

    //打开编码器
    if (avcodec_open2(avCodecContext, vCode, NULL) < 0) {
        LOGD("Couldn't open codec.\n");
    }
    LOGD("打开编码器成功.\n");

    //创建各个buffer
    avFrame = av_frame_alloc();

    rgbFrame = av_frame_alloc();

    avPacket = av_packet_alloc();

    int width = avCodecContext->width;
    int height = avCodecContext->height;

    //计算解码大小
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_ARGB, width, height, 1);
    LOGD("计算解码后的rgb %d ,width : %d, height: %d", numBytes,width,height);
    //创建缓存区方便avframe数据到 -> rgbFrame
    outbuffer = static_cast<uint8_t *>(av_malloc(numBytes * sizeof(uint8_t)));
    //进行关联
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, outbuffer, AV_PIX_FMT_RGBA, width,
                         height, 1);


    //创建转换器
    swsContext = sws_getContext(width, height, avCodecContext->pix_fmt, width, height,
                                AV_PIX_FMT_RGBA,
                                SWS_BICUBIC, NULL, NULL, NULL);

    if (ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888) <
        0) {
        LOGD("Couldn't set buffers geometry.\n");
        ANativeWindow_release(nativeWindow);
        return -1;
    }

    LOGD("ANativeWindow_setBuffersGeometry成功\n");

    //轮询读取数据
    while (av_read_frame(avFormatContext, avPacket) >= 0) {

        //暂时只处理视频数据
        if (avPacket->stream_index == videoindex) {
            //开始解码，渲染到surface上面
            int ret = avcodec_send_packet(avCodecContext, avPacket);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                LOGD("解码出错");
                return -1;
            }
            ret = avcodec_receive_frame(avCodecContext, avFrame);
            if (ret == AVERROR(EAGAIN)) {     //还有数据
                continue;
            } else if (ret < 0) {
                break;
            }

            //将解压数据进行缩放
            sws_scale(swsContext, avFrame->data, avFrame->linesize, 0,
                      avCodecContext->height, rgbFrame->data, rgbFrame->linesize);

            if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) < 0) {
                LOGD("cannot lock window");
            } else {
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                for (int h = 0; h < height; h++) {
                    memcpy(dst + h * windowBuffer.stride * 4, outbuffer + h * rgbFrame->linesize[0],
                           rgbFrame->linesize[0]);
                }
            }

            switch (avFrame->pict_type) {
                case AV_PICTURE_TYPE_I:
                    LOGD("I");
                    break;
                case AV_PICTURE_TYPE_P:
                    LOGD("P");
                    break;
                case AV_PICTURE_TYPE_B:
                    LOGD("B");
                    break;
                default:;
                    break;
            }
            ANativeWindow_unlockAndPost(nativeWindow);
        }

        av_usleep(1000 * 8);

    }

    avformat_free_context(avFormatContext);
    env->ReleaseStringUTFChars(url_, url);

    return -1;
}
