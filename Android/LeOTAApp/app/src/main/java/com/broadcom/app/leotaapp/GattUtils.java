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
package com.broadcom.app.leotaapp;

import java.util.ArrayDeque;
import java.util.Queue;
import java.util.UUID;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.util.Log;

/**
 * Helper functions to work with services and characteristics
 */
public class GattUtils {
    private static final String TAG = Constants.TAG_PREFIX + "GattUtils";

    /**
     * Queue for ensuring read/write requests are serialized so that each
     * read/write completes for the the next read/write request is performed
     */
    public static class RequestQueue {

        /**
         * Internal object used to manage a specific read or write request
         */
        private class GattRequest {
            private static final int REQUEST_READ_CHAR = 1;
            private static final int REQUEST_WRITE_CHAR = 2;
            private static final int REQUEST_READ_DESC = 11;
            private static final int REQUEST_WRITE_DESC = 12;

            private final int mRequestType;
            private final byte[] mValue;
            private final BluetoothGatt mGatt;
            private final BluetoothGattCharacteristic mCharacteristic;
            private final BluetoothGattDescriptor mDescriptor;

            public GattRequest(int requestType, BluetoothGatt gatt,
                    BluetoothGattCharacteristic characteristic) {
                mGatt = gatt;
                mCharacteristic = characteristic;
                mDescriptor = null;
                mRequestType = requestType;
                mValue = null;
            }

            public GattRequest(int requestType, BluetoothGatt gatt,
                               BluetoothGattCharacteristic characteristic, byte [] value) {
                mGatt = gatt;
                mValue = value;
                mCharacteristic = characteristic;
                mDescriptor = null;
                mRequestType = requestType;
            }

            public GattRequest(int requestType, BluetoothGatt gatt,
                    BluetoothGattDescriptor descriptor) {
                mGatt = gatt;
                mValue = null;
                mDescriptor = descriptor;
                mCharacteristic = null;
                mRequestType = requestType;
            }

            public GattRequest(int requestType, BluetoothGatt gatt,
                               BluetoothGattDescriptor descriptor, byte[] value) {
                mGatt = gatt;
                mValue = value;
                mDescriptor = descriptor;
                mCharacteristic = null;
                mRequestType = requestType;
            }
        }

        // Queue containing requests
        private final Queue<GattRequest> mRequestQueue = new ArrayDeque<GattRequest>();

        // If true, the request queue is currently processing read/write
        // requests
        private boolean mIsRunning;

        /**
         * Add a read descriptor request to the queue
         *
         * @param gatt
         * @param descriptor
         */
        public synchronized void addReadDescriptor(BluetoothGatt gatt,
                BluetoothGattDescriptor descriptor) {
            if (gatt == null || descriptor == null) {
                Log.d(TAG, "addReadDescriptor(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_READ_DESC, gatt, descriptor));
        }

        /**
         * Add a write descriptor request to the queue
         *
         * @param gatt
         * @param descriptor
         */
        public synchronized void addWriteDescriptor(BluetoothGatt gatt,
                BluetoothGattDescriptor descriptor, byte[] value) {
            if (gatt == null || descriptor == null) {
                Log.d(TAG, "addWriteDescriptor(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_WRITE_DESC, gatt, descriptor, value));
        }

        /**
         * Add a read characteristic request to the queue
         *
         * @param gatt
         * @param characteristic
         */
        public synchronized void addReadCharacteristic(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic) {
            if (gatt == null || characteristic == null) {
                Log.d(TAG, "addReadCharacteristic(): invalid data");
                return;
            }
            mRequestQueue.add(new GattRequest(GattRequest.REQUEST_READ_CHAR, gatt, characteristic));
        }

        /**
         * Add a write characteristic request to the queue
         *
         * @param gatt
         * @param characteristic
         */
        public synchronized void addWriteCharacteristic(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic, byte[] value) {
            if (gatt == null || characteristic == null) {
                Log.d(TAG, "addWriteCharacteristic(): invalid data");
                return;
            }
            mRequestQueue
                    .add(new GattRequest(GattRequest.REQUEST_WRITE_CHAR, gatt, characteristic, value));
        }

        /**
         * Clear all the requests in the queue
         */
        public void clear() {
            mRequestQueue.clear();
        }

        /**
         * Get the next queued request, if any, and perform the requested
         * operation
         */
        public void next() {
            GattRequest request = null;
            synchronized (this) {
                Log.d(TAG, "next: queue size=" + mRequestQueue.size() + " mIsRunning=" + mIsRunning);
                request = mRequestQueue.poll();
                if (request == null) {
                    Log.d(TAG, "next: no request()");
                    mIsRunning = false;
                    return;
                }
            }

            if (request.mRequestType == GattRequest.REQUEST_READ_CHAR) {
                request.mGatt.readCharacteristic(request.mCharacteristic);
            } else if (request.mRequestType == GattRequest.REQUEST_WRITE_CHAR) {
                request.mCharacteristic.setValue(request.mValue);
                request.mGatt.writeCharacteristic(request.mCharacteristic);
            } else if (request.mRequestType == GattRequest.REQUEST_READ_DESC) {
                request.mGatt.readDescriptor(request.mDescriptor);
            } else if (request.mRequestType == GattRequest.REQUEST_WRITE_DESC) {
                request.mDescriptor.setValue(request.mValue);
                request.mGatt.writeDescriptor(request.mDescriptor);
            }
        }

        /**
         * Start processing the queued requests, if not already started.
         * Otherwise, this method does nothing if processing already started
         */
        public void execute() {
            synchronized (this) {
                Log.d(TAG, "execute: queue size=" + mRequestQueue.size() + " mIsRunning=" + mIsRunning);
                if (mIsRunning) {
                    return;
                }
                mIsRunning = true;
            }
            next();
        }
    }

    /**
     * Create a read/write request queue
     *
     * @return
     */
    public static RequestQueue createRequestQueue() {
        return new RequestQueue();
    }

    /**
     * Get a characteristic with the specified service UUID and characteristic
     * UUID
     *
     * @param gatt
     * @param serviceUuid
     * @param characteristicUuid
     * @return
     */
    public static BluetoothGattCharacteristic getCharacteristic(BluetoothGatt gatt,
            UUID serviceUuid, UUID characteristicUuid) {
        if (gatt == null) {
            return null;
        }
        BluetoothGattService service = gatt.getService(serviceUuid);
        if (service == null) {
            return null;
        }
        return service.getCharacteristic(characteristicUuid);
    }

    /**
     * Get a descriptor with the specified service UUID, characteristic UUID,
     * and descriptor UUID
     *
     * @param gatt
     * @param serviceUuid
     * @param characteristicUuid
     * @param descriptorUuid
     * @return
     */
    public static BluetoothGattDescriptor getDescriptor(BluetoothGatt gatt, UUID serviceUuid,
            UUID characteristicUuid, UUID descriptorUuid) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, serviceUuid,
                characteristicUuid);
        if (characteristic != null) {
            return characteristic.getDescriptor(descriptorUuid);
        }
        return null;

    }

    /**
     * Converts a byte array into a long value
     *
     * @param bytes
     * @param bytesToRead
     * @param offset
     * @return
     */
    public static long unsignedBytesToLong(byte[] bytes, int bytesToRead, int offset) {
        int shift = 0;
        long l = 0;
        for (int i = offset; i < bytesToRead; i++) {
            int byteFF = bytes[i] & 0xFF;
            l = l + ((long) (bytes[i] & 0xFF) << (8 * shift++));
        }
        return l;
    }

}
