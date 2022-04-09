//
// Created by DELL on 2022/4/2.
//

#include "HBBFFmpeg.h"

HBBFFmpeg::HBBFFmpeg(HbbPlayStatus *playstatus, HBBCallJava *callJava, const char *url) {
    this->playstatus = playstatus;
    this->callJava = callJava;
    this->url = url;
}

void *decodeFFmpeg(void *data) {
    HBBFFmpeg *hbbfFmpeg = (HBBFFmpeg *) data;
    hbbfFmpeg->decodeFFmpegThread();
    pthread_exit(&hbbfFmpeg->decodeThread);
}

void HBBFFmpeg::parpared() {
    //开启子线程初始化FFmpage
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

void HBBFFmpeg::decodeFFmpegThread() {
    //初始化FFmpage模块，并通知上层
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    //打开文件
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open url :%s", url);
        }
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not find streams from %s", url);
        }
        return;
    }
    //找到音轨
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                //初始化音频模块
                audio = new HBBAudio(playstatus, pFormatCtx->streams[i]->codecpar->sample_rate);
                audio->streamIndex = i;
                audio->codecpar = pFormatCtx->streams[i]->codecpar;
            }
        }
    }

    //找到编码器
    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);
    if (!dec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        return;
    }

    //获取编码器上下文
    audio->avCodecContext = avcodec_alloc_context3(dec);
    if (!audio->avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decodecctx");
        }
        return;
    }

    //获取编码信息
    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decodecctx");
        }
        return;
    }

    //打开编码器
    if (avcodec_open2(audio->avCodecContext, dec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        return;
    }
    callJava->onCallParpared(CHILD_THREAD);
}

void HBBFFmpeg::start() {
    /**
     * 1.通知audio初始化openSL并开启，准备从queue中读取数据
     * 2.开始从音频轨里读取数据（压缩数据）
     * 3.放到queue中，Audio中OpenSL主动从quque中读取数据
     * 4.
     */
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("audio is null");
            return;
        }
    }
    int count = 0;
    audio->play();
    //开始从音频轨里读取数据（压缩数据）
    while (playstatus != NULL && !playstatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            //将数据放到audio中的queue队列中
            if (avPacket->stream_index == audio->streamIndex) {
                //解码操作
                count++;
                if (LOG_DEBUG) {
//                    LOGE("解码第 %d 帧", count);
                }
                audio->queue->putAvpacket(avPacket);
            }else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            //生产者读完了，还需要等待消费者处理完
            while (playstatus != NULL && !playstatus->exit) {
                //队列数据处理完了，在将状态置为false
                if (audio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    playstatus->exit = false;
                    break;
                }
            }
        }
    }


}
