package cn.itcast.h264test;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Window;
import android.view.animation.AlphaAnimation;
import android.widget.ImageView;

public class WelCome extends Activity {	
	private ImageView tv;
		
	@Override
	protected void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.welco);	
		final AlphaAnimation a=new AlphaAnimation(1,(float)0.5);
		a.setDuration(2000);
		a.setFillAfter(true);		
		tv=(ImageView)findViewById(R.id.tv);		
		tv.setAnimation(a);			
		Start();		
	}		
	
	public void Start(){
    	new Thread(){
    		public void run(){
    			try {
					Thread.sleep(6000);					
				} catch (InterruptedException e) {					
					e.printStackTrace();
				}
    			startActivity(new Intent(WelCome.this,LjMon.class));    			
    			overridePendingTransition(R.anim.fade, R.anim.hold);    			   			
    		}
    	}.start();
    }
}