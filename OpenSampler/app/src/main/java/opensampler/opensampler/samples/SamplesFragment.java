package opensampler.opensampler.samples;
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
import opensampler.opensampler.MainActivity;
import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class SamplesFragment extends Fragment {
    private static final String TAG = "SampleFragment";
    private Button btnNavFrag1;
    private Button btnNavFrag3;
    private Button btnNavSecondActivity;
    private static final String KEY_LAYOUT_MANAGER = "layoutManager";
    private static final int count = 10;
    private static final int span = 2;

    private enum LayoutManagerType {
        GRID_LAYOUT_MANAGER,
        LINEAR_LAYOUT_MANAGER
    }

    protected LayoutManagerType mCurrentLayoutManagerType;
    protected RecyclerView mRecyclerView;
    protected SampleAdapter mSamAdapt;
    protected RecyclerView.LayoutManager mLayoutManager;
    protected String[] mDataset;

    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        initDataset();
    }

    private void initDataset() {
        mDataset = new String[count];
        for (int i = 0; i < count; i++) {
            mDataset[i] = "This is sample #" + i;
        }
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState){
        View view = inflater.inflate(R.layout.sample_frag, container, false);
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;

        mLayoutManager = new LinearLayoutManager(getActivity());
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mRecyclerView = (RecyclerView) view.findViewById(R.id.samRecView);
        mLayoutManager = new LinearLayoutManager(getActivity());

        mSamAdapt = new SampleAdapter(mDataset);
        mRecyclerView.setAdapter(mSamAdapt);
        mRecyclerView.setLayoutManager(mLayoutManager);
        mRecyclerView.scrollToPosition(0);

        btnNavFrag1 = (Button) view.findViewById(R.id.btnNavFrag1);
        btnNavFrag3 = (Button) view.findViewById(R.id.btnNavFrag3);
        btnNavSecondActivity = (Button) view.findViewById(R.id.btnNavSecondActivity);
        Log.d(TAG, "onCreateView: Started.");
        btnNavFrag1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                Toast.makeText(getActivity(), "Going to Connections", Toast.LENGTH_SHORT).show();
                ((MainActivity)getActivity()).setViewPager(0);
            }

        });
        btnNavFrag3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                Toast.makeText(getActivity(), "Going to Schedule", Toast.LENGTH_SHORT).show();
                ((MainActivity)getActivity()).setViewPager(2);            }

        });
        btnNavSecondActivity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view){
                Toast.makeText(getActivity(), "Going to Samples Activity", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(getActivity(), SampleActivity.class);
                startActivity(intent);
            }

        });
        return view;
    }

    public void setRecyclerViewLayoutManager(LayoutManagerType layoutManagerType) {
        int scrollPosition = 0;
        // If a layout manager has already been set, get current scroll position.
        if (mRecyclerView.getLayoutManager() != null) {
            scrollPosition = ((LinearLayoutManager) mRecyclerView.getLayoutManager())
                    .findFirstCompletelyVisibleItemPosition();
        }

        switch (layoutManagerType) {
            case GRID_LAYOUT_MANAGER:
                mLayoutManager = new GridLayoutManager(getActivity(), span);
                mCurrentLayoutManagerType = LayoutManagerType.GRID_LAYOUT_MANAGER;
                break;
            case LINEAR_LAYOUT_MANAGER:
                mLayoutManager = new LinearLayoutManager(getActivity());
                mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
                break;
            default:
                mLayoutManager = new LinearLayoutManager(getActivity());
                mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        }

        mRecyclerView.setLayoutManager(mLayoutManager);
        mRecyclerView.scrollToPosition(scrollPosition);
    }
}
