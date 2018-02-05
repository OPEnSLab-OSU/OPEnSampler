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
import android.widget.Toast;

import opensampler.opensampler.MainActivity;
import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ScheduleFragment extends Fragment {
    private static final String TAG = "ScheduleFragment";
    private String temp = "";
    private Button btnNavFrag2;
    private Button btnNavSecondActivity;
    private Button btnSetSched;
	private static final String KEY_LAYOUT_MANAGER = "layoutManager";
    private static final int count = 10;
    private static final int span = 2;
    private EditText periodLength;
    private EditText sampleLength;
    private EditText numDays;
    public static EditText startingHour;
    private static EditText startingMin;
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
        View view = inflater.inflate(R.layout.schedule_frag, container, false);

        //Buttons, EditText and Textview declarations for schedule_frag.xml
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mSchedAdapt = new ScheduleAdapter(mDataset);
        btnNavFrag2 = (Button) view.findViewById(R.id.btnNavFrag2);
        btnNavSecondActivity = (Button) view.findViewById(R.id.btnNavSecondActivity);
        btnSetSched = (Button) view.findViewById(R.id.setSchedType);
        schedMenu = (Spinner) view.findViewById(R.id.spinSchedMenu);
        numDays = (EditText) view.findViewById(R.id.numDays);


        sampleLength = (EditText) view.findViewById(R.id.sampleLength);
        periodLength = (EditText) view.findViewById(R.id.periodLength);
        startingHour = (EditText) view.findViewById(R.id.startingHour);
        startingMin = (EditText) view.findViewById(R.id.startingMinute);

        //Button click listener functions for all three buttons on the screen
        Log.d(TAG, "onCreateView: Started.");
        btnNavFrag2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                Toast.makeText(getActivity(), "Going to Samples", Toast.LENGTH_SHORT).show();
                ((MainActivity)getActivity()).setViewPager(1);            }
        });
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
                    sampleLength.setText("");
                    periodLength.setText("");
                    numDays.setText("");
                    numDays.setEnabled(true);
                    sampleLength.setEnabled(true);
                    periodLength.setEnabled(true);
                }
                if(temp.equals("Daily")){
                    Log.d(TAG, "Daily was hit!");
                    sampleLength.setText("");
                    numDays.setText("");
                    periodLength.setText("1440");
                    sampleLength.setEnabled(true);
                    numDays.setEnabled(true);
                    periodLength.setEnabled(false);
                }
                if(temp.equals("Custom")){
                    Log.d(TAG, "Custom was hit!");
                    numDays.setText("");
                    sampleLength.setText("");
                    periodLength.setText("");
                    sampleLength.setEnabled(true);
                    periodLength.setEnabled(true);
                    numDays.setEnabled(true);
                }
            }
        });
        return view;
    }
}