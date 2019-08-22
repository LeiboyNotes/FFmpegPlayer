//
// Created by womob on 2019/8/13.
//

#include "JavaCallHelper.h"


JavaCallHelper::JavaCallHelper(JavaVM *javaVM_, JNIEnv *env_, jobject instance_) {
    this->javaVM = javaVM_;
    this->env = env_;
    //一旦涉及到jobject跨方法，跨线程  需要创建全局引用
//    this->instance = instance_;
    this->instance = env->NewGlobalRef(instance_);
    jclass clazz = env->GetObjectClass(instance);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_error = env->GetMethodID(clazz, "onError", "(I)V");

}

JavaCallHelper::~JavaCallHelper() {
    javaVM = 0;
    env->DeleteGlobalRef(instance);
    instance = 0;

}

void JavaCallHelper::onPrepared(int threadMode) {

    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_prepared);
    } else {
        //子线程
        //当前子线程的  JNIEnv
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_prepared);


        javaVM->DetachCurrentThread();
    }
}

void JavaCallHelper::onError(int threadMode,int errorCode) {
    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_error);
    } else {
        //子线程
        //当前子线程的  JNIEnv
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_error,errorCode);


        javaVM->DetachCurrentThread();
    }
}
