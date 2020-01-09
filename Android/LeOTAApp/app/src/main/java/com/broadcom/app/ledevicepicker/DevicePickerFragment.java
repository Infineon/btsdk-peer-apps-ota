/******************************************************************************
 *
 *  Copyright (C) 2013-2014 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
package com.broadcom.app.ledevicepicker;

import java.util.Collection;

import com.broadcom.app.leotaapp.Constants;
import com.broadcom.app.leotaapp.R;
import com.broadcom.app.ledevicepicker.DeviceListFragment.Callback;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

/**
 * Wrapper fragment to wrap the Device Picker device list in a Fragment
 * @author fredc
 *
 */
public class DevicePickerFragment extends DialogFragment implements DeviceListFragment.Callback,
        android.view.View.OnClickListener {
    private static final String TAG = Constants.TAG_PREFIX + "DevicePickerFragment";

    public static DevicePickerFragment createDialog(Callback callback, String dialogTitle,
            boolean startScanning) {
        DevicePickerFragment f = new DevicePickerFragment();
        f.mTitle = dialogTitle;
        f.mCallback = callback;
        f.mStartScanning = startScanning;
        f.setStyle(DialogFragment.STYLE_NORMAL, R.style.DialogTheme);

        return f;
    }

    private String mTitle;
    private Callback mCallback;
    private Button mScanButton;
    private boolean mIsScanning;
    private boolean mStartScanning;
    private DeviceListFragment mDevicePickerFragment;

    private void setScanState(boolean isScanning) {
        if (isScanning) {
            mScanButton.setText(R.string.devicepicker_menu_stop);
        } else {
            mScanButton.setText(R.string.devicepicker_menu_scan);
        }
        mIsScanning = isScanning;

    }

    private void initDevicePickerFragment() {
        FragmentManager mgr = getFragmentManager();
        mDevicePickerFragment = (DeviceListFragment) mgr.findFragmentById(R.id.device_picker_id);
        mDevicePickerFragment.setCallback(this);

    }

    private void scan() {
        if (!mIsScanning) {
            setScanState(true);
            mDevicePickerFragment.scan(true);
        }
    }

    private void stopScan() {
        if (mIsScanning) {
            setScanState(false);
            mDevicePickerFragment.scan(false);
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Activity appContext = getActivity();
        View view = appContext.getLayoutInflater().inflate(R.layout.devicepicker_layout, null);
        mScanButton = (Button) view.findViewById(R.id.scan_button);
        mScanButton.setOnClickListener(this);
        initDevicePickerFragment();
        AlertDialog.Builder builder = new AlertDialog.Builder(appContext);
        builder.setTitle(mTitle != null ? mTitle : getActivity().getString(
                R.string.devicepicker_default_title));
        builder.setView(view);
        return builder.create();
    }

    @Override
    public void onClick(View v) {
        boolean isScanning = !mIsScanning;
        setScanState(isScanning);
        mDevicePickerFragment.scan(isScanning);
    }

    @Override
    public void onDevicePicked(BluetoothDevice device) {
        if (mCallback != null) {
            mCallback.onDevicePicked(device);
        }
        dismiss();
    }

    @Override
    public void onDevicePickCancelled() {
        if (mCallback != null) {
            mCallback.onDevicePickCancelled();
        }

    }

    @Override
    public void onDevicePickError() {
        if (mCallback != null) {
            mCallback.onDevicePickError();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mStartScanning) {
            scan();
        } else {
            stopScan();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        stopScan();
        dismiss();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        super.onDismiss(dialog);
        if (mDevicePickerFragment != null) {
            mDevicePickerFragment.getFragmentManager().beginTransaction()
                    .remove(mDevicePickerFragment).commit();
        }
    }

    /**
     * Add a collection of devices to the list of devices excluded from the
     * device picker
     *
     * @param deviceAddress
     */
    public void addExcludedDevices(Collection<String> deviceAddresses) {
        mDevicePickerFragment.addExcludedDevices(deviceAddresses);
    }

    /**
     * Add a device to the list of devices excluded from the device picker
     *
     * @param deviceAddress
     */
    public void addExcludedDevice(String deviceAddress) {
        mDevicePickerFragment.addExcludedDevice(deviceAddress);
    }

    /**
     * Remove the device from the list of devices excluded from the device
     * picker
     *
     * @param deviceAddress
     */
    public void removeExcludedDevice(String address) {
        mDevicePickerFragment.removeExcludedDevice(address);
    }

    /**
     * Clear the list of devices excluded from the device picker
     *
     * @param deviceAddress
     */
    public void clearExcludedDevices() {
        mDevicePickerFragment.clearExcludedDevices();
    }
}
