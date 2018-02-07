package opensampler.opensampler.schedule;
/**
 * Created by Godtop on 1/23/2018.
 */
import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import opensampler.opensampler.R;
import opensampler.opensampler.schedule.ScheduleFragment;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ScheduleActivity extends AppCompatActivity {

    private static final String TAG = "ScheduleActivity";
    private EditText hour;
    private TextView hourText;
    private EditText minute;
    private TimePicker mTimePicker;
    private Button submitTime;
    private int hourPick = 0;
    private int minutePick = 0;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.schedule_activity_layout);
        Log.d(TAG, "onCreate: Started.");
        Log.d(TAG, "AFTER THE BUTTON");
        hourText = (TextView) findViewById(R.id.startHourText);
        minute = (EditText) findViewById(R.id.startingMinute);
        mTimePicker = (TimePicker) findViewById(R.id.timePicker);
        submitTime = (Button) findViewById(R.id.timeSubmit);
        sendItems();
    }

    private void sendItems(){
        submitTime.setOnClickListener(new View.OnClickListener() {
            @RequiresApi(api = Build.VERSION_CODES.M)
            @Override
            public void onClick(View view) {
                String value1 = "";
                String value2 = "";
                hourPick = mTimePicker.getHour();
                minutePick = mTimePicker.getMinute();
                value1 = Integer.toString(hourPick);
                value2 = Integer.toString(minutePick);
                Bundle args = new Bundle();
                args.putString("hour", value1);
                args.putString("minute", value2);
                ScheduleFragment.putArguments(args);
                finish();
            }
        });
    }
}
