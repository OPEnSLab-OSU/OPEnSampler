package opensampler.opensampler.puppet;

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

public class PuppetAdapter extends RecyclerView.Adapter<PuppetAdapter.ViewHolder>{ //Puppet Adapter class
    private static final String TAG = "PuppetAdapter"; //Simple Tag
    private OnSearchResultClickListener mSearchResultClickListener; //Click Listener for when this was sample fragment
    private String[] mDataSet; //That THICC data

    public PuppetAdapter(OnSearchResultClickListener clickListener, String[] dataSet){ //
        mSearchResultClickListener = clickListener; //Click Listener for the samples
        mDataSet = dataSet; //Setting that THICC data
    }

    public interface OnSearchResultClickListener{
        void onSampleElementClick(); //Old function
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup viewGroup, int viewType) {
        View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.puppet_item, viewGroup, false); //Sets the puppet items, but no longer needed the recyclerView
        return new ViewHolder(view); //Returns the View for the recyclerView
    }

    @Override
    public void onBindViewHolder(ViewHolder viewHolder, final int position) { //Only here because it is required to be
        Log.d(TAG, "Element " + position + " set."); //Simple Tag message
        viewHolder.getTextView().setText(mDataSet[position]); //Sets things for when we were using recyclerView
    }

    @Override
    public int getItemCount() {
        return mDataSet.length;
    } //Simple get function for data

    //This is no longer needed, but left in case someone wants to eventually update this
    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener{
        private final TextView textView;
        //ViewHolder for the recyclerView
        public ViewHolder(View view){
            super(view);
            textView = (TextView) view.findViewById(R.id.tv_sample_text);
            view.setOnClickListener(this);
        }
        public TextView getTextView(){
            return textView;
        }

        @Override
        public void onClick(View v) {
            Log.d(TAG, "View Clicked");
            mSearchResultClickListener.onSampleElementClick();
        }
    }
}
