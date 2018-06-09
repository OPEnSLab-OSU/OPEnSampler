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
import android.widget.Toast;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class PuppetFragment extends Fragment implements PuppetAdapter.OnSearchResultClickListener {
    private static final String TAG = "SampleFragment";
    private Button btnNavSecondActivity;
    private static final String KEY_LAYOUT_MANAGER = "layoutManager";
    private static final int count = 10;
    private static final int span = 2;

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
        btnNavSecondActivity = (Button) view.findViewById(R.id.timeSelectorActivity);
        mLayoutManager = new LinearLayoutManager(getActivity());

        mSamAdapt = new PuppetAdapter(this, mDataset);
        Log.d(TAG, "onCreateView: Started.");

        btnNavSecondActivity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(getActivity(), "Going to Samples Activity", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(getActivity(), PuppetActivity.class);
                startActivity(intent);
            }

        });
        return view;
    }

};

