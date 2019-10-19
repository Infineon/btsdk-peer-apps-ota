
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
