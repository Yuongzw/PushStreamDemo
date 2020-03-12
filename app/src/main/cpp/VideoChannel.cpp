//
// Created by Administrator on 2020/3/8.
//


#include "VideoChannel.h"


void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;
    ySize = width * height;
    uvSize = ySize / 4;

    //初始化x264的编码器
    x264_param_t param;
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    //编码复杂度
    param.i_level_idc = 32;//一般取这个值
    //输入数据格式
    param.i_csp = X264_CSP_I420;
    //宽高
    param.i_width = width;
    param.i_height = height;
    //无B帧 直播一般设置为0
    param.i_bframe = 0;
    //i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    param.rc.i_rc_method = X264_RC_ABR;
    //码率，比特率（单位 Kbps）
    param.rc.i_bitrate = bitrate / 1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    //设置了i_vbv_max_bitrate 必须设置此参数，码率控制区大小，单位Kbps
    param.rc.i_vbv_buffer_size = bitrate / 1000;

    //帧率的分子
    param.i_fps_num = fps;
    //帧率的分母（单位：s）  分子/分母 = 帧率
    param.i_fps_den = 1;
    //时间基分子
    param.i_timebase_num = param.i_fps_den;
    //时间基分母
    param.i_timebase_den = param.i_fps_num;
    //vfr输入  1:时间基和时间戳用于码率控制    0:只用于码率控制
    param.b_vfr_input = 0;
    //关键帧间隔（单位:s） 就是上一个I帧到下一个I帧之间的间距
    param.i_keyint_max = fps * 2;
    //是否复制fps和品pps放在每个关键帧的前面，该参数设置是让每个关键帧（I帧）都附带fps/pps
    param.b_repeat_headers = 1;
    //并行编码多帧，线程数；如果为0就自动开启多线程编码
    param.i_threads = 1;
    //设置编码质量   直播要求时时性，所以编码质量要求不高，最基本的即可
    x264_param_apply_profile(&param, "baseline");
    //打开解码器
    videoCodec = x264_encoder_open(&param);
    //初始化一帧图片
    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);

}

//解码  NV21 ---》 YUV I420
void VideoChannel::encodeData(int8_t *data) {
    //数据  data    容器  pic_in
    memcpy(pic_in->img.plane[0], data, ySize);//把 Y 数据拷贝到第0个元素
    for (int i = 0; i < uvSize; ++i) {
        //U 数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1);
        //V 数据
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2);
    }

    //编码出来的数据  NALU单元数组
    x264_nal_t *pp_nal;
    //编码出来有几个数据（多少个NALU单元）
    int pi_nal;
    //输出的图片
    x264_picture_t *pic_put;
    //编码
    x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, pic_put);
    int spslen;
    int ppslen;
    uint8_t sps[100];
    uint8_t pps[100];
    for (int i = 0; i < pi_nal; ++i) {
        if (pp_nal[i].i_type == NAL_SPS) {
            spslen = pp_nal[i].i_payload - 4;   //由于sps数据前面有00 00 00 01 4个字节数据 所以减去4
            memcpy(sps, pp_nal[i].p_payload + 4, spslen);
        } else if (pp_nal[i].i_type == NAL_PPS) {
            ppslen = pp_nal[i].i_payload - 4;   //由于pps数据前面有00 00 00 01 4个字节数据 所以减去4
            memcpy(pps, pp_nal[i].p_payload + 4, ppslen);
            sendSpsPps(sps, pps, spslen, ppslen);
        } else {
            //非关键帧与非关键帧
            sendFrame(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
        }
    }
}

void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int spslen, int ppslen) {
    //sps pps --> packet
    RTMPPacket *packet = new RTMPPacket;
    int bodySize = 13 + spslen + 3 + ppslen;
    RTMPPacket_Alloc(packet, bodySize);

    int i = 0;
    //固定头
    //类型
    packet->m_body[i++] = 0x17;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    //以下根据表格来进行编码
    //版本
    packet->m_body[i++] = 0x01;
    //编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;
    //整个sps
    packet->m_body[i++] = 0xE1;
    //sps长度  高8位和低8位
    packet->m_body[i++] = (spslen >> 8) & 0xff;
    packet->m_body[i++] = spslen & 0xff;
    memcpy(&packet->m_body[i], sps, spslen);
    i += spslen;
    //pps
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (ppslen >> 8) & 0xff;
    packet->m_body[i++] = ppslen & 0xff;
    memcpy(&packet->m_body[i], pps, ppslen);

    //数据类型
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    //分配管道
    packet->m_nChannel = 10;
    //sps pps没有时间戳
    packet->m_nTimeStamp = 0;
    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    if (videoCallBack) {
        videoCallBack(packet);
    }

}

void VideoChannel::sendFrame(int type, uint8_t *payload, int iPayload) {
    if (payload[2] == 0x00) {
        iPayload -= 4;
        payload -= 4;
    } else {
        iPayload -= 3;
        payload -= 3;
    }
    //看表
    int bodySize = 9 + iPayload;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);

    packet->m_body[0] = 0x27;
    if (type == NAL_SLICE_IDR) {    //关键帧
        packet->m_body[0] = 0x17;
        LOGD("关键帧");
    }
    //类型
    packet->m_body[1] = 0x01;
    //时间戳
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;
    //数据长度 int 4个字节
    packet->m_body[5] = (iPayload >> 24) & 0xff;
    packet->m_body[6] = (iPayload >> 16) & 0xff;
    packet->m_body[7] = (iPayload >> 8) & 0xff;
    packet->m_body[8] = (iPayload) & 0xff;
    //图片数据
    memcpy(&packet->m_body[9], payload, iPayload);

    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    //数据类型
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nChannel = 0x10;
    if (videoCallBack) {
        videoCallBack(packet);
    }

}

void VideoChannel::setVideoCallBack(VideoCallBack videoCallBack) {
    this->videoCallBack = videoCallBack;
}

