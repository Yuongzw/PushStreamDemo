package com.yuong.pushstream;

import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private LivePusher livePusher;
    private SurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = findViewById(R.id.surfaceView);
        livePusher = new LivePusher(this, 800, 480, 800_000, 10, Camera.CameraInfo.CAMERA_FACING_FRONT);
        livePusher.setPreviewDisplay(surfaceView.getHolder());
    }



    public void switchCamera(View view) {
    }

    public void startLive(View view) {
        String path = "";
        livePusher.startLive(path);
    }

    public void stopLive(View view) {
    }
}
