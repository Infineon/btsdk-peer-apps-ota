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
WsOtaUpgrade <image.bin>

The <image.bin> file is built when application is compiled for
applications that support OTA. It is located in Debug folder of the app.
(For example mainapp_download.ota.bin)

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
