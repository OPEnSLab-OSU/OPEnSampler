package opensampler.opensampler.connections;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 2/5/2018.
 */

public class ConnectionDetailActivity extends AppCompatActivity {
    private TextView mNameActual;
    private TextView mBlueActual;
    private TextView mNameLabel;
    private TextView mBlueLabel;
    private Button mBackButton;

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.connection_detail);
        mNameActual = (TextView) findViewById(R.id.nameActual);
        mBlueActual = (TextView) findViewById(R.id.sampleTimeActual);
        mBackButton = (Button) findViewById(R.id.connectBackButton);
        mBackButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
    }
}
