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
