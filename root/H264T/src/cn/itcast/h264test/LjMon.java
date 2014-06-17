package cn.itcast.h264test;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.ImageView;

public class LjMon extends Activity{
	private ImageView buttonSet,sendVideo; 
	private Context context;
		
	@Override
	protected void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);		
		setContentView(R.layout.maie);						
		buttonSet=(ImageView)findViewById(R.id.button_settings);		
		sendVideo=(ImageView)findViewById(R.id.button_about);		
		context = this.getApplicationContext();				
		buttonSet.setOnClickListener(new OnClickListener() { 
			public void onClick(View v) {				
	            startActivityForResult(new Intent(context,Option.class), 0);	            
	            overridePendingTransition(R.anim.fade, R.anim.hold);
			} 
		}); 
		
		sendVideo.setOnClickListener(new OnClickListener() { 
			public void onClick(View v) { 				
				startActivity(new Intent(LjMon.this, 
						MainActivity.class));
				overridePendingTransition(R.anim.fade, R.anim.hold);
			} 
		}); 	
	}	
}
	