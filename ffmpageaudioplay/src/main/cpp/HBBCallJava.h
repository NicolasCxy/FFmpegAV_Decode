//
// Created by DELL on 2022/4/2.
//

#ifndef HBB_FFMPAGE_HBBCALLJAVA_H
#define HBB_FFMPAGE_HBBCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class HBBCallJava {
public:
    _JavaVM *javaVm = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj = NULL;
    jmethodID jmid_parpared = NULL;
public:
    HBBCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~HBBCallJava();


    void onCallParpared(int type);

};


#endif //HBB_FFMPAGE_HBBCALLJAVA_H
