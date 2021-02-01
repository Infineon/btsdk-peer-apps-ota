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
// DeviceSelect.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DeviceSelectAdv.h"
#include "afxdialogex.h"
#include <Sddl.h>
#include <WinBase.h>

// =======================
#pragma comment(lib, "RuntimeObject.lib")

extern void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid);
extern void ods(char * fmt_str, ...);

EventRegistrationToken *watcherToken;
ComPtr<IBluetoothLEAdvertisementWatcherFactory> bleAdvWatcherFactory;
ComPtr<IBluetoothLEAdvertisementWatcher> bleWatcher;
ComPtr<IBluetoothLEAdvertisementFilter> bleFilter;
ComPtr<ITypedEventHandler<BluetoothLEAdvertisementWatcher*, BluetoothLEAdvertisementReceivedEventArgs*>> handler;

void BthAddrToBDA(BD_ADDR bda, ULONGLONG *btha)
{
    BYTE *p = (BYTE *)btha;
    STREAM_TO_BDADDR(bda, p);
}

// Prints an error string for the provided source code line and HRESULT
// value and returns the HRESULT value as an int.
int PrintError(unsigned int line, HRESULT hr)
{
    wprintf_s(L"ERROR: Line:%d HRESULT: 0x%X\n", line, hr);
    return hr;
}

template <typename T, typename U>
extern inline HRESULT await(const Microsoft::WRL::ComPtr<T> &asyncOp, U *results, UINT timeout = 0);

HRESULT CDeviceSelectAdv::OnAdvertisementReceived(IBluetoothLEAdvertisementWatcher* watcher, IBluetoothLEAdvertisementReceivedEventArgs* args)
{
    UINT64 address;
    HRESULT hr;

    ods("OnAdvertisementReceived received");

    BluetoothLEAdvertisementType  value;

    hr = args->get_AdvertisementType(&value);

    ods("Advertisement type: %d", (int)value);

    hr = args->get_BluetoothAddress(&address);
    if (FAILED(hr))
    {
        ods("OnAdvertisementReceived address not retrieved");
    }
    else
    {
        BD_ADDR bda;
        char buff[256] = { 0 };
        bool add_device = false;
        bool bOTADevice = false;
        int iIndex = 0;

        BthAddrToBDA(bda, &address);
        ods("OnAdvertisementReceived address: %lld", address);
        sprintf_s(buff, sizeof(buff), " LE Device: %02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        ods("%s", buff);

        //==
        WCHAR buf[64] = { 0 };

        BthAddrToBDA(bda, &address);
        swprintf_s(buf, sizeof(buf) / sizeof(buf[0]), L"%02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);


        WCHAR   buf2[64] = { 0 };
        int     i;
        for (i = 0; i < m_numDevices; i++)
        {
            m_lbDevices.GetText(i, buf2);
            if (!wcslen(buf2) || !wcsncmp(buf2, buf, wcslen(buf)))
                break;
        }
        // Add this device if it is new one
        if (i >= m_numDevices)
        {
            add_device = true;
            m_numDevices++;
        }
        //==


        ComPtr<IBluetoothLEAdvertisement> bleAdvert;

        hr = args->get_Advertisement(&bleAdvert);
        if (FAILED(hr))
        {
            if(add_device)
                iIndex = m_lbDevices.AddString(buf);
            ods("get_Advertisement failed");
        }
        else
        {
            ods("get_Advertisement data retreived");

            // Get Name of the device
            HString name;
            WCHAR   wbuf_name[64] = { 0 };
            WCHAR   wbuf_txt[128] = { 0 };
            char buff_name[256] = { 0 };
            hr = bleAdvert->get_LocalName(name.GetAddressOf());
            // Append device name to BDA if name is not null
            if (wcslen(name.GetRawBuffer(nullptr)) != 0)
                sprintf_s(buff_name, sizeof(buff_name), " : name (%S)", name.GetRawBuffer(nullptr));

            CString strName = name.GetRawBuffer(nullptr);
            if (strName.CompareNoCase(L"OTA_FW_UPGRADE") == 0)
                bOTADevice = true;

            ods("Local Name: %s", buff_name);

            MultiByteToWideChar(CP_UTF8, 0, (const char *)buff_name, (int)strlen(buff_name), wbuf_name, (int)sizeof(wbuf_name) / sizeof(WCHAR));
            wcscpy_s(wbuf_txt, buf);
            wcscat_s(wbuf_txt, wbuf_name);

            if (add_device)
            {
                if (!bOTADevice)
                    iIndex = m_lbDevices.AddString(wbuf_txt);
                else
                    iIndex = m_lbDevices.InsertString(0, wbuf_txt);
            }

            // Get Services
            ComPtr<ABI::Windows::Foundation::Collections::IVector<GUID>> vecGuid;

            hr = bleAdvert->get_ServiceUuids(&vecGuid);

            if (FAILED(hr))
            {
                ods("get_ServiceUuids failed");
            }
            else
            {
                UINT guidCount = 0;
                hr = vecGuid->get_Size(&guidCount);
                ods("Guid Count: %d", guidCount);

                if (SUCCEEDED(hr))
                {
                    for (int i = 0; i < (int)guidCount; ++i)
                    {
                        GUID guid = { 0 };
                        //ComPtr<GUID> guid;
                        hr = vecGuid->GetAt(i, &guid);
                        if (SUCCEEDED(hr))
                        {
                            WCHAR szService[80] = { 0 };
                            UuidToString(szService, 80, &guid);
                            sprintf_s(buff, sizeof(buff), "%S", szService);
                            ods("index:%d GUID %s", i, buff);
                        }
                    }
                }
            }
        }

        // Get Advertisement Data

        ComPtr <ABI::Windows::Foundation::Collections::IVector<ABI::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementDataSection*>> vecData;
        hr = bleAdvert->get_DataSections(&vecData);

        if (FAILED(hr))
        {
            ods("get_DataSections failed");
        }
        else
        {
            UINT count = 0;
            hr = vecData->get_Size(&count);

            ods("Datasections Count %d", count);

            if (SUCCEEDED(hr))
            {
                for (UINT i = 0; i < count; ++i)
                {
                    ComPtr<ABI::Windows::Devices::Bluetooth::Advertisement::IBluetoothLEAdvertisementDataSection> ds;

                    hr = vecData->GetAt(i, &ds);
                    if (SUCCEEDED(hr))
                    {
                        ComPtr<ABI::Windows::Storage::Streams::IBuffer> ibuf;
                        BYTE datatype = 0;

                        hr = ds->get_DataType(&datatype);
                        memset(buff, 0, sizeof(buff));
                        sprintf_s(buff, sizeof(buff), "%d", datatype);
                        ods("Data Type: %s", buff);

                        hr = ds->get_Data(&ibuf);

                        Microsoft::WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> pBufferByteAccess;
                        ibuf.As(&pBufferByteAccess);

                        // Get pointer to pixel bytes
                        byte* pdatabuf = nullptr;
                        pBufferByteAccess->Buffer(&pdatabuf);

                        memset(buff, 0, sizeof(buff));

                        UINT32 length = 0;

                        hr = ibuf->get_Length(&length);

                        ods("Length %d", length);

                        for (UINT32 i = 0; i < length; ++i)
                        {
                            sprintf_s(buff + 3 * i, sizeof(buff) - 3 * i, "%02x ", *(pdatabuf + i));
                        }

                        ods("Data Buffer: %s", buff);
                    }
                }
            }
        }


    } // got address

    return S_OK;
}

HRESULT CDeviceSelectAdv::onBluetoothLEDeviceFoundAsync(IAsyncOperation<BluetoothLEDevice *> *op, AsyncStatus status)
{
    if (status != AsyncStatus::Completed)
        return S_OK;

    ComPtr<IBluetoothLEDevice> device;
    HRESULT hr;
    hr = op->GetResults(&device);
    return hr;
}

int CDeviceSelectAdv::StartLEAdvertisementWatcher()
{
    OutputDebugStringW(L"StartLEAdvertisementWatcher");

    HRESULT hr = NULL;

    // Initialize the Windows Runtime.
    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        return PrintError(__LINE__, initialize);
    }

    watcherToken = new EventRegistrationToken();

    // Get the activation factory for the IBluetoothLEAdvertisementWatcherFactory interface.
    OutputDebugStringW(L"StartLEAdvertisementWatcher");
     hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementWatcher).Get(), &bleAdvWatcherFactory);
    if (FAILED(hr))
    {
        return PrintError(__LINE__, hr);
    }

    Wrappers::HStringReference class_id_filter2(RuntimeClass_Windows_Devices_Bluetooth_Advertisement_BluetoothLEAdvertisementFilter);
    hr = RoActivateInstance(class_id_filter2.Get(), reinterpret_cast<IInspectable**>(bleFilter.GetAddressOf()));

    hr = bleAdvWatcherFactory->Create(bleFilter.Get(), &bleWatcher);

    OutputDebugStringW(L"StartLEAdvertisementWatcher");
    if (bleWatcher == NULL)
    {
        ods("bleWatcher is NULL, err is 0x%x", hr);
    }
    else
    {
        handler = Callback<ITypedEventHandler<BluetoothLEAdvertisementWatcher*, BluetoothLEAdvertisementReceivedEventArgs*>>
            (this, &CDeviceSelectAdv::OnAdvertisementReceived);

        OutputDebugStringW(L"StartLEAdvertisementWatcher");
        hr = bleWatcher->add_Received(handler.Get(), watcherToken);
        if (FAILED(hr))
        {
            return PrintError(__LINE__, hr);
        }

        HRESULT hr = bleWatcher->Start();
        if (FAILED(hr))
        {
            return PrintError(__LINE__, hr);
        }

        while (!m_bStop) {
            Sleep(100);
        }
    }

    OutputDebugStringW(L"StartLEAdvertisementWatcher - return");
    return 0;
}

int CDeviceSelectAdv::StartwatchLEAdvertisementWatcher()
{
    OutputDebugStringW(L"StartLEAdvertisementWatcher");

    HRESULT hr = bleWatcher->Start();
    if (FAILED(hr))
    {
        return PrintError(__LINE__, hr);
    }
    return 0;
}

int CDeviceSelectAdv::StopLEAdvertisementWatcher()
{
    HRESULT hr = bleWatcher->Stop();

    m_bStop = TRUE;

    if (FAILED(hr))
    {
        return PrintError(__LINE__, hr);
    }

    return 0;
}

// =======================
// CDeviceSelectAdv dialog

IMPLEMENT_DYNAMIC(CDeviceSelectAdv, CDialogEx)

CDeviceSelectAdv::CDeviceSelectAdv(CWnd* pParent /*=NULL*/)
    : CDialogEx(CDeviceSelectAdv::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDeviceSelectAdv::~CDeviceSelectAdv()
{
}

void CDeviceSelectAdv::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_LIST_ADV, m_lbDevices);
}

BEGIN_MESSAGE_MAP(CDeviceSelectAdv, CDialogEx)
    ON_LBN_DBLCLK(IDC_DEVICE_LIST_ADV, &CDeviceSelectAdv::OnDblclkDeviceList)
    ON_BN_CLICKED(IDOK, &CDeviceSelectAdv::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDeviceSelectAdv::OnBnClickedCancel)

END_MESSAGE_MAP()

DWORD WINAPI DeviceDiscoveryThread(void *Context)
{
    CDeviceSelectAdv *pDlg = (CDeviceSelectAdv *)Context;
    pDlg->StartLEAdvertisementWatcher();
    return 0;
}

// CDeviceSelectAdv message handlers
BOOL CDeviceSelectAdv::OnInitDialog()
{
    OutputDebugStringW(L"CDeviceSelectAdv::OnInitDialog");

    CDialogEx::OnInitDialog();

    m_bStop = FALSE;

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);             // Set big icon
    SetIcon(m_hIcon, FALSE);            // Set small icon

    m_numDevices = 0;

    GetDlgItem(IDC_DEVICE_LIST_ADV)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_NO_DEVICES_ADV)->ShowWindow(SW_HIDE);

    CreateThread(NULL, 0, DeviceDiscoveryThread, this, 0, NULL);

    return TRUE;
}

void CDeviceSelectAdv::OnDblclkDeviceList()
{
    OnBnClickedOk();
}

void CDeviceSelectAdv::OnBnClickedOk()
{
    WCHAR buf_txt[128] = { 0 };
    WCHAR buf_bda[24] = { 0 };

    StopLEAdvertisementWatcher();

    m_lbDevices.GetText(m_lbDevices.GetCurSel(), buf_txt);

    // The device name contains BDA and may contain device name. Get just the formatted BDA (first 17 bytes, xx:xx:xx:xx:xx:xx)
    wcsncpy_s(buf_bda, buf_txt, 17);

    int bda[6];
    if (swscanf_s(buf_bda, L"%02x:%02x:%02x:%02x:%02x:%02x", &bda[0], &bda[1], &bda[2], &bda[3], &bda[4], &bda[5]) == 6)
    {
        for (int i = 0; i < 6; i++)
             m_bth.rgBytes[5 - i] = (BYTE)bda[i];
    }

    CDialogEx::OnOK();
}

void CDeviceSelectAdv::OnBnClickedCancel()
{
    m_bth.ullLong = 0;

    StopLEAdvertisementWatcher();

    CDialogEx::OnCancel();
}
