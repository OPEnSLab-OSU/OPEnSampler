package opensampler.opensampler.phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.support.v4.app.Fragment;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.UnsupportedEncodingException;
import java.text.DateFormat;
import java.util.Date;

import opensampler.opensampler.BluetoothService;
import opensampler.opensampler.MainActivity;
import opensampler.opensampler.R;

import static opensampler.opensampler.DeviceListActivity.TAG;

/**
 * Created by Godtop on 2/21/2018.
 */

public class PhoneFragment extends Fragment {

    private String mParam1;
    private String temp;
    private static final int count = 10;
    protected String[] mDataset;

    private Spinner phoneMenu;
    private Spinner phoneNumbers;

    private Button submitMenu;
    private Button selectPhoneUpdate;
    private Button submitPhoneUpdate;
    private Button submitPhoneAdd;
    private Button submitPhoneDelete;

    private EditText phoneAdd;
    private EditText phoneUpdate;

    private TextView phoneNumberMsg;
    private TextView newNumber;
    private TextView addPhoneNumber;

    private BluetoothService mService = null;

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

    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState){
        final View view = inflater.inflate(R.layout.phone_frag, container, false);

        //Spinners
            phoneMenu = (Spinner) view.findViewById(R.id.phoneCommands);
            phoneNumbers = (Spinner) view.findViewById(R.id.phoneNumbers);
        //Buttons
            submitMenu = (Button) view.findViewById(R.id.submitPhoneMenu);
            submitPhoneAdd = (Button) view.findViewById(R.id.addPhone);
            submitPhoneDelete = (Button) view.findViewById(R.id.removePhone);
            submitPhoneUpdate = (Button) view.findViewById(R.id.updatePhone);
        //EditText
            phoneAdd = (EditText) view.findViewById(R.id.addNumber);
            phoneUpdate = (EditText) view.findViewById(R.id.updateNumber) ;
        //TextViews
            phoneNumberMsg = (TextView) view.findViewById(R.id.phoneNumberMsg);
            newNumber = (TextView) view.findViewById(R.id.numberToChange);
            addPhoneNumber = (TextView) view.findViewById(R.id.addMessage);

        submitPhoneAdd.setEnabled(false);
        submitPhoneDelete.setEnabled(false);
        submitPhoneUpdate.setEnabled(false);

        phoneNumbers.setVisibility(view.INVISIBLE);
        addPhoneNumber.setVisibility(view.INVISIBLE);
        phoneNumberMsg.setVisibility(view.INVISIBLE);
        phoneAdd.setVisibility(view.INVISIBLE);
        submitPhoneDelete.setVisibility(view.INVISIBLE);
        submitPhoneAdd.setVisibility(view.INVISIBLE);
        submitPhoneUpdate.setVisibility(view.INVISIBLE);
        newNumber.setVisibility(view.INVISIBLE);
        phoneUpdate.setVisibility(view.INVISIBLE);

        submitMenu.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                temp = phoneMenu.getSelectedItem().toString();
                if(temp.equals("Add")){
                    //add
                    phoneAdd.setVisibility(view.VISIBLE);
                    submitPhoneAdd.setVisibility(view.VISIBLE);
                    addPhoneNumber.setVisibility(view.VISIBLE);
                    //update
                    submitPhoneUpdate.setVisibility(view.INVISIBLE);
                    newNumber.setVisibility(view.INVISIBLE);
                    phoneUpdate.setVisibility(view.INVISIBLE);
                    //remove
                    phoneNumbers.setVisibility(view.INVISIBLE);
                    phoneNumberMsg.setVisibility(view.INVISIBLE);
                    submitPhoneDelete.setVisibility(view.INVISIBLE);

                }
                if(temp.equals("Remove")){
                    //add
                    phoneAdd.setVisibility(view.INVISIBLE);
                    submitPhoneAdd.setVisibility(view.INVISIBLE);
                    addPhoneNumber.setVisibility(view.INVISIBLE);
                    //update
                    submitPhoneUpdate.setVisibility(view.INVISIBLE);
                    newNumber.setVisibility(view.INVISIBLE);
                    phoneUpdate.setVisibility(view.INVISIBLE);
                    //remove
                    phoneNumbers.setVisibility(view.VISIBLE);
                    phoneNumberMsg.setVisibility(view.VISIBLE);
                    submitPhoneDelete.setVisibility(view.VISIBLE);
                }
                if(temp.equals("Update")){
                    //add
                    phoneAdd.setVisibility(view.INVISIBLE);
                    submitPhoneAdd.setVisibility(view.INVISIBLE);
                    addPhoneNumber.setVisibility(view.INVISIBLE);
                    //update
                    phoneNumbers.setVisibility(view.VISIBLE);
                    phoneNumberMsg.setVisibility(view.VISIBLE);
                    submitPhoneUpdate.setVisibility(view.VISIBLE);
                    newNumber.setVisibility(view.VISIBLE);
                    phoneUpdate.setVisibility(view.VISIBLE);
                    //remove
                    submitPhoneDelete.setVisibility(view.INVISIBLE);
                }

            }
        });

        submitPhoneAdd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                String pnumber = phoneAdd.getText().toString();
                String message = "W" + pnumber;
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

        submitPhoneDelete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){

            }
        });

        submitPhoneUpdate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                MainActivity xxx = (MainActivity)getActivity();
                mService = xxx.mainService;
                 String pnumber = phoneUpdate.getText().toString();
                 String message = "U" + pnumber;
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

    private final BroadcastReceiver UARTStatusChangeReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            //all broadcasts are given an intent, intent.getaction is the action specified by the broadcast
            //the action is used to determine what needs to change in the ui such as changing the connect button to disconnect
            //currently not receiving broadcasts for some reason
            String action = intent.getAction();

            final Intent mIntent = intent;
            //*********************//
            if (action.equals(BluetoothService.ACTION_GATT_CONNECTED)) {
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        //enable add phone button
                        submitPhoneAdd.setEnabled(true);
                        submitPhoneDelete.setEnabled(true);
                        submitPhoneUpdate.setEnabled(true);
                        MainActivity xxx = (MainActivity)getActivity();
                        mService = xxx.mainService;
                    }
                });
            }

            //*********************//
            if (action.equals(BluetoothService.ACTION_GATT_DISCONNECTED)) {
                getActivity().runOnUiThread(new Runnable() {
                    public void run() {
                        submitPhoneAdd.setEnabled(false);
                        submitPhoneDelete.setEnabled(false);
                        submitPhoneUpdate.setEnabled(false);
                        MainActivity xxx = (MainActivity)getActivity();
                        mService = xxx.mainService;
                    }
                });
            }
        }
    };
}
