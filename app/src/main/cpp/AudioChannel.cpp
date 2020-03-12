//
// Created by Administrator on 2020/3/12.
//

#include <cstring>
#include "AudioChannel.h"


void AudioChannel::setAudioCallBack(AudioCallBack audioCallBack) {
    this->audioCallBack = audioCallBack;
}

void AudioChannel::encodeData(int8_t *data) {
    //编码 数据存放在 buffer中
    int bytelen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples, buffer, maxOutputBytes);
    if (bytelen > 0) {  //有数据
        //看表
        int bodySize = bytelen + 2;//编码后的大小
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, bodySize);
        //前两个数据
        packet->m_body[0] = 0xAF;
        if (mChannels == 1) {   //单通道
            packet->m_body[0] = 0xAE;
        }
        packet->m_body[1] = 0x01;
        //后面的数据
        memcpy(&packet->m_body[2], buffer, bytelen);


        //aac
        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        audioCallBack(packet);
    }

}

void AudioChannel::setAudioEncInfo(int sampleInHz, int channels) {
    audioCodec = faacEncOpen(sampleInHz, channels, &inputSamples, &maxOutputBytes);
    //设置参数
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioCodec);
    //编码版本
    config->mpegVersion = MPEG4;//目前最主流的版本
    //编码标准
    config->aacObjectType = LOW;
    //位数
    config->inputFormat = FAAC_INPUT_16BIT;
    //编码出原始数据  Bitstream output format (0 = Raw; 1 = ADTS)
    config->outputFormat = 0;
    faacEncSetConfiguration(audioCodec, config);

    //缓冲区
    buffer = new u_char(maxOutputBytes);
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

//音频需要提前发送一帧给服务器
RTMPPacket *AudioChannel::getAudioTag() {
    u_char *buf;
    u_long len;
    //当前的编码器信息
    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &len);
    int bodySize = 2 + len;

    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);
    //前两个数据
    packet->m_body[0] = 0xAF;
    if (mChannels == 1) {   //单通道
        packet->m_body[0] = 0xAE;
    }
    packet->m_body[1] = 0x01;
    //后面的数据
    memcpy(&packet->m_body[2], buf, len);


    //aac
    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    return packet;
}
