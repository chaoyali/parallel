/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.neon;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.example.xgzhu.mylittlelibrary.MyView;
import com.example.xgzhu.myneonlibrary.NeonWrapper;

public class HelloNeon extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
//        View v = new MyView(this);
//        setContentView(v);

        NeonWrapper w = new NeonWrapper();
        TextView  tv = new TextView(this);
        tv.setText( w.hello() );
        setContentView(tv);


//        TextView  tv = new TextView(this);
//        tv.setText( stringFromJNI() );
//        setContentView(tv);
    }

    /* A native method that is implemented by the
     * 'helloneon' native library, which is packaged
     * with this application.
     */
    public native String stringFromJNI();


    /* this is used to load the 'helloneon' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.neon/lib/libhelloneon.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("helloneon");
    }
}
