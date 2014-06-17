package cn.itcast.h264test;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException; 

import android.hardware.Camera;
import android.media.MediaRecorder;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.util.Log;
import android.view.Menu;
import android.view.SurfaceHolder;
import android.view.Window;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
 
public class MainActivity extends Activity implements Callback,Runnable,OnClickListener,
							OnSharedPreferenceChangeListener{   
	
	private static final String TAG = "VideoCamera";	
	private final String mediaShare="media";
	private boolean mMediaRecorderRecording=false;	
	private byte[] SPS,PPS; 
	private byte[] SAP=new byte[30];
    private byte[] sendbuf=new byte[1500];
    private byte[] h264frame=new byte[9000];  
    private SurfaceHolder mSurfaceHolder;
    private SurfaceView mSurfaceView; 
    private MediaRecorder mMediaRecorder=null;     
    private Camera camera=null;
    private int StartMdatPlace=0;
    private int videoRate=10;
    private String fd="/storage/sdcard0/DCIM/Camera/h2.3gp"; 
    public static DatagramSocket client=null;	    
   	public static InetAddress addr=null;
   	int seq_num=0,bytes=0,h264length=0,ts_current=0;
    static int videoWidth,videoHeight,timestamp_increse; 
    static float framerate;
    static String Ip;     
    DataInputStream dtInput=null;
    SharedPreferences sharedPre;
    LocalServerSocket lss;
    LocalSocket receiver,sender;    
   	
    @Override
    public void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState); 
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.main);        
        SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(this);        
        videoHeight=settings.getInt("video_resX", 240);
        videoWidth=settings.getInt("video_resY", 320);       
        framerate=Float.parseFloat(settings.getString("video_framerate", "10"));
        timestamp_increse=(int)(90000.0/framerate);
        Ip=settings.getString("video_targetip", "192.168.1.145");       
        settings.registerOnSharedPreferenceChangeListener(this);        
        InitSurfaceView();//初始化播放界面
        InitMediaSharePre();//初始化属性记录器                 
    }     
    
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
    	if (key.equals("video_resX") || key.equals("video_resY")) {
    		videoWidth = sharedPreferences.getInt("video_resX", 0);
    		videoHeight = sharedPreferences.getInt("video_resY", 0);
    	}else if (key.equals("video_framerate")) {    		
    		framerate = Float.parseFloat(sharedPreferences.getString("video_framerate", "10"));
    	}else if(key.equals("video_targetip")) {
    		Ip = sharedPreferences.getString("video_targetip", "192.168.1.145");
    	}    	
    }
    
    @Override
    public void onClick(View v){
    	try{            	
            client.close();
        	dtInput.close();
        	receiver.close();
        	sender.close();
        	lss.close();              	
        } 
        catch(IOException e){               
        }   
    	while(mMediaRecorder!= null){ 
  			releaseMediaRecorder();   			                    
        }   	
    	mMediaRecorderRecording=false;
    	mSurfaceView.setVisibility(View.INVISIBLE);    	
	}    
    
    //初始化SurfaceView   
    private void InitSurfaceView(){
        mSurfaceView=(SurfaceView)this.findViewById(R.id.surface_camera);
        mSurfaceView.setOnClickListener(this);
        SurfaceHolder holder=mSurfaceView.getHolder();
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mSurfaceView.setVisibility(View.VISIBLE);
    }        
    
    //初始化，记录mdat开始位置    
    private void InitMediaSharePre(){
        sharedPre=this.getSharedPreferences(mediaShare, MODE_PRIVATE);       
    }    
    
    public void surfaceCreated(SurfaceHolder holder) {
        mSurfaceHolder=holder;
    } 
    
    public void surfaceChanged(SurfaceHolder holder,int format,int width,int height){
        mSurfaceHolder=holder;
        if(!mMediaRecorderRecording){
            InitLocalSocket();//初始化数据接口
            getSPSAndPPS();            
            initializeVideo();//开始录像
            startVideo();
        }         
    } 
    
    public void surfaceDestroyed(SurfaceHolder holder) {
    	   	                
    }     
    
    private void InitLocalSocket() {
    	try  {
    		lss=new LocalServerSocket("H264");
            receiver=new LocalSocket();             
            receiver.connect(new LocalSocketAddress("H264"));
            receiver.setReceiveBufferSize(300000);
            receiver.setSendBufferSize(50000);             
            sender=lss.accept();
            sender.setReceiveBufferSize(300000);
            sender.setSendBufferSize(50000);             
        } catch(IOException e){
            Log.e(TAG, e.toString());
            this.finish();
            return;
        }         
    }    
    
    //得到序列参数集SPS和图像参数集PPS
    private void getSPSAndPPS() {
    	//读取属性文件
        StartMdatPlace=sharedPre.getInt(
        		String.format("mdata_%d%d.mdat",videoHeight,videoWidth),-1);        
        //数据存在
        if(StartMdatPlace!=-1){
            byte[] temp=new byte[100];
            try {
                FileInputStream file_in=MainActivity.this.openFileInput(
                        String.format("%d%d.sps",videoHeight,videoWidth));                 
                int index=0;
                int read=0;
                while(true) {
                    read=file_in.read(temp,index,10);
                    if(read==-1) break;
                    else index+= read;
                }
                Log.e(TAG, "sps length:"+index);
                SPS=new byte[index];
                System.arraycopy(temp,0,SPS,0,index);                                
                file_in.close();                 
                index=0;
                //read PPS
                file_in=MainActivity.this.openFileInput(
                        String.format("%d%d.pps",videoHeight,videoWidth));
                while(true) {
                    read=file_in.read(temp,index,10);
                    if(read==-1) break;
                    else index+=read;
                }
                Log.e(TAG, "pps length:"+index);
                PPS=new byte[index];
                System.arraycopy(temp,0,PPS,0,index);
            } 
            catch (FileNotFoundException e) {                
                Log.e(TAG, e.toString());
            } 
            catch (IOException e){                
                Log.e(TAG, e.toString());
            }
        } 
        else {
            SPS=null;
            PPS=null;
        }
    }     
    
    //初始化MediaRecorder       
    private boolean initializeVideo(){
        if(mSurfaceHolder==null) {          
        	return false;
        }         
        mMediaRecorderRecording=true;         
        if(mMediaRecorder==null){
            mMediaRecorder=new MediaRecorder();
        }         
        else {
            mMediaRecorder.reset();
        }         
        //设置摄像机参数
        if (camera==null){
        	camera=Camera.open();
            Camera.Parameters parameters=camera.getParameters();
            camera.setDisplayOrientation(90);           
            camera.setParameters(parameters); 
            camera.unlock();
		}        
        mMediaRecorder.setCamera(camera);
        mMediaRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
        mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        mMediaRecorder.setVideoFrameRate(videoRate);
        mMediaRecorder.setVideoSize(videoHeight,videoWidth);
        mMediaRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.H264);
        mMediaRecorder.setPreviewDisplay(mSurfaceHolder.getSurface());
        mMediaRecorder.setMaxDuration(0);
        mMediaRecorder.setMaxFileSize(0);        
        if(SPS==null){
            mMediaRecorder.setOutputFile(fd);
        }
        else{
            mMediaRecorder.setOutputFile(sender.getFileDescriptor());
        } 
        try{
            mMediaRecorder.prepare();
            mMediaRecorder.start();
        } 
        catch(IllegalStateException e) {
            e.printStackTrace();
        } 
        catch (IOException e){
            e.printStackTrace();
            releaseMediaRecorder();
        }         
        return true;
    }
 
    //释放MediaRecorder
    private void releaseMediaRecorder(){
        if(mMediaRecorder!= null){
            if(mMediaRecorderRecording){
                mMediaRecorder.stop();
                mMediaRecorderRecording=false;
            }
            mMediaRecorder.reset();
            mMediaRecorder.release();
            mMediaRecorder = null;            
            if (camera!= null){
            	camera.release();
    		}
        }        
    }     
  
    //启动线程
    private void startVideo(){    	
        new Thread(this).start();
    }        
         	
    public void run(){
        try{        	
            if(SPS==null){
                Log.e(TAG, "Rlease MediaRecorder and getSPSandPPS");
                Thread.sleep(2000);                
                releaseMediaRecorder();                
                findSPSAndPPS();                
                initializeVideo();
            }            
            InitSocket();
            dtInput=new DataInputStream(receiver.getInputStream());            
            dtInput.read(h264frame,0,StartMdatPlace); 
            Sendspspps();
            while(mMediaRecorderRecording){             	
                h264length=dtInput.readInt(); 
                ReadSize(h264length,dtInput);
//                if(h264frame[0]==65){
//                	Sendspspps(); 
//                	SendR(h264frame,h264length);                	
//                }else{
                SendR(h264frame,h264length);               
            }             
        } catch (Exception e) {
            e.printStackTrace();
            client.close();            
        }        
    }
    
    public static void InitSocket() throws UnknownHostException,SocketException{    	
    	client=new DatagramSocket();     	
		addr=InetAddress.getByName(Ip);		
	}
    
    private void ReadSize(int h264length,DataInputStream dataInput) 
    		throws IOException,InterruptedException{
        int read=0,temp=0;        
        while(read<h264length){
        	temp=dataInput.read(h264frame,read,h264length-read);                       
            if(temp==-1){                
                Thread.sleep(2000);
                continue;            
            }read+=temp;
        }
    } 
    
    private void Sendspspps()throws IOException{
    	memset(SAP,0,30);
    	SAP[1]=(byte)(SAP[1]|96); //负载类型号96
    	SAP[0]=(byte)(SAP[0]|0x80); //版本号,此版本固定为2
    	SAP[1]=(byte)(SAP[1]&254); //标志位,由具体协议规定其值
    	SAP[11]=10;//在本RTP回话中全局唯一,java默认采用网络字节序号不用转换    	
    	SAP[1]=(byte)(SAP[1]|0x80);
    	System.arraycopy(intToByte(seq_num++),0,SAP,2,2);
    	{  
			byte temp=0;
			temp=SAP[3];
			SAP[3]=SAP[2];
			SAP[2]=temp;
		}     		
    	SAP[12]=(byte)(SAP[12]|((byte)(SPS[0]&0x80))<<7);
    	SAP[12]=(byte)(SAP[12]|((byte)((SPS[0]&0x60)>>5))<<5);
    	SAP[12]=(byte)(SAP[12]|((byte)(SPS[0]&0x1f)));    		
		System.arraycopy(SPS,1,SAP,13,SPS.length-1);    		
		ts_current = ts_current+timestamp_increse;			
		System.arraycopy(intToByte(ts_current),0,SAP,4,4);
		{    			
			byte temp=0;
			temp=SAP[4];
			SAP[4]=SAP[7];
			SAP[7]=temp;    				
			temp=SAP[5];
			SAP[5]=SAP[6];
			SAP[6]=temp;
		}
		bytes=SPS.length+12 ;  
		client.send(new DatagramPacket(SAP,bytes,addr,9200));
		
		
		memset(SAP,0,30);
    	SAP[1]=(byte)(SAP[1]|96); //负载类型号96
    	SAP[0]=(byte)(SAP[0]|0x80); //版本号,此版本固定为2
    	SAP[1]=(byte)(SAP[1]&254); //标志位,由具体协议规定其值
    	SAP[11]=10;//在本RTP回话中全局唯一,java默认采用网络字节序号不用转换    	
    	SAP[1]=(byte)(SAP[1]|0x80);
		System.arraycopy(intToByte(seq_num++),0,SAP,2,2);
    	{  
			byte temp=0;
			temp=SAP[3];
			SAP[3]=SAP[2];
			SAP[2]=temp;
		}     		
    	SAP[12]=(byte)(SAP[12]|((byte)(PPS[0]&0x80))<<7);
    	SAP[12]=(byte)(SAP[12]|((byte)((PPS[0]&0x60)>>5))<<5);
    	SAP[12]=(byte)(SAP[12]|((byte)(PPS[0]&0x1f)));    		
		System.arraycopy(PPS,1,SAP,13,PPS.length-1);    		
		ts_current = ts_current+timestamp_increse;			
		System.arraycopy(intToByte(ts_current),0,SAP,4,4);
		{    			
			byte temp=0;
			temp=SAP[4];
			SAP[4]=SAP[7];
			SAP[7]=temp;    				
			temp=SAP[5];
			SAP[5]=SAP[6];
			SAP[6]=temp;
		}
		bytes=PPS.length+12 ;  
		client.send(new DatagramPacket(SAP,bytes,addr,9200));		
    }    
    
    private void SendR(byte[] r,int h264len)throws IOException{    	
    	memset(sendbuf,0,1500); 
    	sendbuf[1]=(byte)(sendbuf[1]|96); //负载类型号96
    	sendbuf[0]=(byte)(sendbuf[0]|0x80); //版本号,此版本固定为2
    	sendbuf[1]=(byte)(sendbuf[1]&254); //标志位,由具体协议规定其值
    	sendbuf[11]=10;//在本RTP回话中全局唯一,java默认采用网络字节序号不用转换    	
    	if(h264len<=1400){        		   			
    		sendbuf[1]=(byte)(sendbuf[1]|0x80);     		
    		System.arraycopy(intToByte(seq_num++),0,sendbuf,2,2);
    		{  
				byte temp=0;
				temp=sendbuf[3];
				sendbuf[3]=sendbuf[2];
				sendbuf[2]=temp;
    		}     		
    		sendbuf[12]=(byte)(sendbuf[12]|((byte)(r[0]&0x80))<<7);
    		sendbuf[12]=(byte)(sendbuf[12]|((byte)((r[0]&0x60)>>5))<<5);
    		sendbuf[12]=(byte)(sendbuf[12]|((byte)(r[0]&0x1f)));    		
    		System.arraycopy(r,1,sendbuf,13,h264len-1);    		
    		ts_current = ts_current+timestamp_increse;			
    		System.arraycopy(intToByte(ts_current),0,sendbuf,4,4);
    		{    			
				byte temp=0;
				temp=sendbuf[4];
				sendbuf[4]=sendbuf[7];
				sendbuf[7]=temp;    				
				temp=sendbuf[5];
				sendbuf[5]=sendbuf[6];
				sendbuf[6]=temp;
    		}
			bytes=h264len+12 ;  
			client.send(new DatagramPacket(sendbuf,bytes,addr,9200));        		
    	}
    	else if(h264len>1400){    		
    		int k=0,l=0;
    		k=h264len/1400;
    		l=h264len%1400;
    		int t=0; 
    		ts_current=ts_current+timestamp_increse;    		
    		System.arraycopy(intToByte(ts_current),0,sendbuf,4,4);
    		{  	
				byte temp=0;
				temp=sendbuf[4];
				sendbuf[4]=sendbuf[7];
				sendbuf[7]=temp;    				
				temp=sendbuf[5];
				sendbuf[5]=sendbuf[6];
				sendbuf[6]=temp;				
    		}
    		while(t<=k){    			
    			System.arraycopy(intToByte(seq_num++),0,sendbuf,2,2);
    			{
    				byte temp=0;
    				temp=sendbuf[3];
    				sendbuf[3]=sendbuf[2];
    				sendbuf[2]=temp;
    			}
    			if(t==0){    				
    				sendbuf[1]=(byte)(sendbuf[1]&0x7F);     				
            		sendbuf[12]=(byte)(sendbuf[12]|((byte)(r[0]&0x80))<<7);
            		sendbuf[12]=(byte)(sendbuf[12]|((byte)((r[0]&0x60)>>5))<<5);
            		sendbuf[12]=(byte)(sendbuf[12]|(byte)(28));           		
            		sendbuf[13]=(byte)(sendbuf[13]&0xBF);//E=0
            		sendbuf[13]=(byte)(sendbuf[13]&0xDF);//R=0
            		sendbuf[13]=(byte)(sendbuf[13]|0x80);//S=1
            		sendbuf[13]=(byte)(sendbuf[13]|((byte)(r[0]&0x1f)));           		
            		System.arraycopy(r,1,sendbuf,14,1400);
            		client.send(new DatagramPacket(sendbuf,1414,addr,9200));
            		t++;
    			}    			
    			else if(t==k){     				
    				sendbuf[1]=(byte)(sendbuf[1]|0x80);    				
    				sendbuf[12]=(byte)(sendbuf[12]|((byte)(r[0]&0x80))<<7);
            		sendbuf[12]=(byte)(sendbuf[12]|((byte)((r[0]&0x60)>>5))<<5);
            		sendbuf[12]=(byte)(sendbuf[12]|(byte)(28));            		
            		sendbuf[13]=(byte)(sendbuf[13]&0xDF); //R=0
            		sendbuf[13]=(byte)(sendbuf[13]&0x7F); //S=0
            		sendbuf[13]=(byte)(sendbuf[13]|0x40); //E=1
            		sendbuf[13]=(byte)(sendbuf[13]|((byte)(r[0]&0x1f)));            		
            		System.arraycopy(r,t*1400+1,sendbuf,14,l-1);
            		bytes=l-1+14;
            		client.send(new DatagramPacket(sendbuf,bytes,addr,9200));
            		t++;
    			}
    			else if(t<k&&0!=t) {    				
    				sendbuf[1]=(byte)(sendbuf[1]&0x7F); //M=0     				
    				sendbuf[12]=(byte)(sendbuf[12]|((byte)(r[0]&0x80))<<7);
            		sendbuf[12]=(byte)(sendbuf[12]|((byte)((r[0]&0x60)>>5))<<5);
            		sendbuf[12]=(byte)(sendbuf[12]|(byte)(28));            		
             		sendbuf[13]=(byte)(sendbuf[13]&0xDF); //R=0
            		sendbuf[13]=(byte)(sendbuf[13]&0x7F); //S=0
            		sendbuf[13]=(byte)(sendbuf[13]&0xBF); //E=0
            		sendbuf[13]=(byte)(sendbuf[13]|((byte)(r[0]&0x1f)));            		
            		System.arraycopy(r,t*1400+1,sendbuf,14,1400);					
					client.send(new DatagramPacket(sendbuf,1414,addr,9200));					
					t++;
    			}
    		}
    	}
	}     
    
    public static void memset(byte[] buf,int value,int size){
		for (int i=0;i<size;i++){
			buf[i]=(byte)value;
		}
	}
    
    public static byte[] intToByte(int number){ 
        int temp=number; 
        byte[] b=new byte[4]; 
        for(int i=0;i<b.length;i++){ 
            b[i]=new Integer(temp&0xff).byteValue();
            temp=temp >> 8; 
        } 
        return b; 
    }   
    
    //SPS And PPS   
    private void findSPSAndPPS() throws Exception{
        File file=new File(fd);
        FileInputStream fileInput=new FileInputStream(file);         
        int length=(int)file.length();
        byte[] data=new byte[length];         
        fileInput.read(data);         
        final byte[] mdat=new byte[]{0x6D,0x64,0x61,0x74};
        final byte[] avcc=new byte[]{0x61,0x76,0x63,0x43};         
        for(int i=0;i<length; i++){
            if(data[i]==mdat[0]&&data[i+1]==mdat[1]&& 
            		data[i+2]==mdat[2]&&data[i+3]==mdat[3]){
                StartMdatPlace = i+4;//find mdat
                break;
            }
        }
        Log.e(TAG, "StartMdatPlace:"+StartMdatPlace);
        //记录到xml文件里
        String mdatStr=String.format("mdata_%d%d.mdat",videoHeight,videoWidth);
        Editor editor=sharedPre.edit();
        editor.putInt(mdatStr, StartMdatPlace);      
        editor.commit();         
        for(int i=0;i<length;i++){
            if(data[i]==avcc[0]&&data[i+1]==avcc[1]&&data[i+2]==avcc[2]
            		&&data[i+3]==avcc[3]){
                int sps_start = i+3+7;//i+3指avcc的c，加7跳过6位AVCDecoderConfigRecord.               
                //sps length and data
                byte[] sps_3gp=new byte[2];//sps length
                sps_3gp[1]=data[sps_start];
                sps_3gp[0]=data[sps_start+1];
                int sps_length=bytes2short(sps_3gp);
                Log.e(TAG, "sps_length :"+sps_length);
                 
                sps_start+=2;//skip length
                SPS=new byte[sps_length];
                System.arraycopy(data,sps_start,SPS,0,sps_length);
                //save sps
                FileOutputStream file_out=MainActivity.this.openFileOutput(
                        String.format("%d%d.sps",videoHeight,videoWidth),
                        Context.MODE_PRIVATE);
                file_out.write(SPS);
                file_out.close();                 
                //pps length and data
                int pps_start=sps_start+sps_length+1;
                byte[] pps_3gp=new byte[2];
                pps_3gp[1]=data[pps_start];
                pps_3gp[0]=data[pps_start+1];
                int pps_length=bytes2short(pps_3gp);
                Log.e(TAG, "PPS LENGTH:"+pps_length);                 
                pps_start+=2;                 
                PPS=new byte[pps_length];
                System.arraycopy(data,pps_start,PPS,0,pps_length);                  
                //Save PPS
                file_out = MainActivity.this.openFileOutput(
                        String.format("%d%d.pps",videoHeight,videoWidth),
                        Context.MODE_PRIVATE);
                file_out.write(PPS);
                file_out.close();
                break;
            }
        }
        fileInput.close();         
    }     
    
    public short bytes2short(byte[] b){    	
    	short mask=0xff;
    	short temp=0;
    	short res=0;
    	for(int i=0;i<2;i++) {
    		res<<=8;
    		temp=(short)(b[1-i]&mask);
    		res|=temp;
    	}
    	return res;
    }
 
    @Override
    public boolean onCreateOptionsMenu(Menu menu){
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
 
    @Override
    protected void onPause(){        
        super.onPause();
        try{            	
            client.close();
        	dtInput.close();
        	receiver.close();
        	sender.close();
        	lss.close();              	
        } 
        catch(IOException e){               
        }   
    	while(mMediaRecorder!= null){ 
  			releaseMediaRecorder();   			                    
        }   	
    	mMediaRecorderRecording=false;
    	mSurfaceView.setVisibility(View.INVISIBLE);
        finish();
    }       
    @Override
  	public void onStart(){
  		super.onStart();
  	}  	
  	@Override
  	public void onStop() {
  		super.onStop();   		
  		try{            	
            client.close();
        	dtInput.close();
        	receiver.close();
        	sender.close();
        	lss.close();              	
        } 
        catch(IOException e){               
        }   
    	while(mMediaRecorder!= null){ 
  			releaseMediaRecorder();   			                    
        }   	
    	mMediaRecorderRecording=false;
    	mSurfaceView.setVisibility(View.INVISIBLE);
        finish();	
  	}  	
}