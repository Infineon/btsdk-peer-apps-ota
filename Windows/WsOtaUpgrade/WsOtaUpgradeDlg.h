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

// WsOtaUpgradeDlg.h : header file
//

#pragma once
#include "Win7Interface.h"
#include "Win8Interface.h"
#include "Win10Interface.h"
#include "WsOtaDownloader.h"
#include "afxcmn.h"
#include "afxwin.h"

#define WM_CONNECTED                    (WM_USER + 101)
#define WM_NOTIFIED                     (WM_USER + 102)
#define WM_PROGRESS                     (WM_USER + 103)
#define WM_DEVICE_CONNECTED             (WM_USER + 104)
#define WM_DEVICE_DISCONNECTED          (WM_USER + 105)

// CWsOtaUpgradeDlg dialog
class CWsOtaUpgradeDlg : public CDialogEx
{
// Construction
public:
    CWsOtaUpgradeDlg(LPBYTE pPatch, DWORD dwPatchSize, CWnd* pParent = NULL);	// standard constructor
    virtual ~CWsOtaUpgradeDlg();

// Dialog Data
    enum { IDD = IDD_WS_UPGRADE_DIALOG };

    BOOL m_bWin8;
    BOOL m_bWin10;
//    BLUETOOTH_ADDRESS m_bth;
    HMODULE m_hLib;
    CBtInterface *m_btInterface;
    BLUETOOTH_ADDRESS m_bth;

    void SetParam(BLUETOOTH_ADDRESS *bth);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
    int    numNotifies;
    HANDLE m_hDevice;
    CComboBox m_cbDevices;
    BOOL   GetDeviceList();
    int m_numDevices;
    WSDownloader *m_pDownloader;
    LPBYTE m_pPatch;
    DWORD m_dwPatchSize;

// Implementation
protected:
    HICON m_hIcon;


    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
    virtual void PostNcDestroy();
    LRESULT OnConnected(WPARAM bConnected, LPARAM lparam);
    LRESULT OnNotified(WPARAM op, LPARAM lparam);
    LRESULT OnProgress(WPARAM completed, LPARAM total);
    LRESULT OnDeviceConnected(WPARAM Instance, LPARAM lparam);
    LRESULT OnDeviceDisconnected(WPARAM Instance, LPARAM lparam);
public:
    afx_msg void OnBnClickedStart();
    afx_msg void OnSelectDevice();
    CProgressCtrl m_Progress;
};
