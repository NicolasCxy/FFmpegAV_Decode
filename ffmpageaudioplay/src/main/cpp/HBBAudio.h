//
// Created by DELL on 2022/4/2.
//

#ifndef HBB_FFMPAGE_HBBAUDIO_H
#define HBB_FFMPAGE_HBBAUDIO_H


#include "HbbPlayStatus.h"
#include "HBBQueue.h"
#include "AndroidLog.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include <libswresample/swresample.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};


class HBBAudio {
public:
    HbbPlayStatus *playstatus = NULL;
    int sample_rate = 0;
    int dataSize = 0;
    HBBQueue *queue = NULL;
    int streamIndex = -1;
    int ret = 0;
    uint8_t *buffer = NULL;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *avCodecContext = NULL;

    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;


public:
    HBBAudio(HbbPlayStatus *playstatus, int sample_rate);

    ~HBBAudio();

    void play();

    void initOpenSLES();

    int getCurrentSampleRateForOpensles(int sample_rate);

    int resampleAudio();
};


#endif //HBB_FFMPAGE_HBBAUDIO_H
