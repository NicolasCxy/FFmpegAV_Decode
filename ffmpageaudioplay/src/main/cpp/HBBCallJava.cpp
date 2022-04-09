//
// Created by DELL on 2022/4/2.
//

#include "HBBCallJava.h"

HBBCallJava::HBBCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVm = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jlz = jniEnv->GetObjectClass(jobj);
    if (!jlz) {
        if (LOG_DEBUG) {
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_parpared = env->GetMethodID(jlz, "onCallParpared", "()V");

}

void HBBCallJava::onCallParpared(int type) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
    } else if (type == CHILD_THREAD) {
        JNIEnv *otherJniEnv;
        if (javaVm->AttachCurrentThread(&otherJniEnv, 0) != JNI_OK) {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        otherJniEnv->CallVoidMethod(jobj,jmid_parpared);
        javaVm->DetachCurrentThread();
    }



}
