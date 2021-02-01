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

// HelloClientDlg.h : header file
//

#pragma once
#include "BtInterface.h"

// Windows 7 definitions
typedef DWORD (WINAPI *FP_BtwGattRegister)
(
    const BLUETOOTH_ADDRESS    *pAddress,
    PFN_GATT_CONNECT_CALLBACK  pfnConnectCallback,
    PFN_GATT_COMPLETE_CALLBACK pfnCompleteCallback,
    PFN_GATT_REQUEST_CALLBACK  pfnRequestCallback,
    PVOID                      pRefData,
    HANDLE                     *phReg
);

typedef void (WINAPI *FP_BtwGattDeregister)(HANDLE hReg);

typedef DWORD (WINAPI *FP_BtwGattWriteDescriptor)
(
    HANDLE                      hReg,               // handle of the application registration
    BLUETOOTH_ADDRESS           *pAddress,          // address of the device to read
    const GUID                  *pGuidService,      // guid of the service
    DWORD                       dwServiceInstance,  // instance of the service (most likely 0)
    const GUID                  *pGuidChar,         // guid of the char
    DWORD                       dwCharInstance,     // instance of the char (most likely 0)
    const GUID                  *pGuidDescr,        // guid of the char descriptor
    DWORD                       security,           // required security
    BTW_GATT_VALUE              *pValue,            // pointer to the result, can be NULL if completion is used
    BOOL                        bWait,              // if False returns right away
    LPVOID                      pRefData            // pointer returned in the completion
);

typedef DWORD (WINAPI *FP_BtwGattReadCharacteristic)
(
    HANDLE                      hReg,               // handle of the application registration
    BLUETOOTH_ADDRESS           *pAddress,          // address of the device to read
    const GUID                  *pGuidService,      // guid of the service
    DWORD                       dwServiceInstance,  // instance of the service (most likely 0)
    const GUID                  *pGuidChar,         // guid of the char
    DWORD                       dwCharInstance,     // instance of the char (most likely 0)
    DWORD                       security,           // required security
    BTW_GATT_VALUE              *pValue,            // pointer to the result, can be NULL if completion is used
    BOOL                        bWait,              // if False returns right away
    LPVOID                      pRefData            // pointer returned in the completion
);

typedef DWORD (WINAPI *FP_BtwGattWriteCharacteristic)
(
    HANDLE                      hReg,               // handle of the application registration
    BLUETOOTH_ADDRESS           *pAddress,          // address of the device to read
    const GUID                  *pGuidService,      // guid of the service
    DWORD                       dwServiceInstance,  // instance of the service (most likely 0)
    const GUID                  *pGuidChar,         // guid of the char
    DWORD                       dwCharInstance,     // instance of the char (most likely 0)
    DWORD                       security,           // required security
    BTW_GATT_VALUE              *pValue,            // pointer to the result, can be NULL if completion is used
    BOOL                        bWait,              // if False returns right away
    LPVOID                      pRefData            // pointer returned in the completion
);


class CBtWin7Interface : public CBtInterface
{
public:
    CBtWin7Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext);
    virtual ~CBtWin7Interface();

    BOOL Init();
    BOOL GetDescriptorValue(USHORT *DescriptorValue);
    BOOL SetDescriptorValue(USHORT Value);
    BOOL SendWsUpgradeCommand(BYTE Command);
    BOOL SendWsUpgradeCommand(BYTE Command, USHORT sParam);
    BOOL SendWsUpgradeCommand(BYTE Command, ULONG lParam);
    BOOL SendWsUpgradeData(BYTE *Data, DWORD len);

private:
    HANDLE m_hReg;
};
