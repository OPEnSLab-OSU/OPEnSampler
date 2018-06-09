package opensampler.opensampler.schedule;

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

public class ScheduleAdapter extends RecyclerView.Adapter<ScheduleAdapter.ViewHolder>{
    private static final String TAG = "ScheduleAdapter"; //Tag for this Adapter
    private String[] mDataSet;  //That THICC Data
    //Constructor for the adapter simply setting
    public ScheduleAdapter(String[] dataSet){
        mDataSet = dataSet; //sets the THICC Data to what is generated in the form
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup viewGroup, int viewType) {
        View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.schedule_item, viewGroup, false); //sets up the view variable for the recyclerView
        return new ViewHolder(view); //return the ViewHolder with the desired .xml file
    }

    @Override
    public void onBindViewHolder(ViewHolder viewHolder, final int position) {
        Log.d(TAG, "Element " + position + " set."); //Used for recycler view
        viewHolder.getTextView().setText(mDataSet[position]); //
    }

    @Override
    public int getItemCount() {
        return mDataSet.length; //Get function for length of data
    }

    public class ViewHolder extends RecyclerView.ViewHolder{
        private final TextView textView; //

        public ViewHolder(View view){
            super(view); //
            view.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.d(TAG, "Element " + getAdapterPosition() + " clicked."); //Simple log message
                }
            });
            textView = (TextView) view.findViewById(R.id.tv_schedule_text); //
        }
        public TextView getTextView(){
            return textView; //
        }
    }
}