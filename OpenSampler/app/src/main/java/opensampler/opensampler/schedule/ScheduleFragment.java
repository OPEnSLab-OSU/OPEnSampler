package opensampler.opensampler.schedule;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ScheduleFragment extends Fragment {
    private static final String TAG = "ScheduleFragment";
    private String temp = "";
    private Button btnNavFrag2;
    private Button btnSetSched;
	private static final String KEY_LAYOUT_MANAGER = "layoutManager";
    private static final int count = 10;
    private static final int span = 2;
    private EditText numDays;

    //Periodic
    private EditText periodLengthPeriodic;
    private TextView periodLengthPeriodicText;
    private EditText sampleLengthPeriodic;
    private TextView sampleLengthPeriodicText;
    private EditText flushDurationPeriodic;
    private TextView flushDurationPeriodicText;

    //Daily
    private static EditText startingHour;
    private TextView startingHourText;
    private static EditText startingMin;
    private TextView startingMinText;
    private EditText sampleLengthDaily;
    private TextView sampleLengthDailyText;
    private EditText flushDurationDaily;
    private TextView flushDurationDailyText;
    private Button btnNavSecondActivity;
    private TextView timeSelectionText;

    private Spinner schedMenu;
    private String mParam1;

	private enum LayoutManagerType {
		GRID_LAYOUT_MANAGER,
        LINEAR_LAYOUT_MANAGER
	}

	protected LayoutManagerType mCurrentLayoutManagerType;
    protected ScheduleAdapter mSchedAdapt;
    protected String[] mDataset;

	@Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        if(getArguments() != null){
            mParam1 = getArguments().getString("params");
        }
        initDataset();
    }

    private void initDataset() {
            mDataset = new String[count];
            for (int i = 0; i < count; i++) {
                mDataset[i] = "This is schedule #" + i;
            }
    }

    public static void putArguments(Bundle args){
        String startHour = args.getString("hour");
        String startMinute = args.getString("minute");
        startingMin.setText(startMinute);
        startingHour.setText(startHour);
    }

    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState){
        final View view = inflater.inflate(R.layout.schedule_frag, container, false);

        //Declarations for schedule_frag.xml
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mSchedAdapt = new ScheduleAdapter(mDataset);
        btnSetSched = (Button) view.findViewById(R.id.setSchedType);
        schedMenu = (Spinner) view.findViewById(R.id.spinSchedMenu);

        //Daily
        sampleLengthDaily = (EditText) view.findViewById(R.id.sampleLengthDaily);
        sampleLengthDailyText = (TextView) view.findViewById(R.id.sampleLengthDailyText);
        flushDurationDaily = (EditText) view.findViewById(R.id.flushDurationDaily);
        flushDurationDailyText = (TextView) view.findViewById(R.id.flushDurationDailyText);
        startingHour = (EditText) view.findViewById(R.id.startingHourDaily);
        startingHourText = (TextView) view.findViewById(R.id.startingHourDailyText);
        startingMin = (EditText) view.findViewById(R.id.startingMinuteDaily);
        startingMinText = (TextView) view.findViewById(R.id.startingMinuteDailyText);
        btnNavSecondActivity = (Button) view.findViewById(R.id.timeSelectorActivity);
        timeSelectionText = (TextView) view.findViewById(R.id.timeSelectionText);

        //Periodic
        periodLengthPeriodic = (EditText) view.findViewById(R.id.periodLengthPeriod);
        periodLengthPeriodicText = (TextView) view.findViewById(R.id.periodLengthPeriodText);
        sampleLengthPeriodic = (EditText) view.findViewById(R.id.sampleLengthPeriod);
        sampleLengthPeriodicText = (TextView) view.findViewById(R.id.sampleLengthPeriodText);
        flushDurationPeriodic = (EditText) view.findViewById(R.id.flushDurationPeriod);
        flushDurationPeriodicText = (TextView) view.findViewById(R.id.flushDurationPeriodText);

        btnNavSecondActivity.setVisibility(view.VISIBLE);
        timeSelectionText.setVisibility(view.VISIBLE);
        startingHour.setVisibility(view.VISIBLE);
        startingHourText.setVisibility(view.VISIBLE);
        startingMin.setVisibility(view.VISIBLE);
        startingMinText.setVisibility(view.VISIBLE);
        sampleLengthDaily.setVisibility(view.VISIBLE);
        sampleLengthDailyText.setVisibility(view.VISIBLE);
        flushDurationDaily.setVisibility(view.VISIBLE);
        flushDurationDailyText.setVisibility(view.VISIBLE);
        sampleLengthDaily.setEnabled(true);
        flushDurationDaily.setEnabled(true);

        //Hide all the Periodic view items
        periodLengthPeriodic.setVisibility(view.INVISIBLE);
        periodLengthPeriodicText.setVisibility(view.INVISIBLE);
        flushDurationPeriodic.setVisibility(view.INVISIBLE);
        flushDurationPeriodicText.setVisibility(view.INVISIBLE);
        sampleLengthPeriodic.setVisibility(view.INVISIBLE);
        sampleLengthPeriodicText.setVisibility(view.INVISIBLE);


        // This is for a secondary activity inside the schedule in case we want to add that later
        btnNavSecondActivity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                Toast.makeText(getActivity(), "Going to Schedule Activity", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(getActivity(), ScheduleActivity.class);
                startActivity(intent);

            }
        });

        btnSetSched.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                temp = schedMenu.getSelectedItem().toString();
                if(temp.equals("Periodic")){
                    Log.d(TAG, "Periodic was hit!");
                    //Hide all the Daily View Items
                    btnNavSecondActivity.setVisibility(view.INVISIBLE);
                    timeSelectionText.setVisibility(view.INVISIBLE);
                    startingHour.setVisibility(view.INVISIBLE);
                    startingHourText.setVisibility(view.INVISIBLE);
                    startingMin.setVisibility(view.INVISIBLE);
                    startingMinText.setVisibility(view.INVISIBLE);
                    sampleLengthDaily.setVisibility(view.INVISIBLE);
                    sampleLengthDailyText.setVisibility(view.INVISIBLE);
                    flushDurationDaily.setVisibility(view.INVISIBLE);
                    flushDurationDailyText.setVisibility(view.INVISIBLE);
                    //Show all the Periodic View Items
                    periodLengthPeriodic.setVisibility(view.VISIBLE);
                    periodLengthPeriodicText.setVisibility(view.VISIBLE);
                    flushDurationPeriodic.setVisibility(view.VISIBLE);
                    flushDurationPeriodicText.setVisibility(view.VISIBLE);
                    sampleLengthPeriodic.setVisibility(view.VISIBLE);
                    sampleLengthPeriodicText.setVisibility(view.VISIBLE);
                }
                if(temp.equals("Daily")){
                    Log.d(TAG, "Daily was hit!");
                    //Show all the daily items
                    btnNavSecondActivity.setVisibility(view.VISIBLE);
                    timeSelectionText.setVisibility(view.VISIBLE);
                    startingHour.setVisibility(view.VISIBLE);
                    startingHourText.setVisibility(view.VISIBLE);
                    startingMin.setVisibility(view.VISIBLE);
                    startingMinText.setVisibility(view.VISIBLE);
                    sampleLengthDaily.setVisibility(view.VISIBLE);
                    sampleLengthDailyText.setVisibility(view.VISIBLE);
                    flushDurationDaily.setVisibility(view.VISIBLE);
                    flushDurationDailyText.setVisibility(view.VISIBLE);
                    //Hide all the Periodic view items
                    periodLengthPeriodic.setVisibility(view.INVISIBLE);
                    periodLengthPeriodicText.setVisibility(view.INVISIBLE);
                    flushDurationPeriodic.setVisibility(view.INVISIBLE);
                    flushDurationPeriodicText.setVisibility(view.INVISIBLE);
                    sampleLengthPeriodic.setVisibility(view.INVISIBLE);
                    sampleLengthPeriodicText.setVisibility(view.INVISIBLE);
                }
            }
        });
        return view;
    }
}

