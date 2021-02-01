/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#pragma once
#include "afxwin.h"

#include <bthdef.h>

#include <iostream>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <wrl\event.h>
#include <stdio.h>
#include <windows.devices.bluetooth.h>
#include <windows.devices.bluetooth.advertisement.h>
#include <windows.devices.bluetooth.genericattributeprofile.h>

#include <memory>
#include <functional>

#include <windows.storage.h>
#include <windows.storage.streams.h>
#include <Robuffer.h>


using namespace std;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace ABI::Windows::Devices::Bluetooth::Advertisement;

using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;


#ifndef BD_ADDR_LEN
#define BD_ADDR_LEN     6
typedef UINT8 BD_ADDR[BD_ADDR_LEN];
#endif

#define STREAM_TO_BDADDR(a, p)   {register int _i; register UINT8 *pbda = (UINT8 *)a + BD_ADDR_LEN - 1; for (_i = 0; _i < BD_ADDR_LEN; _i++) *pbda-- = *p++;}



// CDeviceSelectAdv dialog

class CDeviceSelectAdv : public CDialogEx
{
    DECLARE_DYNAMIC(CDeviceSelectAdv)

public:
    CDeviceSelectAdv(CWnd* pParent = NULL);   // standard constructor
    virtual ~CDeviceSelectAdv();

// Dialog Data
    enum { IDD = IDD_SELECT_DEVICE_ADV };

    BLUETOOTH_ADDRESS m_bth;
    BOOL m_bWin8;

public:
    int m_numDevices;

protected:
    HICON m_hIcon;

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDblclkDeviceList();
    virtual BOOL OnInitDialog();
    CListBox m_lbDevices;
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();

    HRESULT OnAdvertisementReceived(IBluetoothLEAdvertisementWatcher* watcher, IBluetoothLEAdvertisementReceivedEventArgs* args);
    HRESULT onBluetoothLEDeviceFoundAsync(IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status);
    int StartLEAdvertisementWatcher();
    int StopLEAdvertisementWatcher();
    int StartwatchLEAdvertisementWatcher();

    BOOL m_bStop;



};
