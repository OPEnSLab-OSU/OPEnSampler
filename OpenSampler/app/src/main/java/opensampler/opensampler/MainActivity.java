package opensampler.opensampler;

import android.Manifest;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import opensampler.opensampler.connections.ConnectionFragment;
import opensampler.opensampler.phone.PhoneFragment;
import opensampler.opensampler.samples.SamplesFragment;
import opensampler.opensampler.schedule.ScheduleFragment;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private SectionStatePagerAdapter mSectionsStatePagerAdapter;
    private ViewPager mViewPager;
    private Button mConnButt;
    private Button mSampButt;
    private Button mSchedButt;
    private Button mPhoneButt;
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 1;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.d(TAG, "onCreate: Started.");
        mSectionsStatePagerAdapter = new SectionStatePagerAdapter(getSupportFragmentManager());
        mViewPager = (ViewPager) findViewById(R.id.container);
        setupViewPager(mViewPager);

        mConnButt = (Button) findViewById(R.id.conn);
        mSampButt = (Button) findViewById(R.id.samp);
        mSchedButt = (Button) findViewById(R.id.sched);
        mPhoneButt = (Button) findViewById(R.id.phone);

        mConnButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                setViewPager(0);            }
        });
        mSampButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                setViewPager(1);
            }
        });
        mSchedButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                setViewPager(2);
            }
        });
        mPhoneButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                setViewPager(3);
            }
        });

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M){
            if(this.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED){
                final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("App requires location access");
                builder.setMessage("Please grant location access to this app");
                builder.setPositiveButton(android.R.string.ok, null);
                builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    @RequiresApi(api = Build.VERSION_CODES.M)
                    @Override
                    public void onDismiss(DialogInterface dialogInterface) {
                        requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSION_REQUEST_COARSE_LOCATION);
                    }
                });
                builder.show();
            }
        }

    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[],
                                           int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_REQUEST_COARSE_LOCATION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Log.d(TAG, "coarse location permission granted");
                } else {
                    final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Functionality limited");
                    builder.setMessage("Since location access has not been granted, this app will not be able to discover beacons when in the background.");
                    builder.setPositiveButton(android.R.string.ok, null);
                    builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                        }
                    });
                    builder.show();
                }
                return;
            }
        }
    }

    private void setupViewPager(ViewPager viewPager){
        SectionStatePagerAdapter adapter = new SectionStatePagerAdapter(getSupportFragmentManager());
        adapter.addFragment(new ConnectionFragment(), "ConnectionFragment");
        adapter.addFragment(new SamplesFragment(), "SamplesFragment");
        adapter.addFragment(new ScheduleFragment(), "ScheduleFragment");
        adapter.addFragment(new PhoneFragment(), "PhoneFragment");
        viewPager.setAdapter(adapter);
    }

    public void setViewPager(int fragmentNumber){
        mViewPager.setCurrentItem(fragmentNumber);
    }
}
