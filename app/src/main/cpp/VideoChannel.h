//
// Created by Administrator on 2020/3/8.
//负责视频解码和视频推送
//

#ifndef PUSHSTREAMDEMO_VIDEOCHANNEL_H
#define PUSHSTREAMDEMO_VIDEOCHANNEL_H

#include <cstdint>
#include "PrintLog.h"
#include "librtmp/rtmp.h"
#include <x264.h>
#include <string.h>


class VideoChannel {
    typedef void (*VideoCallBack)(RTMPPacket *packet);

public:
    void setVideoEncInfo(int width, int height, int fps, int bitrate);

    void encodeData(int8_t *data);

    void setVideoCallBack(VideoCallBack videoCallBack);


private:
    int mWidth;//宽
    int mHeight;//高
    int mFps;//帧率
    int mBitrate;//码率
    int ySize;//当前帧有多少个 Y
    int uvSize;//当前帧有多少个 UV
    x264_t *videoCodec;//编码器
    x264_picture_t *pic_in;//一帧图片
    void sendSpsPps(uint8_t sps[100], uint8_t pps[100], int spslen, int ppslen);

    VideoCallBack videoCallBack;

    void sendFrame(int type, uint8_t *payload, int iPayload);
};


#endif //PUSHSTREAMDEMO_VIDEOCHANNEL_H
