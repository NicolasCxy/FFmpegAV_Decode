//
// Created by DELL on 2022/4/2.
//

#include "HBBAudio.h"

HBBAudio::HBBAudio(HbbPlayStatus *playstatus, int sample_rate) {
    this->playstatus = playstatus;
    this->sample_rate = sample_rate;

    LOGE("CC_rate : %d",sample_rate);
    //初始缓存队列
    queue = new HBBQueue(playstatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);
}

HBBAudio::~HBBAudio() {

}

void *decoPlaye(void *data) {
    HBBAudio *hbbAudio = (HBBAudio *) data;
    hbbAudio->initOpenSLES();
    pthread_exit(&hbbAudio->thread_play);
}

void HBBAudio::play() {
    pthread_create(&thread_play, NULL, decoPlaye, this);

}

//喇叭需要数据了会自动回调这里
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    HBBAudio *hbbAudio = (HBBAudio *) context;
    if (hbbAudio != NULL) {
        LOGD("pcmBufferCallBack - start");
        int buffSize = hbbAudio->resampleAudio();
        LOGE("pcmBufferCallBack %d", buffSize);
        if(buffSize > 0){
            (*hbbAudio->pcmBufferQueue)->Enqueue(hbbAudio->pcmBufferQueue,hbbAudio->buffer,buffSize);
        }
    }
}


//开始解码转换
int HBBAudio::resampleAudio() {

    while (playstatus != NULL && !playstatus->exit) {
        //将压缩数据从队列里读取出来
        avPacket = av_packet_alloc();
        if (queue->getAvpacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        //解码
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0) {
            if (avFrame->channels && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }


            //创建转换上下文
            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                         avFrame->sample_rate, avFrame->channel_layout,
                                         (AVSampleFormat) avFrame->format, avFrame->sample_rate,
                                         NULL, NULL);

            if (!swr_ctx || swr_init(swr_ctx) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                continue;
            }

            int nb = swr_convert(swr_ctx, &buffer, avFrame->nb_samples,
                                 (const uint8_t **) avFrame->data, avFrame->nb_samples);

//            LOGE("转化数据 %d", dataSize);
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            dataSize = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            if(LOG_DEBUG)
            {
                LOGE("data_size is %d", dataSize);
            }
            //释放资源
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;

        } else{
            //释放资源
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        };
    }

    return dataSize;
}


void HBBAudio::initOpenSLES() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //创建混音器
    const SLInterfaceID mids[1] = {SL_IID_OUTPUTMIX};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        (void) result;
    }
//    (void) result;
//    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
//                                              &outputMixEnvironmentalReverb);
//    if (SL_RESULT_SUCCESS == result) {
//        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
//                outputMixEnvironmentalReverb, &reverbSettings);
//        (void) result;
//    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};


    // 配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sample_rate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource slDataSource = {&android_queue, &pcm};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //创建播放器
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk,
                                       1,
                                       ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    //    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);

    // 开始播放
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
}


int HBBAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}


