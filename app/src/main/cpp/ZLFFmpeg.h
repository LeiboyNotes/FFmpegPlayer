//
// Created by womob on 2019/8/13.
//

#ifndef FFMPEGPLAYER_ZLFFMPEG_H
#define FFMPEGPLAYER_ZLFFMPEG_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

#include "macro.h"
#include <cstring>
#include <pthread.h>

extern "C"{
#include <libavformat/avformat.h>

};

class ZLFFmpeg {

public:
    ZLFFmpeg(JavaCallHelper *javaCallHelpe,char *dataSource);

    ~ZLFFmpeg();

    void prepare();
    void _prepare();

    void start();

    void _start();
    void setRenderCallback(RenderCallback renderCallback);

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    bool isPlaying;
    AVFormatContext *formatContext = 0;
    RenderCallback renderCallback;


};


#endif //FFMPEGPLAYER_ZLFFMPEG_H
