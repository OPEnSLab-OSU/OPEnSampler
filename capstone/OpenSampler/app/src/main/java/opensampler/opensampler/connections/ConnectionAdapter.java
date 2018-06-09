package opensampler.opensampler.connections;

import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import opensampler.opensampler.R;

/**
 * Created by Godtop on 1/24/2018.
 */

public class ConnectionAdapter extends RecyclerView.Adapter<ConnectionAdapter.ViewHolder>{
    private static final String TAG = "ConnectionAdapter";
    private OnSearchResultClickListener mSearchResultClickListener;
    private String[] mDataSet;

    public ConnectionAdapter(OnSearchResultClickListener clickListener, String[] dataSet){
        mSearchResultClickListener = clickListener;
        mDataSet = dataSet;
    }

    public interface OnSearchResultClickListener{
        void onConnectionElementClick();
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup viewGroup, int viewType) {
        View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.connect_item, viewGroup, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(ViewHolder viewHolder, final int position) {
        Log.d(TAG, "Element " + position + " set.");
        viewHolder.getTextView().setText(mDataSet[position]);
    }

    @Override
    public int getItemCount() {
        return mDataSet.length;
    }

    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener
    {
        private final TextView textView;

        public ViewHolder(View view){
            super(view);
            textView = (TextView) view.findViewById(R.id.tv_connect_text);
            view.setOnClickListener(this);
        }
        public TextView getTextView(){
            return textView;
        }

        @Override
        public void onClick(View v) {
            Log.d(TAG, "View Clicked");
            mSearchResultClickListener.onConnectionElementClick();
        }
    }
}
