package opensampler.opensampler.schedule;
/**
 * Created by Godtop on 1/23/2018.
 */
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TimePicker;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ScheduleActivity extends AppCompatActivity {

    private static final String TAG = "ScheduleActivity";;
    private TimePicker mTimePicker;
    private Button submitTime;
    private int hourPick = 0;
    private int minutePick = 0;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        //sets schedule_activity_layout.xml as the page being viewed
        setContentView(R.layout.schedule_activity_layout);
        //log messages used for debugging, goes unseen
        Log.d(TAG, "onCreate: Started.");
        Log.d(TAG, "AFTER THE BUTTON");
        //variables for the clock times that are picked
        mTimePicker = (TimePicker) findViewById(R.id.timePicker);
        submitTime = (Button) findViewById(R.id.timeSubmit);
        //Send the values gained from picking a time on the timePicker
        sendItems();
    }

    //Function that sends all the items gained when hitting Send
    private void sendItems(){
        //onClickListener for when the button gets clicked
        submitTime.setOnClickListener(new View.OnClickListener() {
            @RequiresApi(api = Build.VERSION_CODES.M)
            @Override
            public void onClick(View view) {
                //temp strings to be used for sending
                String value1 = "";
                String value2 = "";
                //values selected according to the values selected from the timePicker
                hourPick = mTimePicker.getHour();
                minutePick = mTimePicker.getMinute();
                //Sets the strings to the times picked
                value1 = Integer.toString(hourPick);
                value2 = Integer.toString(minutePick);
                //creates a bundle that will contain the selection
                Bundle args = new Bundle();
                //Puts selection in the bundle
                args.putString("hour", value1);
                args.putString("minute", value2);
                //Takes the bundle to the fragment
                ScheduleFragment.putArguments(args);
                //finishes
                finish();
            }
        });
    }
}