========================================================================
    CONSOLE APPLICATION : WsOtaUpgrade Project Overview
========================================================================

This application is performing firmware upgrade of the WICED Bluetooth
device over the air.  It uses a GATT Vendor Specific Service to send
commands and data packets, and to receive status notifications.

WARNING:  If EEPROM or Serial Flash installed on the device is less then 64
KBytes, the memory after the upgrade might be corrupted.  Use the Recover
procedure described in the README.md file from any BTSDK application project
to continue using the device.

Usage:
------
WsOtaUpgrade.exe <image.bin>

or

WsOtaUpgrade.exe /file <image.bin> [/peername <name>] [/secure] [/nonsecure] [/automation]

The <image.bin> file is built when application is compiled for
applications that support OTA. It is located in Debug folder of the app.
(For example mainapp_download.ota.bin)

The patch file <image.bin> name is mandatory, all others are optional.

The app will show peer app with name "OTA_FW_UPGRADE" or name specified in /peername arg as first in the list (not case sensitive).

If /secure is specified, only secure OTA is supported.

if /nonsecure is specified, only non-secure OTA is supported.

By default, both secure or non-secure are supported.

if /automation is specified, the entire OTA is automated.

In the first device selection dialog, if peer app with name "OTA_FW_UPGRADE" or name specified in /peername arg is found, then it will go to next step.
It will wait for 60 sec before timeout occurs if no such device is found.

The next OTA dialog will finish as soon as there is success or failure. If OTA is in progress and its more than 8 mins, then timeout will occur and OTA will fail.

The success/failure return code is returned by the exe. Return code 0 is a success. Failure codes are as below defined in WsOtaUpgrade.h
#define ERROR_GEN_FAIL -1  // General failure
#define ERROR_SECURE_NONSECURE -2 // Command line specified both secure and non secure OTA
#define ERROR_PATCH_FILE -3 // patch file was not found
#define ERROR_BT_RADIO -4 // Local BT device on the computer was not detected
#define ERROR_NO_OTA_SUPPORT -5 // Peer device did not support OTA
#define ERROR_NO_OTA_SECURE_SUPPORT -6 // Peer device did not support secure OTA
#define ERROR_NO_OTA_NON_SECURE_SUPPORT -7 // Peer device did not non secure support OTA

Notes:
------

The protocol for the upgrade over air is rather simple.

On startup the application reads the FW image file.  Then the
application searches for a paired device that exposes the Vendor
Specific WICED Upgrade service.

The Upgrade service exposes Control Point characteristic which the
application can use to send commands and receive notifications, and
a Data characteristic which the application uses to send chunks of data
to the device.

To start the upgrade the application writes a one byte
WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD command.  And the device replies with
WS_UPGRADE_RESPONSE_OK.

Next the application writes WS_UPGRADE_COMMAND_DOWNLOAD with 2 bytes
of the patch length.  After the application receives an OK response it starts
writing 20 byte chunks of data.  After all the data has been
completed, the application sends the WS_UPGRADE_COMMAND_VERIFY command
passing a 32 bit checksum.

At that time device verifies that the data has been successfully committed
to EEPROM or SFLASH and acknowledges with WS_UPGRADE_RESPONSE_OK if
success or WS_UPGRADE_RESPONSE_FAILED if not all bytes were received
or if checsum does not match.  If the success case the device automatically
reboots after making the downloaded image active.
