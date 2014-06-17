package cn.itcast.h264test;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

public class Option extends PreferenceActivity {		
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);        
        addPreferencesFromResource(R.xml.preferences);
        
        final SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(this);
        final Preference videoEnabled = findPreference("stream_video");     
        final ListPreference videoEncoder = (ListPreference) findPreference("video_encoder");
        final ListPreference videoResolution = (ListPreference) findPreference("video_resolution");        
        final ListPreference videoFramerate = (ListPreference) findPreference("video_framerate");
        final ListPreference videoBitrate = (ListPreference) findPreference("video_bitrate");
        final ListPreference videoTaGetIp = (ListPreference) findPreference("video_targetip");
        
        boolean videoState = settings.getBoolean("stream_video", true);
        videoEncoder.setEnabled(videoState);
		videoResolution.setEnabled(videoState);		
		videoFramerate.setEnabled(videoState);
		videoBitrate.setEnabled(videoState);		
        
        videoResolution.setSummary(getString(R.string.settings0)+" "+videoResolution.getValue()+"px");
        videoFramerate.setSummary(getString(R.string.settings1)+" "+videoFramerate.getValue()+"fps");
        videoBitrate.setSummary(getString(R.string.settings2)+" "+videoBitrate.getValue()+"kbps");
        videoTaGetIp.setSummary(videoTaGetIp.getValue());        

        videoResolution.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
        	public boolean onPreferenceChange(Preference preference, Object newValue) {
        		Editor editor = settings.edit();
        		Pattern pattern = Pattern.compile("([0-9]+)x([0-9]+)");
        		Matcher matcher = pattern.matcher((String)newValue);
        		matcher.find();
        		editor.putInt("video_resX", Integer.parseInt(matcher.group(1)));
        		editor.putInt("video_resY", Integer.parseInt(matcher.group(2)));
        		editor.commit();
        		videoResolution.setSummary(getString(R.string.settings0)+" "+(String)newValue+"px");
        		return true;
			}
        });
        
        videoFramerate.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
        	public boolean onPreferenceChange(Preference preference, Object newValue) {
        		videoFramerate.setSummary(getString(R.string.settings1)+" "+(String)newValue+"fps");        		    		
        		return true;
			}
        });

        videoBitrate.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
        	public boolean onPreferenceChange(Preference preference, Object newValue) {
        		videoBitrate.setSummary(getString(R.string.settings2)+" "+(String)newValue+"kbps");
        		return true;
			}
        });
        
        videoTaGetIp.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
        	public boolean onPreferenceChange(Preference preference, Object newValue) {
        		videoTaGetIp.setSummary((String)newValue);        		
        		return true;
			}
        });
        
        videoEnabled.setOnPreferenceChangeListener(new Preference.OnPreferenceChangeListener() {
        	public boolean onPreferenceChange(Preference preference, Object newValue) {
        		boolean state = (Boolean)newValue;
        		videoEncoder.setEnabled(state);
        		videoResolution.setEnabled(state);
        		videoBitrate.setEnabled(state);
        		videoFramerate.setEnabled(state);
        		return true;
			}
        });        
    }   
}
