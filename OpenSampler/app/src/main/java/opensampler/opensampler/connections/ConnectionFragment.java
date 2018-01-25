package opensampler.opensampler.connections;

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
import android.widget.TextView;
import android.widget.Toast;

import opensampler.opensampler.MainActivity;
import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/22/2018.
 */

public class ConnectionFragment extends Fragment {
    private static final String TAG = "ConnectionFragment";
    private Button btnNavFrag2;
    private Button btnNavSecondActivity;
    private static final String KEY_LAYOUT_MANAGER = "layoutManager";
    private static final int count = 10;
    private static final int span = 2;


    private enum LayoutManagerType {
        GRID_LAYOUT_MANAGER,
        LINEAR_LAYOUT_MANAGER
    }

    protected LayoutManagerType mCurrentLayoutManagerType = null;
    protected RecyclerView mRecyclerView;
    protected ConnectionAdapter mConnAdapt;
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
                mDataset[i] = "This is connection #" + i;
            }
    }
	
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.connect_frag, container, false);
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;

        mLayoutManager = new LinearLayoutManager(getActivity());
        mCurrentLayoutManagerType = LayoutManagerType.LINEAR_LAYOUT_MANAGER;
        mRecyclerView = (RecyclerView) view.findViewById(R.id.connRecView);
        mLayoutManager = new LinearLayoutManager(getActivity());

        mConnAdapt = new ConnectionAdapter(mDataset);
        mRecyclerView.setAdapter(mConnAdapt);

        btnNavFrag2 = (Button) view.findViewById(R.id.btnNavFrag2);
        btnNavSecondActivity = (Button) view.findViewById(R.id.btnNavSecondActivity);
        Log.d(TAG, "onCreateView: Started.");

        mRecyclerView.setLayoutManager(mLayoutManager);
        mRecyclerView.scrollToPosition(0);
        
        btnNavFrag2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(getActivity(), "Going to Samples", Toast.LENGTH_SHORT).show();
                ((MainActivity) getActivity()).setViewPager(1);
            }
        });
        btnNavSecondActivity.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(getActivity(), "Going to Connection Activity", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(getActivity(), ConnectionActivity.class);
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
