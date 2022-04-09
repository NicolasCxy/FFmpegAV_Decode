//
// Created by DELL on 2022/4/2.
//

#ifndef HBB_FFMPAGE_HBBQUEUE_H
#define HBB_FFMPAGE_HBBQUEUE_H


extern "C"
{
#include "libavcodec/avcodec.h"
};


#include "queue"
#include "pthread.h"
#include "AndroidLog.h"
#include "HbbPlayStatus.h"

class HBBQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    HbbPlayStatus *playstatus = NULL;
public:
    HBBQueue(HbbPlayStatus *playstatus);
    ~HBBQueue();

    int putAvpacket(AVPacket *packet);
    int getAvpacket(AVPacket *packet);

    int getQueueSize();
};


#endif //HBB_FFMPAGE_HBBQUEUE_H
