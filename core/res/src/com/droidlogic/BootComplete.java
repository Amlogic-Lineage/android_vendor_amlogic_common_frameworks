/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.droidlogic;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.hardware.hdmi.HdmiDeviceInfo;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Settings;
import android.content.ContentResolver;
import android.util.Log;
import android.media.AudioManager;
import android.provider.Settings;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import java.lang.reflect.AccessibleObject;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import com.droidlogic.app.OutputModeManager;
import com.droidlogic.app.PlayBackManager;
import com.droidlogic.app.SystemControlEvent;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.app.UsbCameraManager;
import com.droidlogic.HdmiCecExtend;
import com.droidlogic.app.DolbyVisionSettingManager;

public class BootComplete extends BroadcastReceiver {
    private static final String TAG             = "BootComplete";
    private static final String DECRYPT_STATE = "encrypted";
    private static final String DECRYPT_TYPE = "file";
    private static final String DROID_SETTINGS_PACKAGE = "com.droidlogic.tv.settings";
    private static final String DROID_SETTINGS_ENCRYPTKEEPERFBE = "com.droidlogic.tv.settings.CryptKeeperFBE";

    private SystemControlEvent mSystemControlEvent;
    private boolean mHasTvUiMode;
    private SystemControlManager mSystemControlManager;
    private AudioManager mAudioManager;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i(TAG, "action: " + action);
        if (SettingsPref.getSavedBootCompletedStatus(context)) {
            SettingsPref.setSavedBootCompletedStatus(context, false);
            return;
        }
        SettingsPref.setSavedBootCompletedStatus(context, true);
        mSystemControlManager =  SystemControlManager.getInstance();
        mHasTvUiMode = mSystemControlManager.getPropertyBoolean("ro.vendor.platform.has.tvuimode", false);
        final ContentResolver resolver = context.getContentResolver();
        //register system control callback
        mSystemControlEvent = new SystemControlEvent(context);
        mSystemControlManager.setListener(mSystemControlEvent);
        final OutputModeManager outputModeManager = new OutputModeManager(context);

        mAudioManager = (AudioManager) context.getSystemService(context.AUDIO_SERVICE);

        int currentIndex = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        setWiredDeviceConnectionState(SystemControlEvent.DEVICE_OUT_AUX_DIGITAL,
                (outputModeManager.isHDMIPlugged() == true) ? 1 : 0, "", "");

        if (SettingsPref.getFirstRun(context)) {
            Log.i(TAG, "first running: " + context.getPackageName());
            SettingsPref.setFirstRun(context, false);
            mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, currentIndex, 1 << 12);
        }

        /*setThisValue for dts scale*/
        outputModeManager.setDtsDrcScaleSysfs();

        //use to check whether disable camera or not
        new UsbCameraManager(context).bootReady();

        if (mHasTvUiMode)
            new DolbyVisionSettingManager(context).initSetDolbyVision();

        new PlayBackManager(context).initHdmiSelfadaption();

        if (needCecExtend(mSystemControlManager, context)) {
            new HdmiCecExtend(context);
        }

        //start optimization service
        context.startService(new Intent(context, Optimization.class));

        if (context.getPackageManager().hasSystemFeature(NetflixService.FEATURE_SOFTWARE_NETFLIX)) {
            context.startService(new Intent(context, NetflixService.class));
        }

        context.startService(new Intent(context,NtpService.class));

        if (mSystemControlManager.getPropertyBoolean("net.wifi.suspend", false))
            context.startService(new Intent(context, WifiSuspendService.class));

        if (mHasTvUiMode)
            context.startService(new Intent(context, EsmService.class));

        Intent gattServiceIntent = new Intent(context, DialogBluetoothService.class);
        context.startService(gattServiceIntent);

        /*  AML default rotation config, cannot use with shipping_api_level=28
            String rotProp = mSystemControlManager.getPropertyString("persist.vendor.sys.app.rotation", "");
            ContentResolver res = context.getContentResolver();
            int acceRotation = Settings.System.getIntForUser(res,
                Settings.System.ACCELEROMETER_ROTATION,
                0,
                UserHandle.USER_CURRENT);
            if (rotProp != null && ("middle_port".equals(rotProp) || "force_land".equals(rotProp))) {
                    if (0 != acceRotation) {
                        Settings.System.putIntForUser(res,
                            Settings.System.ACCELEROMETER_ROTATION,
                            0,
                            UserHandle.USER_CURRENT);
                    }
            }
         */

        enableCryptKeeperComponent(context);

        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            SettingsPref.setSavedBootCompletedStatus(context, false);
        }
    }

    private void setWiredDeviceConnectionState(int type, int state, String address, String name) {
        try {
            Class<?> audioManager = Class.forName("android.media.AudioManager");
            Method setwireState = audioManager.getMethod("setWiredDeviceConnectionState",
                                    int.class, int.class, String.class, String.class);
            Log.d(TAG,"setWireDeviceConnectionState "+setwireState);
            setwireState.invoke(mAudioManager, type, state, address, name);
        } catch(ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException ex) {
            ex.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

    private void enableCryptKeeperComponent(Context context) {
        String state = SystemProperties.get("ro.crypto.state");
        String type = SystemProperties.get("ro.crypto.type");
        boolean isMultiUser = UserManager.supportsMultipleUsers();
        if (("".equals(state) || !DECRYPT_STATE.equals(state) || !DECRYPT_TYPE.equals(type)) || !isMultiUser) {
            return;
        }

        PackageManager pm = context.getPackageManager();
        ComponentName name = new ComponentName(DROID_SETTINGS_PACKAGE, DROID_SETTINGS_ENCRYPTKEEPERFBE);
        Log.d(TAG, "enableCryptKeeperComponent " + name);
        try {
            pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                    PackageManager.DONT_KILL_APP);
        } catch (Exception e) {
            Log.e(TAG, e.toString());
        }
    }

    private boolean needCecExtend(SystemControlManager sm, Context context) {
        //return sm.getPropertyInt("ro.hdmi.device_type", -1) == HdmiDeviceInfo.DEVICE_PLAYBACK;
        return true;
    }

}
