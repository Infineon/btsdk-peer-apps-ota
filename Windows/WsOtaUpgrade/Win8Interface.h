
// Win8Interface.h : header file
//

#pragma once
#include "BtInterface.h"

class CBtWin8Interface : public CBtInterface
{
public:
    CBtWin8Interface (BLUETOOTH_ADDRESS *bth, HMODULE hLib, LPVOID NotificationContext);
    virtual ~CBtWin8Interface();

    BOOL Init();
    BOOL GetDescriptorValue(USHORT *DescriptorValue);
    BOOL SetDescriptorValue(USHORT Value);
    BOOL SendWsUpgradeCommand(BYTE Command);
    BOOL SendWsUpgradeCommand(BYTE Command, USHORT sParam);
    BOOL SendWsUpgradeCommand(BYTE Command, ULONG lParam);
    BOOL SendWsUpgradeData(BYTE *Data, DWORD len);

    void RegisterNotification();
    BTW_GATT_VALUE *HandleNotification(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter);

    BOOL m_bConnected;
private:
    HANDLE m_hDevice;
    BTH_LE_GATT_SERVICE         m_service;
    BTH_LE_GATT_DESCRIPTOR      m_descrClientConfig;
    BLUETOOTH_GATT_EVENT_HANDLE m_pEventHandle;
    BTH_LE_GATT_CHARACTERISTIC  m_charWsUpgradeControlPoint;
    BTH_LE_GATT_CHARACTERISTIC  m_charWsUpgradeData;
};
