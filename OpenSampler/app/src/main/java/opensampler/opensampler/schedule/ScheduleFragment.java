package opensampler.opensampler.schedule;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
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
import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ScheduleFragment extends Fragment {
    private static final String TAG = "ScheduleFragment";
    private String temp = "";
    private Button btnNavFrag2;
    private Button btnSetSched;
    private Button msubmitSched;
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
    private Button btnConnectDisconnect,btnSend;
    private EditText edtMessage;
    TextView mRemoteRssiVal;
    RadioGroup mRg;

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
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if(getArguments() != null){
            mParam1 = getArguments().getString("params");
        }
        initDataset();

        if(mBtAdapter == null){
            Toast.makeText(getActivity(), "Bluetooth is unavailable", Toast.LENGTH_SHORT).show();
            return;
        }
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

    //when uart is connected/disconnected
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder rawBinder) {
            mService = ((BluetoothService.LocalBinder) rawBinder).getService();
            Log.d(TAG, "onServiceConnected mService= " + mService);
            if (!mService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                getActivity().finish();
            }

        }

        public void onServiceDisconnected(ComponentName classname) {
            ////     mService.disconnect(mDevice);
            mService = null;
        }
    };

    private Handler mHandler = new Handler() {
        @Override

        //Handler events that received from UART service
        public void handleMessage(Message msg) {

        }
    };

    /*private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            final Intent mIntent = intent;

            if (action.equals(BluetoothService.ACTION_GATT_CONNECTED)) {
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        Log.d(TAG, "UART_CONNECT_MSG");
                        btnConnectDisconnect.setText("Disconnect");
                        edtMessage.setEnabled(true);
                        btnSend.setEnabled(true);
                        //((TextView) getActivity().findViewById(R.id.deviceName)).setText(mDevice.getName()+ " - ready");
                        listAdapter.add("["+currentDateTimeString+"] Connected to: "+ mDevice.getName());
                        messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);
                        mState = UART_PROFILE_CONNECTED;
                    }
                });
            }


            if (action.equals(BluetoothService.ACTION_GATT_DISCONNECTED)) {
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        Log.d(TAG, "UART_DISCONNECT_MSG");
                        btnConnectDisconnect.setText("Connect");
                        edtMessage.setEnabled(false);
                        btnSend.setEnabled(false);
                        //((TextView) getActivity().findViewById(R.id.deviceName)).setText("Not Connected");
                        listAdapter.add("["+currentDateTimeString+"] Disconnected to: "+ mDevice.getName());
                        mState = UART_PROFILE_DISCONNECTED;
                        mService.close();
                        //setUiState();

                    }
                });
            }



            if (action.equals(BluetoothService.ACTION_GATT_SERVICES_DISCOVERED)) {
                mService.enableTXNotification();
            }
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
            if (action.equals(BluetoothService.DEVICE_DOES_NOT_SUPPORT_UART)){
                //showMessage("Device doesn't support UART. Disconnecting");
                mService.disconnect();
            }


        }
    };*/

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
        msubmitSched.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                temp = schedMenu.getSelectedItem().toString();
                if(temp.equals("Periodic")){
                    String type = "Periodic";
                    String minutes = periodLengthPeriodic.getText().toString();
                    String sampLen = sampleLengthPeriodic.getText().toString();
                    String flushDur = flushDurationPeriodic.getText().toString();
                    String days = numDays.getText().toString();
                    byte[] value;
                    try{
                        //send type
                        value = type.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send minutes
                        value = minutes.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send samplen
                        value = sampLen.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send flushDur
                        value = flushDur.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send days
                        value = days.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);

                        //update log
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        listAdapter.add("["+currentDateTimeString+"] TX: " + type);
                        messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);
                        edtMessage.setText("");
                    }catch (UnsupportedEncodingException e){
                        e.printStackTrace();
                    }
                }
                if(temp.equals("Daily")){
                    String type = "Daily";
                    String hour = startingHour.getText().toString();
                    String minutes = startingMin.getText().toString();
                    String sampLen = sampleLengthDaily.getText().toString();
                    String flushDur = flushDurationDaily.getText().toString();
                    String days = numDays.getText().toString();
                    byte[] value;
                    try{
                        //send type
                        value = type.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send hour to service
                        value = hour.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send minutes
                        value = minutes.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send sampLen
                        value = sampLen.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send flushdur
                        value = flushDur.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);
                        //send days
                        value = days.getBytes("UTF-8");
                        mService.writeRXCharacteristic(value);

                        //update log with time stamp
                        String currentDateTimeString = DateFormat.getTimeInstance().format(new Date());
                        listAdapter.add("["+currentDateTimeString+"] TX: " + type);
                        messageListView.smoothScrollToPosition(listAdapter.getCount() - 1);
                        edtMessage.setText("");
                    } catch(UnsupportedEncodingException e){
                        e.printStackTrace();
                    }
                }
            }
        });
        return view;
    }
    /*
    private void service_init() {
        Intent bindIntent = new Intent(getActivity(), BluetoothService.class);
        bindService(bindIntent, mServiceConnection, Context.BIND_AUTO_CREATE);

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
        unbindService(mServiceConnection);
        mService.stopSelf();
        mService= null;

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

    protected void onRestart() {
        super.getActivity().onRestart();
        Log.d(TAG, "onRestart");
    }

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
                //When the  return, with the selected device address
                if (resultCode == Activity.RESULT_OK && data != null) {
                    String deviceAddress = data.getStringExtra(BluetoothDevice.EXTRA_DEVICE);
                    mDevice = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(deviceAddress);

                    Log.d(TAG, "... onActivityResultdevice.address==" + mDevice + "mserviceValue" + mService);
                    ((TextView) findViewById(R.id.deviceName)).setText(mDevice.getName()+ " - connecting");
                    mService.connect(deviceAddress);


                }
                break;
            case REQUEST_ENABLE_BT:
                // When the request to enable Bluetooth returns
                if (resultCode == Activity.RESULT_OK) {
                    Toast.makeText(this, "Bluetooth has turned on ", Toast.LENGTH_SHORT).show();

                } else {
                    // User did not enable Bluetooth or an error occurred
                    Log.d(TAG, "BT not enabled");
                    Toast.makeText(this, "Problem in BT Turning ON ", Toast.LENGTH_SHORT).show();
                    finish();
                }
                break;
            default:
                Log.e(TAG, "wrong request code");
                break;
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {

    }


    private void showMessage(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();

    }

    @Override
    public void onBackPressed() {
        if (mState == UART_PROFILE_CONNECTED) {
            Intent startMain = new Intent(Intent.ACTION_MAIN);
            startMain.addCategory(Intent.CATEGORY_HOME);
            startMain.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(startMain);
            showMessage("nRFUART's running in background.\n             Disconnect to exit");
        }
        else {
            new AlertDialog.Builder(this)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.popup_title)
                    .setMessage(R.string.popup_message)
                    .setPositiveButton(R.string.popup_yes, new DialogInterface.OnClickListener()
                    {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    })
                    .setNegativeButton(R.string.popup_no, null)
                    .show();
        }
    }*/
}

