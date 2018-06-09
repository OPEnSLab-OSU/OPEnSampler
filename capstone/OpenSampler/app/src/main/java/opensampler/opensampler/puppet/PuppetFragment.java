package opensampler.opensampler.puppet;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import java.io.UnsupportedEncodingException;

import opensampler.opensampler.BluetoothService;
import opensampler.opensampler.MainActivity;
import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class PuppetFragment extends Fragment implements PuppetAdapter.OnSearchResultClickListener {
    private static final String TAG = "SampleFragment";
    private static final String KEY_LAYOUT_MANAGER = "layoutManager";
    private static final int count = 10;
    private static final int span = 2;

    //variables for buttons and spinner menu that need to be read from
    private Button motorOn;// motor on button
    private Button motorOff;//  motor for button
    private Button motorRevers;//   motor reverse button
    private Spinner valve;//    spinner for valve number
    private Button valveOpen;// valve open button
    private Button valveClose;//    valve close button

    //bluetooth service that allows packets to be sent
    private BluetoothService mService = null;

    private enum LayoutManagerType {
        GRID_LAYOUT_MANAGER,
        LINEAR_LAYOUT_MANAGER
    }

    protected LayoutManagerType mCurrentLayoutManagerType;
    protected PuppetAdapter mSamAdapt;
    protected RecyclerView.LayoutManager mLayoutManager;
    protected String[] mDataset;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initDataset();
    }

    private void initDataset() {
        mDataset = new String[count];
        for (int i = 0; i < count; i++) {
            mDataset[i] = "This is sample #" + i;
        }
    }

    public void onSampleElementClick() {
        Log.d(TAG, "Before Intent");
        Intent intent = new Intent(getActivity(), PuppetDetailActivity.class);
        Log.d(TAG, "Before StartActivity");
        startActivity(intent);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.puppet_frag, container, false);
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mLayoutManager = new LinearLayoutManager(getActivity());

        //Set all buttons and spinners so their values can be used
        motorOn = (Button) view.findViewById(R.id.motorOn);
        motorOff = (Button) view.findViewById(R.id.motorOff);
        motorRevers = (Button) view.findViewById(R.id.motorReverse);
        valveOpen = (Button) view.findViewById(R.id.valveOpen);
        valveClose = (Button) view.findViewById(R.id.valveClose);
        valve = (Spinner) view.findViewById(R.id.valveNumbas);

        mSamAdapt = new PuppetAdapter(this, mDataset);
        Log.d(TAG, "onCreateView: Started.");

        //listener for motor on button
        motorOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                //first update the bluetooth service variable with the value in main activity to make sure we can connect
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                //next craft the message, in this case M is the prefix for motor and 1 specifies to turn it on
                String message = "M1";
                byte[] value;
                try{
                    //send message
                    value = message.getBytes("UTF-8");
                    mService.writeRXCharacteristic(value);
                }catch (UnsupportedEncodingException e){
                    e.printStackTrace();
                }
            }
        });
        // listener for motor off button
        motorOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                //first update the bluetooth service variable with the value in main activity to make sure we can connect
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                //next craft the message, in this case M is the prefix for motor and 0 specifies to turn it off
                String message = "M0";
                byte[] value;
                try{
                    //send message
                    value = message.getBytes("UTF-8");
                    mService.writeRXCharacteristic(value);
                }catch (UnsupportedEncodingException e){
                    e.printStackTrace();
                }
            }
        });
        //listener for reverse motor button
        motorRevers.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                //first update the bluetooth service variable with the value in main activity to make sure we can connect
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                //next craft the message, in this case M is the prefix for motor and -1 specifies to reverse it
                String message = "M-1";
                byte[] value;
                try{
                    //send message
                    value = message.getBytes("UTF-8");
                    mService.writeRXCharacteristic(value);
                }catch (UnsupportedEncodingException e){
                    e.printStackTrace();
                }
            }
        });
        //listener for open valve button
        valveOpen.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                //first get the valve number and update the bluetooth service variable with the value in main activity to make sure we can connect
                String temp = valve.getSelectedItem().toString();
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                //next craft the message, in this case U is the prefix for valve and 1 specifies to open the valve pointed to by temp
                String message = "U" + temp + ", 1";
                byte[] value;
                try{
                    //send message
                    value = message.getBytes("UTF-8");
                    mService.writeRXCharacteristic(value);
                }catch (UnsupportedEncodingException e){
                    e.printStackTrace();
                }
            }
        });
        //listener for close valve button
        valveClose.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                //first get the valve number and update the bluetooth service variable with the value in main activity to make sure we can connect
                String temp = valve.getSelectedItem().toString();
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                //next craft the message, in this case U is the prefix for valve and 0 specifies to close the valve pointed to by temp
                String message = "U" + temp + ", 0";
                byte[] value;
                try{
                    //send message
                    value = message.getBytes("UTF-8");
                    mService.writeRXCharacteristic(value);
                }catch (UnsupportedEncodingException e){
                    e.printStackTrace();
                }
            }
        });

        return view;
    }

};

