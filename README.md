# btsdk-peer-apps-ota

### Overview

This repo contains peer apps for the Over-the-Air upgrade app. Binary and source code is included.

These apps work on:

- Android
- iOS
- Windows

### Instructions
Applications to perform and verify OTA FW upgrade are provided for Windows 10, Android. and iOS platforms. Executables are included and source code is provided.

- Windows Peer App: WsOTAUpgrade.exe
- Android Peer App: LeOTAApp (app-debug.apk)
- iOS Peer App: OtaUpgrade

Windows and Android applications can be used to perform and verify secure (signed) and unsecured (unsigned) OTA upgrade. Both applications accept a secure (signed) or unsecured (unsigned) OTA binary images as input (.ota.bin & .ota.bin.signed). iOS app can perform only unsecure OTA at this time.

Note: Android OTA SPP app (OTASPPApp) is for 20721 platform apps such as headset\_speaker.

To demonstrate the app, work through the following steps:

##### To perform OTA upgrade:
1. Plug the WICED eval board into your computer.
2. Build and download the application to the WICED board.
3. The OTA image file (app-name)\_(bsp).ota.bin is generated in the (app-name)/build/(bsp)/Debug folder.

Windows:

1. Copy the OTA image .bin file to the wiced\_btsdk/tools/btsdk-peer-apps-ota/Windows/WsOtaUpgrade/Release/(OS) folder.
2. Launch the appropriate version of WsOtaUpgrade.exe for your OS.
3. Use the WsOtaUpgrade application to test over the air upgrade. Pass the filename of the OTA binary created in the build as the command-line argument.
    - C:..\> WsOtaUpgrade.exe (app-name)\_(bsp).ota.bin
4. Select the peer device and click the start button to initiate OTA process.

Android and iOS:

1. Use LeOTAApp or OtaUpgrade and select the (app-name)\_(bsp).ota.bin file.
2. Select the peer device and click on start button to initiate OTA process.

##### To perform Secure OTA upgrade:
1. Open a command prompt and go to directory - wiced\_btsdk/tools/btsdk-utils/ecdsa256/bin/(OS)
2. Generate a private/public key pair by running the ecdsa_genkey application.
3. Copy the generated ecdsa256\_pub.c to the application directory. For example:
     - (app-name)/secure/ecdsa256\_pub.c
4. In your application, set makefile variable OTA\_SEC\_FW\_UPGRADE=1
5. Clean and rebuild the application and download to the board.
6. Copy generated (app-name)\_(bsp).ota.bin file in (app-name)/build/(bsp)/Debug folder back to ecdsa256/bin/(OS) directory.
7. Sign the binary .ota.bin file with the public key using ecdsa\_sign application in ecdsa256/bin/(OS) folder. This will generate file (app-name)\_bsp.ota.bin.signed
    - ecdsa\_sign (app-name)\_(bsp).ota.bin

Windows:

1. Copy the (app-name)\_(bsp).ota.bin.signed to the same folder as WsOtaUpgrade.exe for your OS.
2. Launch the appropriate version of WsOtaUpgrade.exe
3. Use the WsOtaUpgrade application to test over the air upgrade. Pass the filename of the OTA binary created in the build as the command-line argument.
     - C:..\> WsOtaUpgrade.exe (app-name)\_(bsp).ota.bin.signed
4. Select the peer device and click on the start button to initiate OTA process.

Android:

1. On an Android phone use the LeOTAApp.
2. Copy the signed OTA image to the Android device.
3. Start the LeOTAApp application on the Android device.
4. Click on 'Tap to select OTA image' and select the (app-name)\_(bsp).ota.bin.signed.
5. Click on 'Tap to select a device' and select the OTA FW Upgrade device.
6. Tap Connect.
7. Tap Upgrade.


Note:
On some older Bluetooth Devices on Windows, the OTA might fail because of an incompatible MTU. To work around this, change the code as below and rebuild the Windows application in ./Windows/WsOtaUpgrade/WsOtaDownloader.cpp:

     * define MAX_MTU 256 // (instead of 512)
