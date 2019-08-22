//
// Created by womob on 2019/8/13.
// 可以重新编译ffmpeg 编译一部分  Ctrl+C 终止编译查看
//D:\android\androidStudioSdk\android-sdk-windows\ndk-bundle\platforms\android-21\arch-arm\usr\lib  libz.so
//D:\android\androidStudioSdk\android-ndk-r19c\toolchains\arm-linux-androideabi-4.9\prebuilt\windows-x86_64\bin  nm
//




#include "ZLFFmpeg.h"


ZLFFmpeg::ZLFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource) {
    this->javaCallHelper = javaCallHelper;
    this->dataSource = dataSource;
    //dataSource 是java传过来的字符串，通过jni接口转成C++字符串
    //在jni方法中释放掉了，导致dataSource变成悬空指针（指向一块已经释放的内存）
    //解决方法：内存拷贝
    this->dataSource = new char[strlen(
            dataSource + 1)];//strlen获取字符串长度   C字符串以\0结尾“hello\0”   java  "hello"
    strcpy(this->dataSource, dataSource);

}

ZLFFmpeg::~ZLFFmpeg() {
    DELETE(dataSource);
    DELETE(javaCallHelper);
}

/**
 * 准备线程的执行函数
 * @param args
 * @return
 */
void *task_prepare(void *args) {
    //打开输入
    ZLFFmpeg *ffmpeg = static_cast<ZLFFmpeg *>(args);
    ffmpeg->_prepare();

    return 0;//线程执行方法必须返回0！！！！！！！！！！！！！！！！！！！！
}

void *task_start(void *args) {
    //打开输入
    ZLFFmpeg *ffmpeg = static_cast<ZLFFmpeg *>(args);
    ffmpeg->_start();

    return 0;//线程执行方法必须返回0！！！！！！！！！！！！！！！！！！！！
}

void ZLFFmpeg::_prepare() {
    formatContext = avformat_alloc_context();
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "10000000", 0);//设置超时时间10s  单位微秒
    //1、、、、打开媒体
    int ret = avformat_open_input(&formatContext, dataSource, 0, &dictionary);
    av_dict_free(&dictionary);
    if (ret) {
        //失败   回调给java
        LOGE("打开媒体失败：%s", av_err2str(ret));
        //javaCallHelper  jni 回调java
        //javaCallHelper->onError(ret);
        //可能java层需要根据errorCode来更新UI
        //作业反射通知java
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);//打开文件失败

    }

    //2、、、查找媒体中的流信息
    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //3、、、获取媒体流  音频/视频
        AVStream *stream = formatContext->streams[i];
        //4、、、获取编解码这段流的参数
        AVCodecParameters *codecParameters = stream->codecpar;
        //5、、、通过参数中的（编解码的方式），来查找当前流的解码器
        AVCodec *avCodec = avcodec_find_decoder(codecParameters->codec_id);
        if (!avCodec) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        //6、、、创建解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
        //7、、、设置解码器上下文的参数
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        //8、、、打开解码器
        ret = avcodec_open2(codecContext, avCodec, 0);
        if (ret) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        //判断流类型（音频还是视频？）
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频
            audioChannel = new AudioChannel(i,codecContext);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频

            AVRational frame_rate = stream->avg_frame_rate;
//            int fps = frame_rate.num/frame_rate.den;
             int fps = av_q2d(frame_rate);
            videoChannel = new VideoChannel(i,codecContext,fps);
            videoChannel->setRenderCallback(renderCallback);
        }

    }

    if (!audioChannel && !videoChannel) {
        //既没有音频也没有视频
        if (ret) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
            return;
        }
    }

    //准备好了，反射通知java
    javaCallHelper->onPrepared(THREAD_CHILD);
}

/**
 * 播放准备
 */
void ZLFFmpeg::prepare() {
    //创建子线程
    pthread_create(&pid_prepare, 0, task_prepare, this);

}

/**
 * 开始播放
 */
void ZLFFmpeg::start() {
    isPlaying = 1;
    if (videoChannel) {
        videoChannel->start();
    }
    if (audioChannel) {
        audioChannel->start();
    }
    pthread_create(&pid_start, 0, task_start, this);
}

/**
 * 真正执行解码播放
 */
void ZLFFmpeg::_start() {
//    AVPacket *packet = av_packet_alloc();
    while (isPlaying) {

        /**
         * 内存泄漏   控制packets队列
         */
        if(videoChannel->packets.size()>100){
            av_usleep(10*1000);
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(formatContext, packet);
        if (!ret) {
            //ret为0表示成功
            //判断流类型，是视频还是音频
            if (videoChannel && packet->stream_index == videoChannel->id) {
                //往视频编码数据包队列中添加数据
                videoChannel->packets.push(packet);
            } else if (audioChannel && packet->stream_index == audioChannel->id) {
                //往音频编码数据包队列中添加数据
                audioChannel->packets.push(packet);

            }
        } else if (ret == AVERROR_EOF) {
            //表示读完了
        } else {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_READ_PACKETS_FAIL);
            LOGE("读取音视频数据包失败");
            break;
        }
    }//end while
    isPlaying = 0;
    //停止解码播放（音频、视频）
    videoChannel->stop();
    audioChannel->stop();
}

void ZLFFmpeg::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;

}
//
