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

// WsOtaUpgrade.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "btwleapis.h"
#include "WsOtaUpgrade.h"
#include "WsOtaUpgradeDlg.h"
#include "BTFullLibPath.h"
#include "DeviceSelectAdv.h"

#include <VersionHelpers.h>
#include <Sddl.h>
#include <WinBase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HMODULE hLib;

GUID guidSvcWSUpgrade;
GUID guidCharWSUpgradeControlPoint;
GUID guidCharWSUpgradeData;

// CWsOtaUpgradeApp

BEGIN_MESSAGE_MAP(CWsOtaUpgradeApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWsOtaUpgradeApp construction

CWsOtaUpgradeApp::CWsOtaUpgradeApp()
{
}


// The one and only CWsOtaUpgradeApp object

CWsOtaUpgradeApp theApp;

BOOL IsOSWin10()
{
    BOOL bIsWin10 = FALSE;

    bIsWin10 = IsWindows10OrGreater();

    DWORD dwVersion = 0;
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0;
    DWORD dwBuild = 0;

    dwVersion = GetVersion();

    // Get the Windows version.
    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Get the build number.
    if (dwVersion < 0x80000000)
        dwBuild = (DWORD)(HIWORD(dwVersion));

    char buf[100];
    memset(buf, 0, sizeof(buf));
    sprintf_s(buf, "Version is %d.%d (%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);

    if (dwMajorVersion == 10 && dwBuild < 15063)
        return FALSE;

    return bIsWin10;
}

BOOL IsOSWin8()
{
    BOOL bIsWin8 = FALSE;

    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 2;
    osvi.wServicePackMajor = 0;

    // Initialize the condition mask.
    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

    // Perform the test.
    if(VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
    {
        bIsWin8 = TRUE;
    }

    return bIsWin8;
}

BOOL IsOSWin7()
{
    BOOL bIsWinSeven = FALSE;

    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

    // Initialize the OSVERSIONINFOEX structure.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 1;
    osvi.wServicePackMajor = 0;

    // Initialize the condition mask.
    VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );

    // Perform the test.
    if(VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
    {
        bIsWinSeven = TRUE;
    }

    return bIsWinSeven;
}

void BtwGuidFromGuid(GUID *pOut, const GUID *pIn)
{
    pOut->Data1 = (pIn->Data4[4] << 24) + (pIn->Data4[5] << 16) + (pIn->Data4[6] << 8) + pIn->Data4[7];
    pOut->Data2 = (pIn->Data4[2] << 8) + pIn->Data4[3];
    pOut->Data3 = (pIn->Data4[0] << 8) + pIn->Data4[1];
    pOut->Data4[0] = (pIn->Data3      ) & 0xff;
    pOut->Data4[1] = (pIn->Data3 >> 8 ) & 0xff;
    pOut->Data4[2] = (pIn->Data2      ) & 0xff;
    pOut->Data4[3] = (pIn->Data2 >> 8 ) & 0xff;
    pOut->Data4[4] = (pIn->Data1      ) & 0xff;
    pOut->Data4[5] = (pIn->Data1 >> 8 ) & 0xff;
    pOut->Data4[6] = (pIn->Data1 >> 16) & 0xff;
    pOut->Data4[7] = (pIn->Data1 >> 24) & 0xff;
}



DWORD MakeSDAbsolute(PSECURITY_DESCRIPTOR psidOld, PSECURITY_DESCRIPTOR *psidNew)
{
    PSECURITY_DESCRIPTOR  pSid = NULL;
    DWORD                 cbDescriptor = 0;
    DWORD                 cbDacl = 0;
    DWORD                 cbSacl = 0;
    DWORD                 cbOwnerSID = 0;
    DWORD                 cbGroupSID = 0;
    PACL                  pDacl = NULL;
    PACL                  pSacl = NULL;
    PSID                  psidOwner = NULL;
    PSID                  psidGroup = NULL;
    BOOL                  fPresent = FALSE;
    BOOL                  fSystemDefault = FALSE;
    DWORD                 dwReturnValue = ERROR_SUCCESS;

    // Get SACL
    if (!GetSecurityDescriptorSacl(psidOld, &fPresent, &pSacl, &fSystemDefault))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    if (pSacl && fPresent)
    {
        cbSacl = pSacl->AclSize;
    }

    // Get DACL
    if (!GetSecurityDescriptorDacl(psidOld, &fPresent, &pDacl, &fSystemDefault))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    if (pDacl && fPresent)
    {
        cbDacl = pDacl->AclSize;
    }

    // Get Owner
    if (!GetSecurityDescriptorOwner(psidOld, &psidOwner, &fSystemDefault))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    cbOwnerSID = GetLengthSid(psidOwner);

    // Get Group
    if (!GetSecurityDescriptorGroup(psidOld, &psidGroup, &fSystemDefault))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    cbGroupSID = GetLengthSid(psidGroup);

    // Do the conversion
    cbDescriptor = 0;

    MakeAbsoluteSD(psidOld, pSid, &cbDescriptor, pDacl, &cbDacl, pSacl,
        &cbSacl, psidOwner, &cbOwnerSID, psidGroup,
        &cbGroupSID);

    pSid = (PSECURITY_DESCRIPTOR)malloc(cbDescriptor);
    if (!pSid)
    {
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUP;
    }

    ZeroMemory(pSid, cbDescriptor);

    if (!InitializeSecurityDescriptor(pSid, SECURITY_DESCRIPTOR_REVISION))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    if (!MakeAbsoluteSD(psidOld, pSid, &cbDescriptor, pDacl, &cbDacl, pSacl,
        &cbSacl, psidOwner, &cbOwnerSID, psidGroup,
        &cbGroupSID))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

CLEANUP:

    if (dwReturnValue != ERROR_SUCCESS && pSid)
    {
        free(pSid);
        pSid = NULL;
    }

    *psidNew = pSid;

    return dwReturnValue;
}

// CWsOtaUpgradeApp initialization

BOOL CWsOtaUpgradeApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();


    //==== CoInitializeEx and Security for WRL LE Functionality ====//
    HRESULT hr = NULL;

    if (IsOSWin10())
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

        const WCHAR* security = L"O:BAG:BAD:(A;;0x7;;;PS)(A;;0x3;;;SY)(A;;0x7;;;BA)(A;;0x3;;;AC)(A;;0x3;;;LS)(A;;0x3;;;NS)";
        PSECURITY_DESCRIPTOR pSecurityDescriptor;
        ULONG securityDescriptorSize;

        if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
            security,
            SDDL_REVISION_1,
            &pSecurityDescriptor,
            &securityDescriptorSize))
        {
            return FALSE;
        }

        // MakeSDAbsolute as defined in
        // https://github.com/pauldotknopf/WindowsSDK7-Samples/blob/master/com/fundamentals/dcom/dcomperm/SDMgmt.Cpp
        PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor = NULL;
        MakeSDAbsolute(pSecurityDescriptor, &pAbsoluteSecurityDescriptor);

        HRESULT hResult = CoInitializeSecurity(
            pAbsoluteSecurityDescriptor, // Converted from the above string.
            -1,
            nullptr,
            nullptr,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IDENTIFY,
            NULL,
            EOAC_NONE,
            nullptr);
        if (FAILED(hResult))
        {
            return FALSE;
        }
    }
    //==== CoInitializeEx and Security for WRL LE Functionality ====//

    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    char filename[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, cmdInfo.m_strFileName, -1, filename, sizeof (filename), NULL, 0);

    FILE *fPatch;
    if (fopen_s(&fPatch, filename, "rb"))
    {
        MessageBox(NULL, L"Failed to open the patch file", L"Error", MB_OK);
        return (FALSE);
    }
    // read private key
    fseek(fPatch, 0, SEEK_END);
    DWORD fileSize = ftell(fPatch);
    rewind(fPatch);
    LPBYTE pPatch = (LPBYTE)new BYTE[fileSize];

    DWORD readBytes = (DWORD)fread(pPatch, 1, fileSize, fPatch);
    fclose(fPatch);

    // Lets check if BT is plugged in
    HANDLE hRadio = INVALID_HANDLE_VALUE;
    BLUETOOTH_FIND_RADIO_PARAMS params = {sizeof(BLUETOOTH_FIND_RADIO_PARAMS)};
    HBLUETOOTH_RADIO_FIND hf = BluetoothFindFirstRadio(&params, &hRadio);
    if (hf == NULL)
    {
        MessageBox(NULL, L"Bluetooth radio is not present or not functioning.  Please plug in the dongle and try again.", L"Error", MB_OK);
        delete[] pPatch;
        return FALSE;
    }
    CloseHandle(hRadio);
    BluetoothFindRadioClose(hf);

    // This application supports Microsoft on Win10, Win8 or BTW on Win7.
    CWsOtaUpgradeDlg dlg(pPatch, fileSize);

    dlg.m_bWin10 = IsOSWin10();
    dlg.m_bWin8 = IsOSWin8();

    //====================================
    CDeviceSelectAdv dlgDeviceSelectAdv;

    INT_PTR nResponse;

    if (dlg.m_bWin10)
    {
        dlgDeviceSelectAdv.m_bth.ullLong = 0;
        nResponse = dlgDeviceSelectAdv.DoModal();

        if ((nResponse == IDOK))// && (dlgDeviceSelect.m_bth.ullLong != 0))
        {
            OutputDebugStringW(L"Found device from scan");
            dlg.SetParam(&dlgDeviceSelectAdv.m_bth);
        }
        else
        {
            delete[] pPatch;
            return FALSE;
        }
    }

    //====================================

    // This application supports Microsoft on Win8 or BTW on Win7.
    if (dlg.m_bWin10)
    {
        dlg.m_bWin10 = TRUE;

        guidSvcWSUpgrade = GUID_OTA_FW_UPGRADE_SERVICE;
        guidCharWSUpgradeControlPoint = GUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT;
        guidCharWSUpgradeData = GUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA;
    }
    else if (dlg.m_bWin8)
    {
        dlg.m_bWin8 = TRUE;
        if ((hLib = LoadLibrary(L"BluetoothApis.dll")) == NULL)
        {
            MessageBox(NULL, L"Failed to load BluetoothAPIs library", L"Error", MB_OK);
            delete[] pPatch;
            return FALSE;
        }
        guidCharWSUpgradeControlPoint   = GUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT;
        guidCharWSUpgradeData           = GUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA;
    }
    else if (IsOSWin7())
    {
        dlg.m_bWin8 = FALSE;

        TCHAR BtDevFullPath[MAX_PATH + 1] = { '\0' };
        CBTFullLibPath LibPath;
        LibPath.GetFullInstallPathOf(L"BTWLeApi.Dll", BtDevFullPath, MAX_PATH);
        if ((hLib = LoadLibrary(BtDevFullPath)) == NULL)
        {
            MessageBox(NULL, L"Broadcom Blueototh profile pack for Windows (BTW) has to be installed", L"Error", MB_OK);
            delete[] pPatch;
            return FALSE;
        }
        BtwGuidFromGuid(&guidCharWSUpgradeControlPoint, &GUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT);
        BtwGuidFromGuid(&guidCharWSUpgradeData, &GUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA);
    }
    else
    {
        MessageBox(NULL, L"This application can run on Windows 8 or on Windows 7 with Broadcom Bluetooth profile pack for Windows (BTW) installed", L"Error", MB_OK);
        delete[] pPatch;
        return FALSE;
    }

    // Activate "Windows Native" visual manager for enabling themes in MFC controls
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    SetRegistryKey(_T("Broadcom"));

    m_pMainWnd = &dlg;
    nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
    }
    else if (nResponse == IDCANCEL)
    {
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
        TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
    }
    if (hLib != NULL)
        FreeLibrary(hLib);

    delete[] pPatch;

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}


void ods(char * fmt_str, ...)
{
    char buf[1000] = {0};
    va_list marker = NULL;

    va_start (marker, fmt_str );
    vsnprintf_s (buf, sizeof(buf), _TRUNCATE, fmt_str, marker );
    va_end  ( marker );

    strcat_s(buf, sizeof(buf), "\n");
    OutputDebugString(CA2W(buf));
}

void BdaToString (PWCHAR buffer, BLUETOOTH_ADDRESS *btha)
{
    WCHAR c;
    for (int i = 0; i < 6; i++)
    {
        c = btha->rgBytes[5 - i] >> 4;
        buffer[2 * i    ] = c < 10 ? c + '0' : c + 'A' - 10;
        c = btha->rgBytes[5 - i] & 0x0f;
        buffer[2 * i + 1] = c < 10 ? c + '0' : c + 'A' - 10;
    }
}


void UuidToString(LPWSTR buffer, size_t buffer_size, GUID *uuid)
{
    // Example {00001101-0000-1000-8000-00805F9B34FB}
    _swprintf_p(buffer, buffer_size, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", uuid->Data1, uuid->Data2,
            uuid->Data3, uuid->Data4[0], uuid->Data4[1], uuid->Data4[2], uuid->Data4[3],
            uuid->Data4[4], uuid->Data4[5], uuid->Data4[6], uuid->Data4[7]);
}
