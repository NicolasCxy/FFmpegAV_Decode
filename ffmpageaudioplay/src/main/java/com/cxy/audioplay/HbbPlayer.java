package com.cxy.audioplay;

import android.text.TextUtils;

import com.cxy.audioplay.listener.WlOnParparedListener;
import com.cxy.audioplay.log.MyLog;

public class HbbPlayer {

    private String source;
    private WlOnParparedListener mnOnParparedListener;

    public HbbPlayer() {
    }

    /**
     * 设置数据源
     *
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }


    /**
     * 设置准备接口回调
     *
     * @param mnOnParparedListener
     */
    public void setWlOnParparedListener(WlOnParparedListener mnOnParparedListener) {
        this.mnOnParparedListener = mnOnParparedListener;
    }

    public void parpared() {
        if (TextUtils.isEmpty(source)) {
            MyLog.d("source not be empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();
    }

    public void start(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }



    //native 回调
    public void onCallParpared() {
        if (mnOnParparedListener != null) {
            mnOnParparedListener.onParpared();
        }
    }


    public native void n_parpared(String source);

    public native void n_start();
}
