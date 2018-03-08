package opensampler.opensampler;

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
