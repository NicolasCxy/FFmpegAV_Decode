package com.cxy.audioplay;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import com.cxy.audioplay.listener.WlOnParparedListener;

public class MainActivity extends AppCompatActivity {

    private HbbPlayer mHbbPlayer;


    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mHbbPlayer = new HbbPlayer();
        mHbbPlayer.setWlOnParparedListener(() -> {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(MainActivity.this, "准备好了", Toast.LENGTH_SHORT).show();
                    mHbbPlayer.start();
                }
            });
        });
    }

    public void start(View view) {
        mHbbPlayer.setSource("http://music.163.com/song/media/outer/url?id=1827109585.mp3");
        mHbbPlayer.parpared();
    }
}