package opensampler.opensampler;

import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import opensampler.opensampler.connections.ConnectionFragment;
import opensampler.opensampler.samples.SamplesFragment;
import opensampler.opensampler.schedule.ScheduleFragment;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private SectionStatePagerAdapter mSectionsStatePagerAdapter;
    private ViewPager mViewPager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.d(TAG, "onCreate: Started.");
        mSectionsStatePagerAdapter = new SectionStatePagerAdapter(getSupportFragmentManager());
        mViewPager = (ViewPager) findViewById(R.id.container);
        setupViewPager(mViewPager);
    }

    private void setupViewPager(ViewPager viewPager){
        SectionStatePagerAdapter adapter = new SectionStatePagerAdapter(getSupportFragmentManager());
        adapter.addFragment(new ConnectionFragment(), "ConnectionFragment");
        adapter.addFragment(new SamplesFragment(), "SamplesFragment");
        adapter.addFragment(new ScheduleFragment(), "ScheduleFragment");
        viewPager.setAdapter(adapter);
    }

    public void setViewPager(int fragmentNumber){
        mViewPager.setCurrentItem(fragmentNumber);
    }
}
