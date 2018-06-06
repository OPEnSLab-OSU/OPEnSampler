/* Some of the code used in this application falls under the copyright notice below. To view the original source code please use the following link:
 * https://github.com/NordicPlayground/Android-nRF-UART
 *
 * Copyright (c) 2015, Nordic Semiconductor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package opensampler.opensampler.schedule;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.Text;

import java.io.UnsupportedEncodingException;
import java.text.DateFormat;
import java.util.Date;

import opensampler.opensampler.BluetoothService;
import opensampler.opensampler.DeviceListActivity;
import opensampler.opensampler.MainActivity;
import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ScheduleFragment extends Fragment {
    private static final String TAG = "ScheduleFragment"; //Tag message
    private String temp = "";
    private Button btnSetSched; //Button to change the type of schedule
    private Button msubmitSched; //Button to use the BLE commands to send the schedule variables
    private Button btnConnect; //Button to connect a BLE connection
    private static final int count = 10; //count variable, very good for counting

    //Form variables for setting a Periodic Schedule
    private EditText periodLengthPeriodic; //Period Length
    private TextView periodLengthPeriodicText; //Period Length Label
    private EditText sampleLengthPeriodic; //Sample Length
    private TextView sampleLengthPeriodicText; //Sample Length Label
    private EditText flushDurationPeriodic; //Flush Duration
    private TextView flushDurationPeriodicText; //Flush Duration Label

    //Form variables for setting a Daily Schedule
    private static EditText startingHour; //Starting Hour
    private TextView startingHourText; //Starting Hour Label
    private static EditText startingMin; //Starting Minute
    private TextView startingMinText; //Starting Minute Label
    private EditText sampleLengthDaily; //Sample Length
    private TextView sampleLengthDailyText; //Sample Length Label
    private EditText flushDurationDaily; //Flush Duration
    private TextView flushDurationDailyText; //Flush Duration Label
    private Button btnNavSecondActivity; //Button that opens the timePicker activity
    private TextView timeSelectionText; //

    private Spinner schedMenu; //Dropdown menu with both types of the schedule
    private String mParam1;

    //Hunter BT stuff
    private static final int REQUEST_SELECT_DEVICE = 1;
    private static final int REQUEST_ENABLE_BT = 2;
    private static final int UART_PROFILE_READY = 10;
    private static final int UART_PROFILE_CONNECTED = 20;
    private static final int UART_PROFILE_DISCONNECTED = 21;
    private static final int STATE_OFF = 10;
    private int mState = UART_PROFILE_DISCONNECTED;
    private BluetoothService mService = null;


    private BluetoothDevice mDevice = null;
    private BluetoothAdapter mBtAdapter = null;
    private ListView messageListView;
    private ArrayAdapter<String> listAdapter;
    private EditText edtMessage;
    TextView mRemoteRssiVal;
    RadioGroup mRg;

	private enum LayoutManagerType {
		GRID_LAYOUT_MANAGER,
        LINEAR_LAYOUT_MANAGER //Used to specify what kind of layout
	}

	protected LayoutManagerType mCurrentLayoutManagerType; // Layout Manager
    protected ScheduleAdapter mSchedAdapt; // Schedule Adapter
    protected String[] mDataset; // That THICC Data

	@Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); //Created this fragment
        if(getArguments() != null){
            mParam1 = getArguments().getString("params"); //Not used, but required
        }
        initDataset(); //Initiate the dataset

        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if(mBtAdapter == null){
            Toast.makeText(getActivity(), "Bluetooth is unavailable", Toast.LENGTH_SHORT).show();
            return;
        }
        messageListView = (ListView) getActivity().findViewById(R.id.listMessage);
        listAdapter = new ArrayAdapter<String>(getActivity(), R.layout.message_detail);
        //messageListView.setAdapter(listAdapter);
        //messageListView.setDivider(null);
        edtMessage = (EditText) getActivity().findViewById(R.id.sendText);
        service_init();
    }

    //Function to initiate that THICC data
    private void initDataset() {
            mDataset = new String[count]; //Used for the older recyclerView
            for (int i = 0; i < count; i++) {
                mDataset[i] = "This is schedule #" + i; //Sets the dataset to strings equal to the schedule item
            }
    }

    //Function to retrieve the time selected from the Schedule Activity timePicker
    public static void putArguments(Bundle args){
        String startHour = args.getString("hour"); //The Hour
        String startMinute = args.getString("minute"); //The Minute
        startingMin.setText(startMinute); //Sets the EditText box to the retrieved value of the minute
        startingHour.setText(startHour); //Sets the EditText box to the retrieved value of the hour
    }

    //Function called when you get to this fragment from the pageViewer
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState){
        final View view = inflater.inflate(R.layout.schedule_frag, container, false); //Sets the current view

        //Declarations for schedule_frag.xml
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER; //Sets current Layout Manager type
        mSchedAdapt = new ScheduleAdapter(mDataset); //Creates a new Schedule Adapter open creation
        btnSetSched = (Button) view.findViewById(R.id.setSchedType); //Button for selecting the schedule type
        msubmitSched = (Button) view.findViewById(R.id.submitSched); //Button for sending the schedule data
        btnConnect = (Button) view.findViewById(R.id.btn_connect); //Button for connecting to BLE
        schedMenu = (Spinner) view.findViewById(R.id.spinSchedMenu); //Drop-down menu used for the types of schedule

        //Daily Sample variables gained from schedule_frag.xml
        sampleLengthDaily = (EditText) view.findViewById(R.id.sampleLengthDaily); // Daily Sample Length Input
        sampleLengthDailyText = (TextView) view.findViewById(R.id.sampleLengthDailyText); //Daily Sample Length Form Label
        flushDurationDaily = (EditText) view.findViewById(R.id.flushDurationDaily); //Flush Duration Input
        flushDurationDailyText = (TextView) view.findViewById(R.id.flushDurationDailyText); //Flush Duration Form Label
        startingHour = (EditText) view.findViewById(R.id.startingHourDaily); //Starting Hour Input
        startingHourText = (TextView) view.findViewById(R.id.startingHourDailyText); //Starting Hour Form Label
        startingMin = (EditText) view.findViewById(R.id.startingMinuteDaily); //Starting Minute Input
        startingMinText = (TextView) view.findViewById(R.id.startingMinuteDailyText); //Starting Minute Form Label
        btnNavSecondActivity = (Button) view.findViewById(R.id.timeSelectorActivity); //Time Selector Button
        timeSelectionText = (TextView) view.findViewById(R.id.timeSelectionText); //Time Selector Label

        //Periodic Sample variables from schedule_frag.xml
        periodLengthPeriodic = (EditText) view.findViewById(R.id.periodLengthPeriod); //Period Length Input
        periodLengthPeriodicText = (TextView) view.findViewById(R.id.periodLengthPeriodText); //Period Length Form Label
        sampleLengthPeriodic = (EditText) view.findViewById(R.id.sampleLengthPeriod); //Sample Length Input
        sampleLengthPeriodicText = (TextView) view.findViewById(R.id.sampleLengthPeriodText); //Sample Length Form Label
        flushDurationPeriodic = (EditText) view.findViewById(R.id.flushDurationPeriod); //Flush Duration Input
        flushDurationPeriodicText = (TextView) view.findViewById(R.id.flushDurationPeriodText); //Flush Duration Form Label

        //Makes all Daily Sample schedule items as the default form as this is
        //The first value inside the array of different types of schedule types
        btnNavSecondActivity.setVisibility(view.VISIBLE); //sets timePicker button as visible
        timeSelectionText.setVisibility(view.VISIBLE); //sets timePicker button text as visible
        startingHour.setVisibility(view.VISIBLE); //sets Starting Hour as visible
        startingHourText.setVisibility(view.VISIBLE); //sets Starting Hour Label as visible
        startingMin.setVisibility(view.VISIBLE); //sets Starting Minue as visible
        startingMinText.setVisibility(view.VISIBLE); //sets Starting Minute Label as visible
        sampleLengthDaily.setVisibility(view.VISIBLE); //sets Sample Length as visible
        sampleLengthDailyText.setVisibility(view.VISIBLE); //sets Sample Length Label as visible
        flushDurationDaily.setVisibility(view.VISIBLE); //sets Flush Duration as visible
        flushDurationDailyText.setVisibility(view.VISIBLE); //sets Flush Duration Label as visible
        sampleLengthDaily.setEnabled(true); //Enables the editing of the Sample Length
        flushDurationDaily.setEnabled(true); //Enables the editing of the Flush Duration

        //Hide all the Periodic view items as Daily Sample is the default
        periodLengthPeriodic.setVisibility(view.INVISIBLE); //sets Period Length Input to invisible
        periodLengthPeriodicText.setVisibility(view.INVISIBLE); //sets Period Length Form Label as invisible
        flushDurationPeriodic.setVisibility(view.INVISIBLE); //sets Flush Duration Input as invisible
        flushDurationPeriodicText.setVisibility(view.INVISIBLE); //sets Flush Duration Form Label as invisible
        sampleLengthPeriodic.setVisibility(view.INVISIBLE); //sets Sample Length Input as invisible
        sampleLengthPeriodicText.setVisibility(view.INVISIBLE); //sets Sample Length Form Input as invisible

        msubmitSched.setEnabled(false); //Sets the initial submit button to disabled until we recieve a BLE connection

        // This is for a secondary activity inside the schedule in case we want to add that later
        btnNavSecondActivity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                Toast.makeText(getActivity(), "Going to Schedule Activity", Toast.LENGTH_SHORT).show(); //Screen pop-up message when you enter the schedule activity
                Intent intent = new Intent(getActivity(), ScheduleActivity.class); //Begins Time Picker activity
                startActivity(intent); //Now begins Time Picker acticity

            }
        });
        //OnClickListener function for setting the schedule form
        //Based off selection of schedule from the Dropdown menu
        btnSetSched.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                temp = schedMenu.getSelectedItem().toString(); //Get the type of schedule
                if(temp.equals("Periodic")){ //If the schedule selected is Periodic, make all the Daily invisible and Periodic visible
                    Log.d(TAG, "Periodic was hit!"); //Log message
                    //Hide all the Daily View Items
                    //These have all been labeled above
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
                    //These have all been labeled above
                    periodLengthPeriodic.setVisibility(view.VISIBLE);
                    periodLengthPeriodicText.setVisibility(view.VISIBLE);
                    flushDurationPeriodic.setVisibility(view.VISIBLE);
                    flushDurationPeriodicText.setVisibility(view.VISIBLE);
                    sampleLengthPeriodic.setVisibility(view.VISIBLE);
                    sampleLengthPeriodicText.setVisibility(view.VISIBLE);
                }
                if(temp.equals("Daily")){//If the schedule selected is Daily, make all the Daily visible and Periodic invisible
                    Log.d(TAG, "Daily was hit!");
                    //Show all the daily items
                    //These have all been labeled above
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
                    //These have all been labeled above
                    periodLengthPeriodic.setVisibility(view.INVISIBLE);
                    periodLengthPeriodicText.setVisibility(view.INVISIBLE);
                    flushDurationPeriodic.setVisibility(view.INVISIBLE);
                    flushDurationPeriodicText.setVisibility(view.INVISIBLE);
                    sampleLengthPeriodic.setVisibility(view.INVISIBLE);
                    sampleLengthPeriodicText.setVisibility(view.INVISIBLE);
                }
            }
        });
        msubmitSched.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                temp = schedMenu.getSelectedItem().toString();
                if(temp.equals("Periodic")){
                    String type = "Periodic";
                    String minutes = periodLengthPeriodic.getText().toString();
                    String sampLen = sampleLengthPeriodic.getText().toString();
                    String flushDur = flushDurationPeriodic.getText().toString();
                    String message = "P" + minutes + "|S" + sampLen + "|F" + flushDur;
                    byte[] value;
                    try{
                        //send message
                        value = message.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);

                        //update log
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        listAdapter.add("["+currentDateTimeString+"] TX: " + "Periodic");
                        //messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);
                        //edtMessage.setText("");
                    }catch (UnsupportedEncodingException e){
                        e.printStackTrace();
                    }
                }
                if(temp.equals("Daily")){
                    String type = "Daily";
                    String hour = startingHour.getText().toString();
                    String minutes = startingMin.getText().toString();
                    String flushDur = flushDurationDaily.getText().toString();
                    String sampLen = sampleLengthDaily.getText().toString();
                    String message = "D" + hour + "," + minutes  + "|F" + flushDur  + "|S" + sampLen;
                    byte[] value;
                    try{
                        //send message
                        value = message.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);

                        //update log with time stamp
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        listAdapter.add("["+currentDateTimeString+"] TX: " + "Daily");
                        //messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);
                        //edtMessage.setText("");
                    } catch(UnsupportedEncodingException e){
                        e.printStackTrace();
                    }
                }
            }
        });
        btnConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                if (!mBtAdapter.isEnabled()) {
                    Log.i(TAG, "onClick - BT not enabled yet");
                    Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                    startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
                }
                else {
                    if (btnConnect.getText().equals("Connect")){

                        //Connect button pressed, open DeviceListActivity class, with popup windows that scan for devices
                        Log.d(TAG, "STARTING DEVICE LIST ACTIVITY");
                        Intent newIntent = new Intent(getActivity(), DeviceListActivity.class);
                        startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
                    } else {
                        //Disconnect button pressed
                        if (mDevice!=null)
                        {
                            mService.disconnect();
                        }
                    }
                }
            }
        });
        return view;
    }

    //when uart is connected/disconnected
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            mService = ((BluetoothService.LocalBinder) rawBinder).getService();
            Log.d(TAG, "onServiceConnected mService= " + mService);
            MainActivity xxx = (MainActivity)getActivity();
            xxx.mainService = mService;
            if (!mService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                getActivity().finish();
            }

        }

        public void onServiceDisconnected(ComponentName classname) {
            ////     mService.disconnect(mDevice);
            mService = null;
            MainActivity xxx = (MainActivity)getActivity();
            xxx.mainService = mService;
        }
    };

    private Handler mHandler = new Handler() {
        @Override

        //Handler events that received from UART service
        public void handleMessage(Message msg) {

        }
    };

    //Receives broadcasts and responds accordingly
    private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            //all broadcasts are given an intent, intent.getaction is the action specified by the broadcast
            //the action is used to determine what needs to change in the ui such as changing the connect button to disconnect
            //currently not receiving broadcasts for some reason
            String action = intent.getAction();
            Log.d(TAG, "Received action was " + action);

            final Intent mIntent = intent;
            //*********************//
            if (action.equals(BluetoothService.ACTION_GATT_CONNECTED)) {
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        Log.d(TAG, "UART_CONNECT_MSG");
                        btnConnect.setText("Disconnect");
                        msubmitSched.setEnabled(true);
                        ((TextView) getActivity().findViewById(R.id.deviceName)).setText(mDevice.getName()+ " - ready");
                        listAdapter.add("["+currentDateTimeString+"] Connected to: "+ mDevice.getName());
                        mState = UART_PROFILE_CONNECTED;
                    }
                });
            }

            //*********************//
            if (action.equals(BluetoothService.ACTION_GATT_DISCONNECTED)) {
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        Log.d(TAG, "UART_DISCONNECT_MSG");
                        btnConnect.setText("Connect");
                        msubmitSched.setEnabled(false);
                        ((TextView) getActivity().findViewById(R.id.deviceName)).setText("Not Connected");
                        listAdapter.add("["+currentDateTimeString+"] Disconnected to: "+ mDevice.getName());
                        mState = UART_PROFILE_DISCONNECTED;
                        mService.close();
                    }
                });
            }


            //*********************//
            if (action.equals(BluetoothService.ACTION_GATT_SERVICES_DISCOVERED)) {
                mService.enableTXNotification();
            }
            //*********************//
            if (action.equals(BluetoothService.ACTION_DATA_AVAILABLE)) {

                final byte[] txValue = intent.getByteArrayExtra(BluetoothService.EXTRA_DATA);
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        try {
                            String text = new String(txValue, "UTF-8");
                            String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                            listAdapter.add("["+currentDateTimeString+"] RX: "+text);
                            messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);

                        } catch (Exception e) {
                            Log.e(TAG, e.toString());
                        }
                    }
                });
            }
            //*********************//
            if (action.equals(BluetoothService.DEVICE_DOES_NOT_SUPPORT_UART)){
                showMessage("Device doesn't support UART. Disconnecting");
                mService.disconnect();
            }


        }
    };

    private void service_init() {
        Intent bindIntent = new Intent(getActivity(), BluetoothService.class);
        getActivity().bindService(bindIntent, mServiceConnection, Context.BIND_AUTO_CREATE);

        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(UARTStatusChangeReceiver, makeGattUpdateIntentFilter());
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(BluetoothService.DEVICE_DOES_NOT_SUPPORT_UART);
        return intentFilter;
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy()");

        try {
            LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(UARTStatusChangeReceiver);
        } catch (Exception ignore) {
            Log.e(TAG, ignore.toString());
        }
        getActivity().unbindService(mServiceConnection);
        mService.stopSelf();
        mService= null;
        MainActivity xxx = (MainActivity)getActivity();
        xxx.mainService = mService;

    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    /*
    @Override
    protected void onRestart() {
        super.onRestart();
        Log.d(TAG, "onRestart");
    }*/

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        if (!mBtAdapter.isEnabled()) {
            Log.i(TAG, "onResume - BT not enabled yet");
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
        }

    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {

            case REQUEST_SELECT_DEVICE:
                //When the DeviceListActivity return, with the selected device address
                if (resultCode == Activity.RESULT_OK && data != null) {
                    String deviceAddress = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
                    mDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(deviceAddress);

                    Log.d(TAG, "... onActivityResultdevice.address==" + mDevice + "mserviceValue" + mService);
                    ((TextView) getActivity().findViewById(R.id.deviceName)).setText(mDevice.getName()+ " - connecting");
                    mService.connect(deviceAddress);


                }
                break;
            case REQUEST_ENABLE_BT:
                // When the request to enable Bluetooth returns
                if (resultCode == Activity.RESULT_OK) {
                    Toast.makeText(getActivity(), "Bluetooth has turned on ", Toast.LENGTH_SHORT).show();

                } else {
                    // User did not enable Bluetooth or an error occurred
                    Log.d(TAG, "BT not enabled");
                    Toast.makeText(getActivity(), "Problem in BT Turning ON ", Toast.LENGTH_SHORT).show();
                    getActivity().finish();
                }
                break;
            default:
                Log.e(TAG, "wrong request code");
                break;
        }
    }

    private void showMessage(String msg) {
        Toast.makeText(getActivity(), msg, Toast.LENGTH_SHORT).show();

    }
}

