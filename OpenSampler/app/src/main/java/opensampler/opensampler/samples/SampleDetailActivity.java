package opensampler.opensampler.samples;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 2/5/2018.
 */

public class SampleDetailActivity extends AppCompatActivity {
    private TextView mSampleActual;
    private TextView mTimeActual;
    private TextView mSampleLabel;
    private TextView mTimeLabel;
    private Button mBackButton;

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sample_detail);
        mTimeActual = (TextView) findViewById(R.id.sampleTimeActual);
        mSampleActual = (TextView) findViewById(R.id.sampleTimeActual);
        mBackButton = (Button) findViewById(R.id.sampleBackButton);
        mBackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
    }
}
