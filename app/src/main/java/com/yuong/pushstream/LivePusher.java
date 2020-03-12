package com.yuong.pushstream;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.yuong.pushstream.meida.AudioChannel;
import com.yuong.pushstream.meida.VideoChannel;

public class LivePusher {
    private AudioChannel audioChannel;
    private VideoChannel videoChannel;

    static {
        System.loadLibrary("native-lib");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        native_init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);
    }

    public void setPreviewDisplay(SurfaceHolder holder) {
        videoChannel.setPreviewDisplay(holder);
    }

    //切换摄像头
    public void switchCamera() {
        videoChannel.switchCamera();
    }

    //设置参数
    public void setVideoEncInfo(int width, int height, int fps, int bitrate) {
        native_setVideoEncInfo(width, height, fps, bitrate);
    }

    //开始直播
    void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();
    }

    public void pushVideo(byte[] data) {
        native_pushVideo(data);
    }

    public void pushAudio(byte[] data) {
        native_pushAudio(data);
    }

    public void setAudioEncInfo(int samplesHz, int channels){
        native_setAudioEncInfo(samplesHz, channels);
    }

    /**
     * 获取Faac 初始化的最小缓冲区大小
     * @return
     */
    public int getInputSamples() {
        return native_getInputSamples();
    }

    private native void native_init();

    private native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);

    private native void native_start(String path);

    private native void native_pushVideo(byte[] data);

    private native void native_pushAudio(byte[] data);

    private native void native_setAudioEncInfo(int samplesHz, int channels);

    private native int native_getInputSamples();


}
