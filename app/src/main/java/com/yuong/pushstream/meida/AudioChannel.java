package com.yuong.pushstream.meida;


import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import com.yuong.pushstream.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord audioRecord;
    private int channels = 2;
    private ExecutorService executorService;
    private boolean isLiving;
    int bufferSize;
    int inputSamples;

    public AudioChannel(LivePusher livePusher) {
        executorService = Executors.newSingleThreadExecutor();
        mLivePusher = livePusher;
        int channelConfig;
        if (channels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;//双通道
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO;//单通道
        }
        livePusher.setAudioEncInfo(44100, channels);
        inputSamples = livePusher.getInputSamples() * 2;
        bufferSize = AudioRecord.getMinBufferSize(44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT) * 2;
        Log.w("yuongzw", "inputSamples=" + inputSamples);
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT, bufferSize > inputSamples ? inputSamples : bufferSize);
    }

    public void setChannels(int channels) {
        this.channels = channels;
    }

    public void startLive() {
        isLiving = true;
        executorService.submit(new AudioTask());
    }

    private class AudioTask implements Runnable {
        @Override
        public void run() {
            audioRecord.startRecording();
            byte[] bytes = new byte[bufferSize > inputSamples ? inputSamples : bufferSize];
            while (isLiving) {
                audioRecord.read(bytes, 0, bytes.length);
                mLivePusher.pushAudio(bytes);
            }

        }
    }
}
