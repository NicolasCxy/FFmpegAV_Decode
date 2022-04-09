#include <jni.h>
#include <string>

#include <android/native_window_jni.h>
#include <android/log.h>
#include "HBBFFmpeg.h"
#include "HBBCallJava.h"
#include "HbbPlayStatus.h"


#define LOGD(...) __android_log_print(ANDROID_LOG_INFO,"cxy",__VA_ARGS__)

HBBFFmpeg *ffmpeg = NULL;
HBBCallJava *callJava = NULL;
HbbPlayStatus *playStatus = NULL;
_JavaVM *javaVM = NULL;


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {

        return result;
    }
    return JNI_VERSION_1_4;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cxy_audioplay_HbbPlayer_n_1parpared(JNIEnv *env, jobject thiz, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    if (NULL == ffmpeg) {
        if (callJava == NULL) {
            callJava = new HBBCallJava(javaVM,env,&thiz);
        }
        playStatus = new HbbPlayStatus();
        ffmpeg = new HBBFFmpeg(playStatus,callJava,source);
        ffmpeg->parpared();
    }

    env->ReleaseStringUTFChars(source_, source);
}


extern "C" JNIEXPORT void JNICALL
Java_com_cxy_audioplay_HbbPlayer_n_1start(JNIEnv *env, jobject thiz) {
    if(NULL != ffmpeg){
        ffmpeg->start();
    }
}
