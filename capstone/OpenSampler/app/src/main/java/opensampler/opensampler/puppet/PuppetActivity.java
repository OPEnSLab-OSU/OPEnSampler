package opensampler.opensampler.puppet;


import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class PuppetActivity extends AppCompatActivity {

    private static final String TAG = "PuppetActivity";
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.puppet_activity_layout);
        Log.d(TAG, "onCreate: Started.");
    }
}

