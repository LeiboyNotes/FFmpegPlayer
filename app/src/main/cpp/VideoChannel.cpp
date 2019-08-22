//
// Created by womob on 2019/8/13.
//




#include "VideoChannel.h"

VideoChannel::VideoChannel(int id, AVCodecContext *codecContext,int fps) : BaseChannel(id, codecContext) {
    this->fps = fps;
}

VideoChannel::~VideoChannel() {

}

/**
 * 解码线程的执行函数
 * @param args
 * @return
 */
void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();

    return 0;//线程执行方法必须返回0！！！！！！！！！！！！！！！！！！！！
}

/**
 * 播放线程的执行函数
 * @param args
 * @return
 */
void *task_video_play(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();

    return 0;//线程执行方法必须返回0！！！！！！！！！！！！！！！！！！！！
}

void VideoChannel::start() {

    isPlaying = 1;
    //设置队列状态
    packets.setWork(1);
    frames.setWork(1);
    //解码
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    //播放
    pthread_create(&pid_video_play, 0, task_video_play, this);
}

/**
 * 视频解码
 */
void VideoChannel::stop() {

}

void VideoChannel::video_decode() {
    AVPacket *packet = 0;
//    AVFrame *frame = av_frame_alloc();
    while (isPlaying) {

        int ret = packets.pop(packet);
        if (!isPlaying) {
            //如果停止播放，跳出循环
            break;
        }
        if (!ret) {
            //取数据包失败
            continue;
        }
        //拿到视频数据包（编码压缩的），需要把数据包丢给解码器
        ret = avcodec_send_packet(codecContext, packet);
        if (ret) {
            //往解码器发送数据包失败，跳出循环
            break;
        }
        releaseAVPacket(&packet);//释放packet

        //TODO
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            //往解码器发送数据包失败   跳出循环
            continue;
        } else if (ret != 0) {
            break;
        }
        //ret ==0;数据收发正常，成功获取到  解码后的视频原始数据包  AVFrame  yuv格式
        //对frame进行处理
        /**
         * 内存泄漏处理
         */
        while(isPlaying&&frames.size()>100){
            av_usleep(10*1000);
            continue;
        }
        frames.push(frame);
    }
    releaseAVPacket(&packet);
}

void VideoChannel::video_play() {

    AVFrame *frame = 0;
    //原始数据格式转换  yuv->rgb

    uint8_t *dst_data[4];
    int dst_linesize[4];
    SwsContext *sws_ctx = sws_getContext(codecContext->width, codecContext->height,
                                         codecContext->pix_fmt,
                                         codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
                                         SWS_BILINEAR, NULL, NULL, NULL);
    //dst_data  dst_linesize申请内存
    av_image_alloc(dst_data, dst_linesize, codecContext->width, codecContext->height,
                   AV_PIX_FMT_RGBA, 1);

    //根据fps（传入的流的平均帧率来控制每一帧的延时时间）
    //sleep:fps--->时间
    //单位秒
    double delay_time_per_frame = 1.0/fps;
    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        //取到yuv原始数据转换格式
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, codecContext->height, dst_data,
                  dst_linesize);
        //进行休眠
        //每一帧还有自己的额外延时时间
        //extra_delay = repeat_pict / (2*fps)
        double extra_delay = frame->repeat_pict / (2*fps);
        double real_delay = extra_delay+delay_time_per_frame;
        av_usleep(real_delay*1000000);
        //dst_data  AV_PIX_FMT_RGBA 格式的数据
        //渲染回调出去
        renderCallback(dst_data[0], dst_linesize[0], codecContext->width, codecContext->height);
        releaseAVFrame(&frame);

    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);

}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}
