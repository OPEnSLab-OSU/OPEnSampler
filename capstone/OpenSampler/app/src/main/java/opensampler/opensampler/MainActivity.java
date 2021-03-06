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
import android.widget.ImageButton;
import android.widget.TextView;

import opensampler.opensampler.phone.PhoneFragment;
import opensampler.opensampler.puppet.PuppetFragment;
import opensampler.opensampler.schedule.ScheduleFragment;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private SectionStatePagerAdapter mSectionsStatePagerAdapter;
    private ViewPager mViewPager;
    private Button mPuppetButt;
    private Button mSchedButt;
    private Button mPhoneButt;
    private ImageButton mInfoButt;
    private TextView mScreenName;
	public BluetoothService mainService = null;
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.d(TAG, "onCreate: Started.");
        mSectionsStatePagerAdapter = new SectionStatePagerAdapter(getSupportFragmentManager());
        mViewPager = (ViewPager) findViewById(R.id.container);
        setupViewPager(mViewPager);

        mPuppetButt = (Button) findViewById(R.id.puppet);
        mSchedButt = (Button) findViewById(R.id.sched);
        mPhoneButt = (Button) findViewById(R.id.phone);
        mScreenName = (TextView) findViewById(R.id.screenName);

        mPuppetButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                mScreenName.setText("Puppet Commands");
                setViewPager(1);
            }
        });

        mSchedButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                mScreenName.setText("Scheduler");
                setViewPager(0);
            }
        });

        mPhoneButt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                mScreenName.setText("Phone Commands");
                setViewPager(2);
            }
        });

        mViewPager.addOnPageChangeListener(new ViewPager.OnPageChangeListener() {
            @Override
            public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

            }

            @Override
            public void onPageSelected(int position) {
                if(position == 0){
                    mScreenName.setText("Scheduler");
                }
                if(position == 1){
                    mScreenName.setText("Puppet Commands");
                }
                if(position == 2){
                    mScreenName.setText("Phone Commands");
                }
            }

            @Override
            public void onPageScrollStateChanged(int state) {

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
        adapter.addFragment(new ScheduleFragment(), "ScheduleFragment");
        adapter.addFragment(new PuppetFragment(), "PuppetFragment");
        adapter.addFragment(new PhoneFragment(), "PhoneFragment");
        viewPager.setAdapter(adapter);
    }

    public void setViewPager(int fragmentNumber){
        mViewPager.setCurrentItem(fragmentNumber);
    }
}
