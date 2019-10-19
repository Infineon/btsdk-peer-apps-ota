
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
