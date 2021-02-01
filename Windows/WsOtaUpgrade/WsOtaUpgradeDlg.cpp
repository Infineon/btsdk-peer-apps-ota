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

// WsOtaUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "stdint.h"
#include "afxdialogex.h"
#include <setupapi.h>
#include "WsOtaUpgrade.h"
#include "WsOtaUpgradeDlg.h"
#include "wiced_bt_ota_firmware_upgrade.h"

#include "DeviceSelectAdv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern HMODULE hLib;
extern void BthAddrToBDA(BD_ADDR bda, ULONGLONG *btha);


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// WsOtaUpgradeDlg dialog
CWsOtaUpgradeDlg::CWsOtaUpgradeDlg(LPBYTE pPatch, DWORD dwPatchSize, CWnd* pParent /*=NULL*/)
    : CDialogEx(CWsOtaUpgradeDlg::IDD, pParent),
    m_pPatch(pPatch),
    m_dwPatchSize(dwPatchSize)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_btInterface = NULL;
    m_pDownloader = NULL;
    m_bWin8 = FALSE;
    m_bWin10 = FALSE;
    m_numDevices = 0;
}

CWsOtaUpgradeDlg::~CWsOtaUpgradeDlg()
{
    delete m_btInterface;
    delete m_pDownloader;
}

void CWsOtaUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_LIST, m_cbDevices);
    DDX_Control(pDX, IDC_UPGRADE_PROGRESS, m_Progress);
}

BEGIN_MESSAGE_MAP(CWsOtaUpgradeDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_CONNECTED, OnConnected)
    ON_MESSAGE(WM_NOTIFIED, OnNotified)
    ON_MESSAGE(WM_PROGRESS, OnProgress)
    ON_MESSAGE(WM_DEVICE_DISCONNECTED, &CWsOtaUpgradeDlg::OnDeviceDisconnected)
    ON_MESSAGE(WM_DEVICE_CONNECTED, &CWsOtaUpgradeDlg::OnDeviceConnected)

    ON_BN_CLICKED(IDC_START, &CWsOtaUpgradeDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDC_SELECTDEVICE, &CWsOtaUpgradeDlg::OnSelectDevice)
END_MESSAGE_MAP()

// CWsOtaUpgradeDlg message handlers

BOOL CWsOtaUpgradeDlg::OnInitDialog()
{
    BOOL bConnected = TRUE;  // assume that device is connected

    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // if Windows 10 we load the device from previous device scan.
    if (m_bWin10)
    {
        UINT8 bda[BD_ADDR_LEN] = { 0 };
        WCHAR buf[64] = { 0 };

        for (int i = 0; i < 6; i++)
            bda[5 - i] = (BYTE)m_bth.rgBytes[i];

        swprintf_s(buf, sizeof(buf) / sizeof(buf[0]), L"%02x%02x%02x%02x%02x%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

        m_cbDevices.SetItemData(m_cbDevices.AddString(buf), (DWORD_PTR)0);
        m_numDevices++;
        m_cbDevices.SetCurSel(0);
    }
    else
        GetDeviceList();

    GetDlgItem(IDC_UPGRADE_PROGRESS)->ShowWindow((m_numDevices == 0) ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_DEVICE_LIST)->ShowWindow((m_numDevices == 0) ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_NO_DEVICES)->ShowWindow((m_numDevices == 0) ? SW_SHOW : SW_HIDE);
    GetDlgItem(IDC_START)->SetWindowText((m_numDevices == 0) ? L"Done" : L"Start");

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWsOtaUpgradeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWsOtaUpgradeDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWsOtaUpgradeDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CWsOtaUpgradeDlg::PostNcDestroy()
{
    CDialogEx::PostNcDestroy();
}

void CWsOtaUpgradeDlg::SetParam(BLUETOOTH_ADDRESS *bth)
{
    m_bth = *bth;
}

BOOL CWsOtaUpgradeDlg::GetDeviceList()
{
    HDEVINFO                            hardwareDeviceInfo;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
    WCHAR                               szBda[13] = {0};
    HANDLE                              hDevice = INVALID_HANDLE_VALUE;

    m_numDevices = 0;

    if ((hardwareDeviceInfo = SetupDiGetClassDevs (NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT)) != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA DeviceInfoData;

        memset(&DeviceInfoData, 0, sizeof(DeviceInfoData));
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);

        WCHAR szService[80];

        for (int i = 0; i < 2; i++)
        {
            GUID sGUID = (i == 0) ? GUID_OTA_FW_UPGRADE_SERVICE : GUID_OTA_SEC_FW_UPGRADE_SERVICE;
            GUID guid;
            if (m_bWin8 || m_bWin10)
            {
                guid = sGUID;
            }
            else
            {
                guid.Data1 = (sGUID.Data4[4]) + (sGUID.Data4[5] << 8) + (sGUID.Data4[6] << 16) + (sGUID.Data4[7] << 24);
                guid.Data2 = (sGUID.Data4[2]) + (sGUID.Data4[3] << 8);
                guid.Data3 = (sGUID.Data4[0]) + (sGUID.Data4[1] << 8);
                guid.Data4[0] = (sGUID.Data3) & 0xff;
                guid.Data4[1] = (sGUID.Data3 >> 8) & 0xff;
                guid.Data4[2] = (sGUID.Data2) & 0xff;
                guid.Data4[3] = (sGUID.Data2 >> 8) & 0xff;
                guid.Data4[4] = (sGUID.Data1) & 0xff;
                guid.Data4[5] = (sGUID.Data1 >> 8) & 0xff;
                guid.Data4[6] = (sGUID.Data1 >> 16) & 0xff;
                guid.Data4[7] = (sGUID.Data1 >> 24) & 0xff;
            }
            UuidToString(szService, 80, &guid);
            ods("%S\n", szService);
            for (DWORD n = 0; SetupDiEnumDeviceInfo(hardwareDeviceInfo, n, &DeviceInfoData); n++)
            {
                DWORD dwBytes = 0;

                SetupDiGetDeviceInstanceId(hardwareDeviceInfo, &DeviceInfoData, NULL, 0, &dwBytes);

                PWSTR szInstanceId = new WCHAR[dwBytes];
                if (szInstanceId)
                {
                    if (SetupDiGetDeviceInstanceId(hardwareDeviceInfo, &DeviceInfoData, szInstanceId, dwBytes, &dwBytes))
                    {
                        _wcsupr_s(szInstanceId, dwBytes);
                        if (wcsstr(szInstanceId, szService))
                        {
                            OutputDebugStringW(szInstanceId);
                            WCHAR buf[13];
                            wchar_t* pStart;
                            wchar_t* pEnd;
                            if (m_bWin8 || m_bWin10)
                            {
                                pStart = wcsrchr(szInstanceId, '_');
                                pEnd = wcsrchr(szInstanceId, '\\');
                            }
                            else
                            {
                                pStart = wcsrchr(szInstanceId, '&');
                                pEnd = wcsrchr(szInstanceId, '_');
                            }
                            if (pStart && pEnd)
                            {
                                *pEnd = 0;
                                wcscpy_s(buf, pStart + 1);
                                m_cbDevices.SetItemData(m_cbDevices.AddString(buf), (DWORD_PTR)i);
                                m_numDevices++;
                            }
                        }
                    }
                    delete[] szInstanceId;
                }
            }
        }
        SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    }
    if (m_numDevices)
        m_cbDevices.SetCurSel(0);

    return TRUE;
}

LRESULT CWsOtaUpgradeDlg::OnConnected(WPARAM bConnected, LPARAM lparam)
{
    SetDlgItemText(IDC_DEVICE_STATE, bConnected ? L"Connected" : L"Disconnected");

    if (!bConnected)
        return S_OK;

    SetDlgItemText(IDC_STATUS, L"Ready");
    m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_CONNECTED);
    return S_OK;
}

LRESULT CWsOtaUpgradeDlg::OnNotified(WPARAM bConnected, LPARAM lparam)
{
    BTW_GATT_VALUE *pValue = (BTW_GATT_VALUE *)lparam;
    if (pValue->len == 1)
    {
        switch (pValue->value[0])
        {
        case WICED_OTA_UPGRADE_STATUS_OK:
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_RESPONSE_OK);
            break;
        case WICED_OTA_UPGRADE_STATUS_CONTINUE:
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_CONTINUE);
            break;
        default:
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_RESPONSE_FAILED);
            break;
        }
    }
    free (pValue);

    return S_OK;
}

LRESULT CWsOtaUpgradeDlg::OnProgress(WPARAM state, LPARAM param)
{
    static UINT total;
    if (state == WSDownloader::WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD)
    {
        total = (UINT)param;
        m_Progress.SetRange32(0, (int)param);
        SetDlgItemText(IDC_STATUS, L"Transfer");
        SetDlgItemText(IDC_START, L"Abort");
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_DATA_TRANSFER)
    {
        m_Progress.SetPos((int)param);
        if (param == total)
        {
            m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_START_VERIFICATION);
            SetDlgItemText(IDC_STATUS, L"Download verification");
        }
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_VERIFIED)
    {
        SetDlgItemText(IDC_STATUS, L"Success");
        SetDlgItemText(IDC_START, L"Done");
    }
    else if (state == WSDownloader::WS_UPGRADE_STATE_ABORTED)
    {
        m_Progress.SetPos(total);
        SetDlgItemText(IDC_STATUS, L"Aborted");
        SetDlgItemText(IDC_START, L"Done");
    }
    return S_OK;
}

void CWsOtaUpgradeDlg::OnBnClickedStart()
{
    // if no devices exit
    if (m_numDevices == 0)
    {
        OnCancel();
        return;
    }
    // if downloader is completed exit
    if ((m_pDownloader != NULL)
     && ((m_pDownloader->m_state == WSDownloader::WS_UPGRADE_STATE_VERIFIED)
      || (m_pDownloader->m_state == WSDownloader::WS_UPGRADE_STATE_ABORTED)))
    {
        OnCancel();
        return;
    }
    // if transfer do abort
    if ((m_pDownloader != NULL) && (m_pDownloader->m_state == WSDownloader::WS_UPGRADE_STATE_DATA_TRANSFER))
    {
        m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_ABORT);
        return;
    }

    int sel = m_cbDevices.GetCurSel();
    WCHAR buf[13];
    m_cbDevices.GetLBText(m_cbDevices.GetCurSel(), buf);
    int bda[6];
    BLUETOOTH_ADDRESS bth = {0};
    if (swscanf_s(buf, L"%02x%02x%02x%02x%02x%02x", &bda[0], &bda[1], &bda[2], &bda[3], &bda[4], &bda[5]) == 6)
    {
        for (int i = 0; i < 6; i++)
            bth.rgBytes[5 - i] = (BYTE)bda[i];
    }

    if (m_bWin10)
        m_btInterface = new CBtWin10Interface(&bth, this);
    else if (m_bWin8)
        m_btInterface = new CBtWin8Interface(&bth, hLib, this);
    else
        m_btInterface = new CBtWin7Interface(&bth, hLib, this);

    m_btInterface->m_bSecure = (BOOL)m_cbDevices.GetItemData(sel);

    if (!m_btInterface->Init())
    {
        MessageBox(L"Error initializing interface. This device may not support OTA FW Upgrade. Select another device.", L"Error", MB_OK);
        return;
    }

    if (m_bWin10 && !((CBtWin10Interface*)m_btInterface)->CheckForOTAServices())
    {
        MessageBox(L"This device may not support OTA FW Upgrade. Select another device.", L"Error", MB_OK);
        return;
    }

    m_pDownloader = new WSDownloader(m_btInterface, m_pPatch, m_dwPatchSize, m_hWnd);

    // on Win7 we will receive notification when device is connected and will intialize dialog there
    if (!m_bWin8 && !m_bWin10)
        return;

    if (m_bWin10)
    {
        CBtWin10Interface *pWin10BtInterface = dynamic_cast<CBtWin10Interface *>(m_btInterface);
        DWORD mtu = 0;

        // Assume that we are connected.  Failed attempt to read battery will change that to FALSE.
        pWin10BtInterface->m_bConnected = TRUE;

        if (pWin10BtInterface->m_bConnected)
        {
            BTW_GATT_VALUE gatt_value;
            gatt_value.len = 2;
            gatt_value.value[0] = 3;
            gatt_value.value[1] = 0;

            guidSvcWSUpgrade = m_btInterface->m_bSecure ? GUID_OTA_SEC_FW_UPGRADE_SERVICE : GUID_OTA_FW_UPGRADE_SERVICE;

            pWin10BtInterface->SetDescriptorValue(&guidSvcWSUpgrade, &guidCharWSUpgradeControlPoint, BTW_GATT_UUID_DESCRIPTOR_CLIENT_CONFIG, &gatt_value);
            pWin10BtInterface->RegisterNotification(&guidSvcWSUpgrade, &guidCharWSUpgradeControlPoint);

            mtu = pWin10BtInterface->GetMTUSize();
            if (mtu)
                m_pDownloader->SetMTU(mtu);

            OnConnected(TRUE, NULL);
        }
    }
    else if (m_bWin8)
    {
        CBtWin8Interface *pWin8BtInterface = dynamic_cast<CBtWin8Interface *>(m_btInterface);

        // Assume that we are connected.  Failed attempt to read battery will change that to FALSE.
        pWin8BtInterface->m_bConnected = TRUE;

        if (pWin8BtInterface->m_bConnected)
        {
            // register for notifications with the status
            for (int i = 0; i < 3; i++)
            {
                USHORT ClientConfDescrControlPoint = 3;
                if ((pWin8BtInterface->m_bConnected = m_btInterface->SetDescriptorValue(ClientConfDescrControlPoint)) == TRUE)
                {
                    pWin8BtInterface->RegisterNotification();
                    OnConnected(TRUE, NULL);
                    break;
                }
                Sleep(1000);
            }
        }
    }
}

void CWsOtaUpgradeDlg::OnSelectDevice()
{
    if (!m_bWin10)
        return;

    CDeviceSelectAdv dlgDeviceSelect;
    dlgDeviceSelect.m_bWin8 = FALSE;
    dlgDeviceSelect.m_bth.ullLong = 0;
    INT_PTR nResponse = dlgDeviceSelect.DoModal();
    if ((nResponse == IDOK))
    {
        OutputDebugStringW(L"CMeshControllerDlg::dlg.hLib");

        {
            CBtWin10Interface *pWin10BtInterface = dynamic_cast<CBtWin10Interface *>(m_btInterface);
            pWin10BtInterface->ResetInterface();
            delete m_btInterface;
            m_btInterface = NULL;
        }

        m_bth = dlgDeviceSelect.m_bth;

        UINT8 bda[BD_ADDR_LEN] = { 0 };
        WCHAR buf[64] = { 0 };

        for (int i = 0; i < 6; i++)
            bda[5 - i] = (BYTE)m_bth.rgBytes[i];

        swprintf_s(buf, sizeof(buf) / sizeof(buf[0]), L"%02x%02x%02x%02x%02x%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

        m_cbDevices.ResetContent();
        m_cbDevices.SetItemData(m_cbDevices.AddString(buf), (DWORD_PTR)0);
        m_numDevices = 1;
        m_cbDevices.SetCurSel(0);
    }

    return;
}

LRESULT CWsOtaUpgradeDlg::OnDeviceConnected(WPARAM Instance, LPARAM lparam)
{
    ods("OnDeviceConnected:\n");

    return S_OK;
}

LRESULT CWsOtaUpgradeDlg::OnDeviceDisconnected(WPARAM Instance, LPARAM lparam)
{
    ods("OnDeviceDisconnected:\n");

    if (m_pDownloader)
        m_pDownloader->ProcessEvent(WSDownloader::WS_UPGRADE_DISCONNECTED);

    CBtWin10Interface *pWin10BtInterface = dynamic_cast<CBtWin10Interface *>(m_btInterface);
    if (pWin10BtInterface)
    {
        pWin10BtInterface->ResetInterface();
        delete m_btInterface;
        m_btInterface = NULL;
    }

    return S_OK;
}
