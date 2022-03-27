package com.cxy.ffmpage;

import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.DataBindingUtil;
import androidx.databinding.ViewDataBinding;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.TextView;

import com.cxy.ffmpage.databinding.ActivityMainBinding;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    private ActivityMainBinding mainBind;
    private SurfaceHolder holder;
    private Surface surface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mainBind = DataBindingUtil.setContentView(this, R.layout.activity_main);
        checkPermission();
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        initSurface();

    }

    private void initSurface() {
        holder = mainBind.svSurface.getHolder();
        holder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                //获取文件路径，这里将文件放置在手机根目录下
                surface = holder.getSurface();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }


    public boolean checkPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.CAMERA
            }, 1);

        }
        return false;
    }


    public native String stringFromJNI();

    public void play(View view) {
        String folderUrl = new File(Environment.getExternalStorageDirectory(), "666666.mp4").getAbsolutePath();
        play(folderUrl, surface);
    }

    public native int play(String url, Surface surface);
}