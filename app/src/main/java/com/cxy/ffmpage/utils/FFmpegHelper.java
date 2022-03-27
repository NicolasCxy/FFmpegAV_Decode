package com.cxy.ffmpage.utils;

import android.content.Context;

public class FFmpegHelper {
    private Context mContext;
    private static FFmpegHelper mInstance;

    public FFmpegHelper(Context mContext) {
        this.mContext = mContext;
    }

    public static synchronized FFmpegHelper getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new FFmpegHelper(context);
        }
        return mInstance;
    }

    public void init(){
        
    }
}
