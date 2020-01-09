/******************************************************************************
 *
 *  Copyright (C) 2018 Cypress Semiconductor Corporation
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
package com.cypress.app.devicepicker;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import android.app.Activity;
import android.app.ListFragment;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.cypress.app.otasppapp.Constants;
import com.cypress.app.otasppapp.R;

/**
 * UI component to allow users to scan for Bluetooth devices and pick a
 * selected device *
 */
public class DeviceListFragment extends ListFragment implements BluetoothAdapter.LeScanCallback {
    private static final String TAG = Constants.TAG_PREFIX + "DevicePickerFragment";

    /**
     * Interface to listen for results
     *
     * @author fredc
     *
     */
    public static interface Callback {
        public void onDevicePicked(BluetoothDevice device);

        public void onDevicePickCancelled();

        public void onDevicePickError();
    }

    /**
     * Helper class used for displaying LE devices in a pick list
     *
     * @author fredc
     *
     */
    private static class DeviceAdapter extends BaseAdapter {
        class DeviceRecord {
            public BluetoothDevice device;
            public int rssi;
            public Long last_scanned;
            public int state;

            public DeviceRecord(BluetoothDevice device, int rssi, int state) {
                this.device = device;
                this.rssi = rssi;
                this.state = state;
                last_scanned = System.currentTimeMillis() / 1000;
            }
        }

        public static final int DEVICE_SOURCE_SCAN = 0;
        public static final int DEVICE_SOURCE_BONDED = 2;

        private long mLastUpdate = 0;

        private final Context mContext;
        private final ArrayList<DeviceRecord> mDevices;
        private final LayoutInflater mInflater;

        public DeviceAdapter(Context context) {
            mContext = context;

            mInflater = LayoutInflater.from(context);
            mDevices = new ArrayList<DeviceRecord>();
        }

        public void addDevice(BluetoothDevice device, int rssi, int state) {
            synchronized (mDevices) {
                for (DeviceRecord rec : mDevices) {
                    if (rec.device.equals(device)) {
                        rec.rssi = rssi;
                        rec.last_scanned = System.currentTimeMillis() / 1000;
                        updateUi(false);
                        return;
                    }
                }

                mDevices.add(new DeviceRecord(device, rssi, state));
                updateUi(true);
            }
        }

        public BluetoothDevice getDevice(int position) {
            if (position < mDevices.size())
                return mDevices.get(position).device;
            return null;
        }

        @Override
        public int getCount() {
            return mDevices.size();
        }

        @Override
        public Object getItem(int position) {
            return mDevices.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;

            if (convertView == null || convertView.findViewById(R.id.device_name) == null) {
                convertView = mInflater.inflate(R.layout.devicepicker_listitem, null);
                holder = new ViewHolder();
                holder.device_name = (TextView) convertView.findViewById(R.id.device_name);
                holder.device_addr = (TextView) convertView.findViewById(R.id.device_addr);
                holder.device_rssi = (ProgressBar) convertView.findViewById(R.id.device_rssi);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            DeviceRecord rec = mDevices.get(position);
            holder.device_rssi.setProgress(normaliseRssi(rec.rssi));

            String deviceName = rec.device.getName();
            if (deviceName != null && deviceName.length() > 0) {
                holder.device_name.setText(rec.device.getName());
                holder.device_addr.setText(rec.device.getAddress());
            } else {
                holder.device_name.setText(rec.device.getAddress());
                holder.device_addr.setText(mContext.getResources().getString(
                        R.string.devicepicker_unknown_device));
            }

            return convertView;
        }

        static class ViewHolder {
            TextView device_name;
            TextView device_addr;
            ProgressBar device_rssi;
        }

        private void updateUi(boolean force) {
            Long ts = System.currentTimeMillis() / 1000;
            if (force || ((ts - mLastUpdate) >= 1)) {
                removeOutdated();
                ((Activity) mContext).runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        notifyDataSetChanged();
                    }
                });
            }

            mLastUpdate = ts;
        }

        private void removeOutdated() {
            Long ts = System.currentTimeMillis() / 1000;
            synchronized (mDevices) {
                for (Iterator<DeviceRecord> it = mDevices.iterator(); it.hasNext();) {
                    DeviceRecord rec = it.next();
                    if ((ts - rec.last_scanned) > 3 && rec.state == DEVICE_SOURCE_SCAN) {
                        it.remove();
                    }
                }
            }
        }

        private int normaliseRssi(int rssi) {
            // Expected input range is -127 -> 20
            // Output range is 0 -> 100
            final int RSSI_RANGE = 147;
            final int RSSI_MAX = 20;

            return (RSSI_RANGE + (rssi - RSSI_MAX)) * 100 / RSSI_RANGE;
        }
    }

    private DeviceAdapter mDeviceAdapter;
    private BluetoothAdapter mBluetoothAdapter = null;
    private Callback mCallback;
    private boolean mDevicePicked;
    private final HashSet<String> mDevicesToExclude = new HashSet<String>();
    private ArrayAdapter<String> mPairedDevicesArrayAdapter;
    private ArrayAdapter<String> mNewDevicesArrayAdapter;

    private void addDevices() {
        BluetoothManager btManager = null;

        if (mBluetoothAdapter != null) {
            btManager = (BluetoothManager) getActivity()
                    .getSystemService(Context.BLUETOOTH_SERVICE);
        }
        if (btManager == null) {
            if (mCallback != null) {
                mCallback.onDevicePickError();
            }
            return;
        }

    }

    /**
     * Set the callback object to invoke when a device is picked OR if the
     * device picker is cancelled
     *
     * @param cb
     */
    public void setCallback(Callback cb) {
        mCallback = cb;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Activity activity = getActivity();
        BluetoothManager bluetoothManager = (BluetoothManager) activity
                .getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            mBluetoothAdapter = bluetoothManager.getAdapter();
        }
        if (mBluetoothAdapter == null) {
            if (mCallback != null) {
                try {
                    mCallback.onDevicePickError();
                } catch (Throwable t) {
                    Log.w(TAG, "onCreate(): error calling onError", t);
                }
                return;
            }
        }

        // Otherwise populate the list device
        mDeviceAdapter = new DeviceAdapter(activity);
        setListAdapter(mDeviceAdapter);

        // Register for broadcasts when a device is discovered
        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        activity.registerReceiver(mReceiver, filter);

        // Register for broadcasts when discovery has finished
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        activity.registerReceiver(mReceiver, filter);

        // Get a set of currently paired devices
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

        // If there are paired devices, add each one to the ArrayAdapter
        if (pairedDevices.size() > 0) {

            for (BluetoothDevice device : pairedDevices) {
                mDeviceAdapter.addDevice(device, 0, DeviceAdapter.DEVICE_SOURCE_BONDED);
            }
        } else {
            String noDevices = getResources().getText(R.string.none_paired).toString();
        }
    }

    @Override
    public void onListItemClick(ListView list, View view, int position, long id) {
        BluetoothDevice device = mDeviceAdapter.getDevice(position);
        if (device != null && mCallback != null) {
            try {
                mDevicePicked = true;
                mCallback.onDevicePicked(device);
            } catch (Throwable t) {
                Log.w(TAG, "onListItemClick: error calling callback", t);
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (!mDevicePicked && mCallback != null) {
            mCallback.onDevicePickCancelled();
        }
    }

    @Override
    public void onLeScan(final BluetoothDevice device, final int rssi, byte[] scanRecord) {
        if (mDevicesToExclude.size() != 0 && mDevicesToExclude.contains(device.getAddress())) {
            return;
        }
        Activity activity = getActivity();

        if(activity != null)
            activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDeviceAdapter.addDevice(device, rssi, DeviceAdapter.DEVICE_SOURCE_SCAN);
            }
        });
    }

    /**
     * Add a device to the list of devices excluded from the device picker
     *
     * @param deviceAddress
     */
    public void addExcludedDevice(String deviceAddress) {
        if (deviceAddress != null && !deviceAddress.isEmpty()
                && mDevicesToExclude.contains(deviceAddress)) {
            mDevicesToExclude.add(deviceAddress);
        }
    }

    /**
     * Add a collection of devices to the list of devices excluded from the
     * device picker
     *
     * @param deviceAddress
     */
    public void addExcludedDevices(Collection<String> deviceAddresses) {
        if (deviceAddresses != null && deviceAddresses.size() > 0) {
            for (String address : deviceAddresses) {
                addExcludedDevice(address);
            }
        }
    }

    /**
     * Remove the device from the list of devices excluded from the device
     * picker
     *
     * @param deviceAddress
     */
    public void removeExcludedDevice(String address) {
        mDevicesToExclude.remove(address);
    }

    /**
     * Clear the list of devices excluded from the device picker
     *
     * @param deviceAddress
     */
    public void clearExcludedDevices() {
        mDevicesToExclude.clear();
    }

    /**
     * Start or stop scanning for LE devices
     *
     * @param enable
     */
    public void scan(boolean enable) {
        if (mBluetoothAdapter == null)
            return;

        if (enable) {
            addDevices();
            // First cancel any previous discovery then start a new one
            mBluetoothAdapter.cancelDiscovery();
            mBluetoothAdapter.startDiscovery();
        } else {
            mBluetoothAdapter.cancelDiscovery();
        }

        getActivity().invalidateOptionsMenu();
    }

    // The BroadcastReceiver that listens for discovered devices and
    // changes the title when discovery is finished
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int rssi = 0;
            // When discovery finds a device
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                Log.d(TAG, "ACTION_FOUND");

                // Get the BluetoothDevice object from the Intent
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                // If it's already paired, skip it, because it's been listed already
                if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                    mDeviceAdapter.addDevice(device, rssi, DeviceAdapter.DEVICE_SOURCE_SCAN);
                }
                // When discovery is finished, change the Activity title
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                Log.d(TAG, "ACTION_DISCOVERY_FINISHED");
            }
            else if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(action)) {
                Log.d(TAG, "ACTION_DISCOVERY_STARTED");
            }
        }
    };

}
