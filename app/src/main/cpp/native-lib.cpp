#include <jni.h>
#include <string>
#include <pthread.h>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
#include "queue.h"

VideoChannel *videoChannel;
AudioChannel *audioChannel;
int isStart = 0;//是否开启直播
pthread_t pthread;
uint32_t startTime;//推流的开始时间
int readyPush = 0;//可以推流标识位
SaveQueue<RTMPPacket *> packets;//队列

//回调接口
void callback(RTMPPacket *packet) {
    if (packet) {
        //设置时间戳
        packet->m_nTimeStamp = RTMP_GetTime() - startTime;
        //加入队列
        packets.put(packet);
    }
}

void releasePacket(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

void *start(void *args) {
    char *url = static_cast<char *>(args);
    //链接服务器
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("alloc rtmp失败");
        return NULL;
    }
    //初始化
    RTMP_Init(rtmp);
    //设置url地址
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("设置URL地址失败:%s", url);
        return NULL;
    }
    //设置连接超时
    rtmp->Link.timeout = 5;
    //设置rtmp 可写入
    RTMP_EnableWrite(rtmp);
    //连接服务器
    ret = RTMP_Connect(rtmp, 0);
    if (!ret) {
        LOGE("连接服务器失败:%s", url);
        return NULL;
    }
    //连接流
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接流失败:%s", url);
        return NULL;
    }
    //记录推流的开始时间
    startTime = RTMP_GetTime();
    //表示可以推流了
    readyPush = 1;
    packets.setWork(1);
    //数据包
    RTMPPacket *packet = 0;
    callback(audioChannel->getAudioTag());
    while (readyPush) {
        //队列里面取数据
        packets.get(packet);
        LOGD("取出一帧数据");
        if (!readyPush) {
            break;
        }
        if (!packet) {
            continue;
        }
        //设置packet数据的流类型  音频还是视频流
        packet->m_nInfoField2 = rtmp->m_stream_id;
        //发送一个数据包
        ret = RTMP_SendPacket(rtmp, packet, 1);
        //释放packet
        releasePacket(packet);
    }
    isStart = 0;
    readyPush = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete url;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yuong_pushstream_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallBack(callback);
    audioChannel = new AudioChannel;
    audioChannel->setAudioCallBack(callback);
}extern "C"
JNIEXPORT void JNICALL
Java_com_yuong_pushstream_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject thiz, jint width,
                                                             jint height, jint fps, jint bitrate) {
    if (!videoChannel) {
        LOGD("videoChannel为空");
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);
}extern "C"
JNIEXPORT void JNICALL
Java_com_yuong_pushstream_LivePusher_native_1start(JNIEnv *env, jobject thiz, jstring path) {
    const char *path_ = env->GetStringUTFChars(path, 0);
    if (isStart) {
        return;
    }
    isStart = 1;
    char *url = new char[strlen(path_) + 1];
    strcpy(url, path_);
    //开启线程来直播
    pthread_create(&pthread, 0, start, url);
    env->ReleaseStringUTFChars(path, path_);
}extern "C"
JNIEXPORT void JNICALL
Java_com_yuong_pushstream_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray data) {

    if (!videoChannel || !readyPush) {
        LOGE("初始化还没完成");
        return;
    }
    jbyte *data_ = env->GetByteArrayElements(data, NULL);

    videoChannel->encodeData(data_);


    env->ReleaseByteArrayElements(data, data_, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_yuong_pushstream_LivePusher_native_1pushAudio(JNIEnv *env, jobject thiz, jbyteArray data) {

    jbyte *data_ = env->GetByteArrayElements(data, NULL);
    if (!audioChannel || !readyPush) {
        LOGE("初始化还没完成");
        return;
    }
    audioChannel->encodeData(data_);

    env->ReleaseByteArrayElements(data, data_, 0);

}extern "C"
JNIEXPORT void JNICALL
Java_com_yuong_pushstream_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject thiz,
                                                             jint samples_hz, jint channels) {
    if (audioChannel) {
        audioChannel->setAudioEncInfo(samples_hz, channels);
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_yuong_pushstream_LivePusher_native_1getInputSamples(JNIEnv *env, jobject thiz) {
    if (audioChannel) {
        return audioChannel->getInputSamples();
    }
    LOGD("audioChanel为空");
    return -1;
}