//----------------------------------------------------------------------------------
// File:        NvAppBase/java/com/nvidia/NvAppBase/NvAppBase.java
// SDK Version: v1.2 
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------
package com.nvidia.NvAppBase;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;

import android.app.NativeActivity;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;
import java.lang.Runnable;

public class NvAppBase extends NativeActivity
{
    public String mCommandLine = "";

    @Override
    protected void onCreate (Bundle savedInstanceState)
    {
        Intent launchIntent = getIntent();
        String extra = launchIntent.getStringExtra("arguments");
        if (extra != null) {
            mCommandLine = extra;
            Log.v("NvAppBase", "command line = " + mCommandLine);
        }
        super.onCreate(savedInstanceState);

        Log.v("NvAppBase", "Calling NvAppBase onCreate");
    }
    
    // We call this function from native to display a toast string
    public void showAlert(String title, String contents, boolean exitApp)
    {
        // We need to use a runnable here to ensure that when the spawned
        // native_app_glue thread calls, we actually post the work to the UI
        // thread.  Otherwise, we'll likely get exceptions because there's no
        // prepared Looper on the native_app_glue main thread.
        final String finalTitle = title;
        final String finalContents = contents;
        final boolean finalExit = exitApp;
        runOnUiThread(new Runnable() {
            public void run()
            {
                AlertDialog.Builder builder = new AlertDialog.Builder(NvAppBase.this);
                builder.setMessage(finalContents)       
                        .setTitle(finalTitle)       
                        .setCancelable(true)
                        .setPositiveButton("OK", new DialogInterface.OnClickListener()
                    {           
                        public void onClick(DialogInterface dialog, int id) {
                            dialog.cancel();
                            if (finalExit)
                                NvAppBase.this.finish();
                        }       
                    });

                builder.create().show();
            }
        });
    }
}
