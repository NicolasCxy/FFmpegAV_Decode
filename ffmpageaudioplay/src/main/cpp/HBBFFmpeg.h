//
// Created by DELL on 2022/4/2.
//

#ifndef HBB_FFMPAGE_HBBFFMPEG_H
#define HBB_FFMPAGE_HBBFFMPEG_H


#include "HbbPlayStatus.h"
#include "HBBCallJava.h"
#include "pthread.h"
#include "HBBAudio.h"

extern "C"
{
#include "libavformat/avformat.h"
};

class HBBFFmpeg {
public:
    HBBCallJava *callJava = NULL;
    const char *url = NULL;
    HbbPlayStatus *playstatus = NULL;
    pthread_t decodeThread = NULL;
    HBBAudio *audio = NULL;
    AVFormatContext * pFormatCtx = NULL;

public:
    HBBFFmpeg(HbbPlayStatus *playstatus, HBBCallJava *callJava, const char *url);

    ~HBBFFmpeg();

    void parpared();
    void decodeFFmpegThread();
    void start();


};


#endif //HBB_FFMPAGE_HBBFFMPEG_H
