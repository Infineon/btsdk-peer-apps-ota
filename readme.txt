 Bluetooth Over the Air (OTA) Firmware Upgrade Peer Applications
 ---------------------------------------------------------------

 Applications to perform and verify OTA FW upgrade are provided for Windows 10,
 Android and iOS platforms. Executable and source code are provided.

 Windows Peer App: WsOTAUpgrade.exe
 Android Peer App: LeOTAApp (app-debug.apk)
 iOS Peer App: OtaUpgrade

 Windows and Android applications can be used to perform and verify secure
 (signed) and unsecured (unsigned) OTA upgrade. Both applications accept a secure
 (signed) or unsecured (unsigned) OTA binary images as input
 (*.ota.bin & *.ota.bin.signed). iOS app can perform only unsecure OTA at this time.

 Note: Android OTA SPP app (OTASPPApp) is for 20721 platform apps such as headset_pro
       and btspeaker_pro only.

 To demonstrate the app, work through the following steps:

 To perform OTA upgrade -
 1. Plug the WICED eval board into your computer.
 2. Build and download the application to the WICED board.
 3. The OTA image file <app-name_bsp>.ota.bin is generated in
    <app-name>\build\<bsp>\Debug folder.
 Windows OS -
 4. Copy the OTA image .bin file to
    wiced_btsdk\tools\btsdk-peer-apps-ota\Windows\WsOtaUpgrade\Release\<OS>\ folder.
 5. Launch appropriate version of WsOtaUpgrade.exe for your OS.
 6. Use WsOtaUpgrade application to test over the air upgrade.  Pass the filename
    of the ota binary created in the build as the command-line argument.
    C:..\> WsOtaUpgrade.exe <app-name_bsp>.ota.bin
 7. Select the peer device and click on start button to initiate OTA process.
 Android and iOS Device -
 4. Use LeOTAApp or OtaUpgrade and select the <app-name_bsp>.ota.bin file.
 5. Select the peer device and click on start button to initiate OTA process.

 To perform Secure OTA upgrade -
 1. Open a command prompt and go to directory -
    wiced_btsdk\tools\btsdk-utils\ecdsa256\bin\<OS>
 2. Generate private/public key pair by running ecdsa_genkey application.
 3. Copy generated ecdsa256_pub.c to the application directory. For example:
    <app-name>\secure\ecdsa256_pub.c
 4. In your application, set makefile variable OTA_SEC_FW_UPGRADE=1
 5. Clean and rebuild the application and download to the board.
 6. Copy generated <app-name_bsp>.ota.bin file back to ecdsa256\bin\<OS> directory.
 7. Sign the binary .ota.bin file with the public key using ecdsa_sign application
    in ecdsa256\bin\<OS> folder.
    > ecdsa_sign <app-name_bsp>.ota.bin
    This will generate file <app-name_bsp>.ota.bin.signed
 Android Device -
   8. On Android phone use LE OTA App.
   9. Copy signed ota image to your Android device.
   10. Start LEOTAApp application on Android device.
   11. Click on <Tap to select OTA image> and select the <app-name_bsp>.ota.bin.signed.
   12. Click on <Tap to select a device> and select OTA FW Upgrade device.
   13. Tap on Connect button.
   14. Tap on Upgrade.
 Windows -
   8. Copy the <app-name_bsp>.ota.bin.signed to the same folder as WsOtaUpgrade.exe
      for your OS.
   9. Launch appropriate version of WsOtaUpgrade.exe
   10. Use WsOtaUpgrade application to test over the air upgrade.
       Pass the filename of the ota binary created in the build as
       the command-line argument.
       C:..\> WsOtaUpgrade.exe <app-name_bsp>.ota.bin.signed
   11. Select the peer device and click on start button to initiate OTA process.

 Note:
 On some older Bluetooth Devices on Windows, the OTA might fail because of improper
 MTU. To work around this, change the code as below and rebuild the Windows application.
 #define MAX_MTU 256 // (instead of 512)
 in .\Windows\WsOtaUpgrade\WsOtaDownloader.cpp
