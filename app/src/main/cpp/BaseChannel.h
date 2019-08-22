//
// Created by womob on 2019/8/13.
//

#ifndef FFMPEGPLAYER_BASECHANNEL_H
#define FFMPEGPLAYER_BASECHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
};

#include "safe_queue.h"

/**
 * VideoChannel和AudioChannel的父类
 */
class BaseChannel {

public:
    BaseChannel(int id,AVCodecContext *codecContext,AVRational time_base) : id(id),codecContext(codecContext),time_base(time_base) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    /**
     * 释放AVPacket
     * @param packet
     */
    static void releaseAVPacket(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    /**
     * 释放AVFrame
     * @param frame
     */
    static void releaseAVFrame(AVFrame **frame) {
        if (frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    //纯虚函数（类似java抽象方法）
    virtual void start() = 0;
    virtual void stop() = 0;

    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    int id;
    bool isPlaying = 0;
    AVCodecContext *codecContext;
    AVRational time_base;
    double audio_time;
};


#endif //FFMPEGPLAYER_BASECHANNEL_H
