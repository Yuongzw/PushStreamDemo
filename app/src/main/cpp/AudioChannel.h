//
// Created by Administrator on 2020/3/12.
//

#ifndef PUSHSTREAMDEMO_AUDIOCHANNEL_H
#define PUSHSTREAMDEMO_AUDIOCHANNEL_H

#include <sys/types.h>
#include "librtmp/rtmp.h"
#include "faac.h"

class AudioChannel {
    typedef void (*AudioCallBack)(RTMPPacket *packet);
public:
    void setAudioCallBack(AudioCallBack audioCallBack);

    void encodeData(int8_t *data);

    void setAudioEncInfo(int sampleInHz, int channels);

    int getInputSamples();

    RTMPPacket *getAudioTag();//首先要给一帧空的帧


private:
    int mChannels;
    faacEncHandle audioCodec;//解码器
    u_long inputSamples;    //最小缓冲区大小
    u_long maxOutputBytes;  //最大缓冲区大小
    u_char *buffer = 0;
    AudioCallBack audioCallBack;
};


#endif //PUSHSTREAMDEMO_AUDIOCHANNEL_H
