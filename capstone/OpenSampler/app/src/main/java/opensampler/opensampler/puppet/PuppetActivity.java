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
    //Simple sets all the required functionality
    private static final String TAG = "PuppetActivity";

    //Enabled when Fragment is created
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.puppet_activity_layout); //Sets the layout to puppet_activity_layout.xml
        Log.d(TAG, "onCreate: Started."); //Basic Tag
    }
}

