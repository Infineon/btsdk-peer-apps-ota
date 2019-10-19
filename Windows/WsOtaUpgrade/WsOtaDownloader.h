#include "BtInterface.h"

#define POLYNOMIAL			    0x04C11DB7
#define WIDTH                   (8 * sizeof(unsigned long))
#define TOPBIT                  (1 << (WIDTH - 1))
#define INITIAL_REMAINDER	    0xFFFFFFFF
#define FINAL_XOR_VALUE		    0xFFFFFFFF
#define REFLECT_DATA(X)	        ((unsigned char) reflect((X), 8))
#define REFLECT_REMAINDER(X)	((unsigned long) reflect((X), WIDTH))
#define CHECK_VALUE			    0xCBF43926

unsigned long crcSlow(unsigned long  crc32, unsigned char const message[], int nBytes);
unsigned long crcComplete(unsigned long crc32);

class WSDownloader
{
public:
    WSDownloader(CBtInterface *pBtInterface, LPBYTE pPatch, DWORD dwPatchSize, HWND hWnd);
    virtual ~WSDownloader();

    void Start();
    void ProcessEvent(BYTE Event);

    void TransferData();
    void SetMTU(DWORD mtu);
    HANDLE m_hThread;
    enum
    {
        WS_UPGRADE_CONNECTED,
        WS_UPGRADE_RESPONSE_OK,
        WS_UPGRADE_CONTINUE,
        WS_UPGRADE_START_VERIFICATION,
        WS_UPGRADE_RESPONSE_FAILED,
        WS_UPGRADE_ABORT,
        WS_UPGRADE_DISCONNECTED
    } m_event;

    enum
    {
        WS_UPGRADE_STATE_IDLE,
        WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD,
        WS_UPGRADE_STATE_DATA_TRANSFER,
        WS_UPGRADE_STATE_VERIFICATION,
        WS_UPGRADE_STATE_VERIFIED,
        WS_UPGRADE_STATE_ABORTED,
    } m_state;

private:
    BOOL m_bConnected;
    HANDLE m_hEvent;
    HWND m_hWnd;
    DWORD m_offset;
    CBtInterface *m_btInterface;
    BYTE *m_Patch;
    DWORD m_PatchSize;
    DWORD m_crc32;
    DWORD m_connMTU;
};
