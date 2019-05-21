/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

package dk.bearware.gui;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.RingtonePreference;
import android.text.TextUtils;
import android.util.Log;
import android.view.MenuItem;

import java.util.List;

import dk.bearware.SoundLevel;
import dk.bearware.StreamType;
import dk.bearware.TeamTalkBase;
import dk.bearware.User;
import dk.bearware.backend.TeamTalkConnection;
import dk.bearware.backend.TeamTalkConnectionListener;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.Preferences;

/**
 * A {@link PreferenceActivity} that presents a set of application settings. On handset devices, settings are presented
 * as a single list. On tablets, settings are split by category, with category headers shown to the left of the list of
 * settings.
 * <p>
 * See <a href="http://developer.android.com/design/patterns/settings.html"> Android Design: Settings</a> for design
 * guidelines and the <a href="http://developer.android.com/guide/topics/ui/settings.html">Settings API Guide</a> for
 * more information on developing a Settings UI.
 */
public class PreferencesActivity extends PreferenceActivity implements TeamTalkConnectionListener {

    public static final String TAG = "bearware";

    TeamTalkConnection mConnection;
    TeamTalkService ttservice;

    int ACTIVITY_REQUEST_BEARWAREID = 2;
    
    @Override
    protected void onStart() {
        super.onStart();
        
        // Bind to LocalService if not already
        if (mConnection == null)
            mConnection = new TeamTalkConnection(this);
        if (!mConnection.isBound()) {
            Intent intent = new Intent(getApplicationContext(), TeamTalkService.class);
            Log.d(TAG, "Binding TeamTalk service");
            if(!bindService(intent, mConnection, Context.BIND_AUTO_CREATE))
                Log.e(TAG, "Failed to bind to TeamTalk service");
        }
    }
    
    @Override
    protected void onStop() {
        super.onStop();
        
        updateSettings();
        
        // Unbind from the service
        if(mConnection.isBound()) {
            Log.d(TAG, "Unbinding TeamTalk service");
            unbindService(mConnection);
            mConnection.setBound(false);
        }
    }
    
    void updateSettings() {

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

        if(ttservice == null)
    		return;
    				
        TeamTalkBase ttinst = ttservice.getTTInstance();
        User myself = ttservice.getUsers().get(ttinst.getMyUserID());
        if (myself != null) {
            String nickname = ttservice.getServerEntry().nickname;
            if (TextUtils.isEmpty(nickname)) {
                String def_nick = getResources().getString(R.string.pref_default_nickname);
                nickname = prefs.getString(Preferences.PREF_GENERAL_NICKNAME, def_nick);
            }
            if (!nickname.equals(myself.szNickname)) {
                ttinst.doChangeNickname(nickname);
            }
        }
        
        int mf_volume = prefs.getInt(Preferences.PREF_SOUNDSYSTEM_MEDIAFILE_VOLUME, 100);
        mf_volume = Utils.refVolume(mf_volume);
        for(User u: ttservice.getUsers().values()) {
            ttinst.setUserVolume(u.nUserID, StreamType.STREAMTYPE_MEDIAFILE_AUDIO, mf_volume);
        }
    }

    /**
     * Determines whether to always show the simplified settings UI, where settings are presented in a single list. When
     * false, settings are shown as a master/detail two-pane view on tablets. When true, a single pane is shown on
     * tablets.
     */
    private static final boolean ALWAYS_SIMPLE_PREFS = false;

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        setupSimplePreferencesScreen();
    }

    @Override
    protected void onResume() {
        super.onResume();

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        String username = prefs.getString(Preferences.PREF_GENERAL_BEARWARE_USERNAME, "");

        CheckBoxPreference preference = (CheckBoxPreference) findPreference(Preferences.PREF_GENERAL_BEARWARE_CHECKED);
        preference.setChecked(username.length() > 0);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
    	// getCanonicalName() returns a string with '$' separator instead of '.'
        return GeneralPreferenceFragment.class.getName().equals(fragmentName) ||
            SoundEventsPreferenceFragment.class.getName().equals(fragmentName) ||
            ConnectionPreferenceFragment.class.getName().equals(fragmentName) ||
            TtsPreferenceFragment.class.getName().equals(fragmentName) ||
            SoundSystemPreferenceFragment.class.getName().equals(fragmentName);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == ACTIVITY_REQUEST_BEARWAREID && resultCode == RESULT_OK) {
            // BearWare login preference handled by onResume()
        }
    }

    /**
     * Shows the simplified settings UI if the device configuration dictates that a
     * simplified, single-pane UI should be shown.
     */
    @SuppressWarnings("deprecation")
    @Deprecated
    private void setupSimplePreferencesScreen() {
        if(!isSimplePreferences(this)) {
            return;
        }

        // In the simplified UI, fragments are not used at all and we instead
        // use the older PreferenceActivity APIs.

        // Add 'general' preferences.
        PreferenceCategory fakeHeader = new PreferenceCategory(this);
        fakeHeader.setTitle(R.string.pref_header_general);
        //getPreferenceScreen().addPreference(fakeHeader); //not allowed
        addPreferencesFromResource(R.xml.pref_general);

        // Add 'soundevents' preferences, and a corresponding header.
        fakeHeader = new PreferenceCategory(this);
        fakeHeader.setTitle(R.string.pref_title_audio_icons);
        getPreferenceScreen().addPreference(fakeHeader);
        addPreferencesFromResource(R.xml.pref_soundevents);

        // Add 'connection' preferences, and a corresponding header.
        fakeHeader = new PreferenceCategory(this);
        fakeHeader.setTitle(R.string.pref_header_connection);
        getPreferenceScreen().addPreference(fakeHeader);
        addPreferencesFromResource(R.xml.pref_connection);

        // Add 'tts' preferences, and a corresponding header.
        fakeHeader = new PreferenceCategory(this);
        fakeHeader.setTitle(R.string.pref_header_tts);
        getPreferenceScreen().addPreference(fakeHeader);
        addPreferencesFromResource(R.xml.pref_tts);

        // Add 'sound system' preferences, and a corresponding header.
        fakeHeader = new PreferenceCategory(this);
        fakeHeader.setTitle(R.string.pref_header_soundsystem);
        getPreferenceScreen().addPreference(fakeHeader);
        addPreferencesFromResource(R.xml.pref_soundsystem);

        // Bind the summaries of EditText/List/Dialog/Ringtone preferences to
        // their values. When their values change, their summaries are updated
        // to reflect the new value, per the Android Design guidelines.
        bindPreferenceSummaryToValue(findPreference(Preferences.PREF_GENERAL_NICKNAME));

        Preference bearwareLogin = findPreference(Preferences.PREF_GENERAL_BEARWARE_CHECKED);
        bearwareLogin.setOnPreferenceChangeListener((preference, o) -> {
            Intent edit = new Intent(PreferencesActivity.this, WebLoginActivity.class);
            startActivityForResult(edit, ACTIVITY_REQUEST_BEARWAREID);
            return true;
        });
    }

    /** {@inheritDoc} */
    @Override
    public boolean onIsMultiPane() {
        return isXLargeTablet(this) && !isSimplePreferences(this);
    }

    /**
     * Helper method to determine if the device has an extra-large screen. For example, 10" tablets are extra-large.
     */
    private static boolean isXLargeTablet(Context context) {
        return (context.getResources().getConfiguration().screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK) >= Configuration.SCREENLAYOUT_SIZE_XLARGE;
    }

    /**
     * Determines whether the simplified settings UI should be shown. This is true if this is forced via
     * {@link #ALWAYS_SIMPLE_PREFS}, or the device doesn't have newer APIs like {@link PreferenceFragment}, or the
     * device doesn't have an extra-large screen. In these cases, a single-pane "simplified" settings UI should be
     * shown.
     */
    private static boolean isSimplePreferences(Context context) {
        return ALWAYS_SIMPLE_PREFS
            || Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB
            || !isXLargeTablet(context);
    }

    /** {@inheritDoc} */
    @Override
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public void onBuildHeaders(List<Header> target) {
        if(!isSimplePreferences(this)) {
            loadHeadersFromResource(R.xml.pref_headers, target);
        }
    }

    /**
     * A preference value change listener that updates the preference's summary to reflect its new value.
     */
    private static Preference.OnPreferenceChangeListener sBindPreferenceSummaryToValueListener = new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object value) {
            String stringValue = value.toString();

            if(preference instanceof ListPreference) {
                // For list preferences, look up the correct display value in
                // the preference's 'entries' list.
                ListPreference listPreference = (ListPreference) preference;
                int index = listPreference.findIndexOfValue(stringValue);

                // Set the summary to reflect the new value.
                preference.setSummary(index >= 0
                    ? listPreference.getEntries()[index] : null);

            }
            else if(preference instanceof RingtonePreference) {
                // For ringtone preferences, look up the correct display value
                // using RingtoneManager.
                if(TextUtils.isEmpty(stringValue)) {
                    // Empty values correspond to 'silent' (no ringtone).

                }
                else {
                    Ringtone ringtone = RingtoneManager.getRingtone(
                        preference.getContext(), Uri.parse(stringValue));

                    if(ringtone == null) {
                        // Clear the summary if there was a lookup error.
                        preference.setSummary(null);
                    }
                    else {
                        // Set the summary to reflect the new ringtone display
                        // name.
                        String name = ringtone.getTitle(preference.getContext());
                        preference.setSummary(name);
                    }
                }

            }
            else if (preference instanceof CheckBoxPreference) {
                if (preference.getKey().equals(Preferences.PREF_GENERAL_BEARWARE_CHECKED)) {
                }
            }
            else {
                // For all other preferences, set the summary to the value's
                // simple string representation.
                preference.setSummary(stringValue);
            }
            return true;
        }
    };

    /**
     * Binds a preference's summary to its value. More specifically, when the preference's value is changed, its summary
     * (line of text below the preference title) is updated to reflect the value. The summary is also immediately
     * updated upon calling this method. The exact display format is dependent on the type of preference.
     * 
     * @see #sBindPreferenceSummaryToValueListener
     */
    private static void bindPreferenceSummaryToValue(Preference preference) {
        // Set the listener to watch for value changes.
        preference.setOnPreferenceChangeListener(sBindPreferenceSummaryToValueListener);

        // Trigger the listener immediately with the preference's
        // current value.
        sBindPreferenceSummaryToValueListener.onPreferenceChange(
            preference,
            PreferenceManager.getDefaultSharedPreferences(
                preference.getContext()).getString(preference.getKey(), ""));
    }

    /**
     * This fragment shows general preferences only. It is used when the activity is showing a two-pane settings UI.
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class GeneralPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_general);

            // Bind the summaries of EditText/List/Dialog/Ringtone preferences
            // to their values. When their values change, their summaries are
            // updated to reflect the new value, per the Android Design
            // guidelines.
            bindPreferenceSummaryToValue(findPreference(Preferences.PREF_GENERAL_NICKNAME));
        }
    }
    
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class SoundEventsPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_soundevents);
        }
    }
    
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class ConnectionPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_connection);
        }
    }

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class TtsPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_tts);
        }
    }

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class SoundSystemPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_soundsystem);
        }
    }

    @Override
    public void onServiceConnected(TeamTalkService service) {
    	this.ttservice = service;
    }

    @Override
    public void onServiceDisconnected(TeamTalkService service) {
    }
}
