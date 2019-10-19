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
package com.cypress.app.otasppapp;

import java.util.UUID;

/**
 * Contains the UUID of services, characteristics, and descriptors
 */
public class Constants {
    public static final String TAG_PREFIX = "OTASPPApp."; //used for debugging

    /**
     * UUID of Upgrade Service
     */
    public static final UUID UPGRADE_SERVICE_UUID = UUID.fromString("ae5d1e47-5c13-43a0-8635-82ad38a1381f");

    /**
     * UUID of Secure Upgrade Service
     */
	public static final UUID SECURE_UPGRADE_SERVICE_UUID = UUID.fromString("c7261110-f425-447a-a1bd-9d7246768bd8");

    /**
     * UUID of Upgrade Control Point
     */
    public static final UUID UPGRADE_CHARACTERISTIC_CONTROL_POINT_UUID = UUID.fromString("a3dd50bf-f7a7-4e99-838e-570a086c661b");

    /**
     * UUID of Upgrade Control Data
     */
    public static final UUID UPGRADE_CHARACTERISTIC_DATA_UUID = UUID.fromString("a2e86c7a-d961-4091-b74f-2409e72efe26");


    /**
     * UUID of OTA Service
     */
//    public static final UUID OTA_SERVICE_UUID = UUID
//            .fromString("695293b2-059d-47e0-a63b-7ebef2fa607e");

    /**
     * UUID of ota configuration characteristic
     */
//    public static final UUID OTA_CHARACTERISTIC_CONFIGURATION_UUID = UUID
//            .fromString("614146e4-ef00-42bc-8727-902d3cfe5e8b");




//    /**
//     * UUID of ota input characteristic
//     */
//    public static final UUID OTA_CHARACTERISTIC_INPUT_UUID = UUID
//            .fromString("8ac32d3f-5cb9-4d44-bec2-ee689169f626");

    /**
     * UUID of the client configuration descriptor
     */
    public static final UUID CLIENT_CONFIG_DESCRIPTOR_UUID = UUID
            .fromString("00002902-0000-1000-8000-00805f9b34fb");

    /**
     * UUID of battery service
     */
    public static final UUID BATTERY_SERVICE_UUID = UUID
            .fromString("0000180F-0000-1000-8000-00805f9b34fb");

    /**
     * UUID of battery level characteristic
     */
    public static final UUID BATTERY_LEVEL_UUID = UUID
            .fromString("00002a19-0000-1000-8000-00805f9b34fb");

    /**
     * UUID of device information service
     */
    public static final UUID DEVICE_INFO_SERVICE_UUID = UUID
            .fromString("0000180A-0000-1000-8000-00805f9b34fb");

    /**
     * UUID of manufacturer name characteristic
     */
    public static final UUID MANUFACTURER_NAME_UUID = UUID
            .fromString("00002A29-0000-1000-8000-00805f9b34fb");
    /**
     * UUID of model number characteristic
     */
    public static final UUID MODEL_NUMBER_UUID = UUID
            .fromString("00002A24-0000-1000-8000-00805f9b34fb");

    /**
     * UUID of system id characteristic
     */
    public static final UUID SYSTEM_ID_UUID = UUID
            .fromString("00002A23-0000-1000-8000-00805f9b34fb");

}
