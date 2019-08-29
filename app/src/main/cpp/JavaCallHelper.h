//
// Created by womob on 2019/8/13.
//

#ifndef FFMPEGPLAYER_JAVACALLHELPER_H
#define FFMPEGPLAYER_JAVACALLHELPER_H

#include <jni.h>
#include "macro.h"

class JavaCallHelper {

public:
    JavaCallHelper(JavaVM *javaVM_, JNIEnv *env_, jobject instance_);

    ~JavaCallHelper();

    void onPrepared(int threadMode);
    void onError(int threadMode,int errorCode);
    void onProgress(int threadMode, int progress);
private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject instance;
    jmethodID jmd_prepared;
    jmethodID jmd_error;
    jmethodID jmd_onProgress;



};


#endif //FFMPEGPLAYER_JAVACALLHELPER_H
