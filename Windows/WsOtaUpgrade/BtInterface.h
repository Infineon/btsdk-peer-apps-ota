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

// BtInterface.h : header file
//

#pragma once


typedef enum {
    OSVERSION_WINDOWS_7 = 0,
    OSVERSION_WINDOWS_8,
    OSVERSION_WINDOWS_10
}tOSVersion;


class CBtInterface
{
public:
    CBtInterface(BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext, tOSVersion osversion)
    {
        m_bth = *bth;
        m_hLib = hLib;
        m_NotificationContext = NotificationContext;
        m_bWin8 = (osversion == OSVERSION_WINDOWS_8) ? TRUE : FALSE;
        m_bWin10 = (osversion == OSVERSION_WINDOWS_10) ? TRUE : FALSE;
    };

    virtual BOOL Init() = NULL;
    virtual BOOL GetDescriptorValue(USHORT *DescriptorValue) = NULL;
    virtual BOOL SetDescriptorValue(USHORT Value) = NULL;
    virtual BOOL SendWsUpgradeCommand(BYTE Command) = NULL;
    virtual BOOL SendWsUpgradeCommand(BYTE Command, USHORT sParam) = NULL;
    virtual BOOL SendWsUpgradeCommand(BYTE Command, ULONG lParam) = NULL;
    virtual BOOL SendWsUpgradeData(BYTE *Data, DWORD len) = NULL;

    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    LPVOID m_NotificationContext;
    BOOL m_bWin8;
    BOOL m_bWin10;
    BOOL m_bSecure;
    tOSVersion m_osversion;
};
