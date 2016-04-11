package com.example.xgzhu.myneonlibrary;

/**
 * Created by xgzhu on 4/10/16.
 */
public class NeonWrapper {
    public NeonWrapper () {}
    public String hello() {
        return "Hello from neon library\n" + stringFromJNI();
    }
    public native String stringFromJNI();
    static {
        System.loadLibrary("myneonlibrary");
    }
}
