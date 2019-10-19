 Bluetooth Over the Air (OTA) Firmware Upgrade Peer Applications
 ---------------------------------------------------------------

  Applications to perform and verify OTA FW upgrade are provided for Windows 10 and Android platforms
  Executable and source code are provided
  Windows Peer App: WsOTAUpgrade.exe
  Android Peer App: LeOTAApp (app-debug.apk)
  iOS Peer App: OtaUpgrade
  Windows and Android applications can be used to perform and verify secure (signed) and unsecured (unsigned) OTA upgrade
  Both applications accept a secure (signed) or unsecured (unsigned) OTA binary images as input (*.ota.bin & *.ota.bin.signed)
  iOS app can perform only unsecure OTA at this time.

  Note: Android OTA SPP app (OTASPPApp) is for 20721 platform apps such as headset_pro and btspeaker_pro only.

 To demonstrate the app, work through the following steps.

 To perform OTA upgrade see steps below -
 1. Plug the WICED eval board into your computer
 2. Build and download the application (to the WICED board)
 3. Copy the <app-name>.ota.bin to the same folder as WsOtaUpgrade.exe for your OS.
 4. If using IDE, create an application "Utils OTA Peer Apps".
 5. If using command line, git clone repo named "btsdk-peer-apps-ota".
 Windows OS -
 6. Launch appropriate version of WsOtaUpgrade.exe located in <OTA_Peer_Apps>\ota_firmware_upgrade\Windows\WsOtaUpgrade\Release folder
 7. Use WsOtaUpgrade application to test over the air upgrade.  Pass the filename of the ota binary created in the build as
    the command-line argument C:..\> WsOtaUpgrade.exe <app-name>.ota.bin
 8. Select the peer device and click on start button to initiate OTA process
 Android and iOS Device -
 6. Use LeOTAApp or OtaUpgrade and select the <app-name>.ota.bin file
 7. Select the peer device and click on start button to initiate OTA process

 To perform Secure OTA upgrade -
 1. Change the application makefile variable OTA_SEC_FW_UPGRADE=1
 2. If using IDE, create an application "Utils BTSDK".
 3. If using command line, git clone repo named "btsdk-utils".
 4. Open a Command Prompt and go directory to -
    Utils_BTSDK\<OS>\ecdsa256\bin
 5. Generate private/public key pair by running ecdsa_genkey application
 6. Copy generated ecdsa256_pub.c to the application directory. For example
    <app>\secure\ecdsa256_pub.c
 5. Rebuild the project and download to the board.
 6. Copy generated <app-name>.ota.bin file back to ecdsa256\bin (or ecdsa256\<OS>) directory.
 7. Sign the binary .ota.bin file with the public key using ecdsa_sign application in ecdsa256\bin (or ecdsa256\<OS>) folder
      >ecdsa_sign.exe <app-name>.ota.bin
 Android Device -
   8. On Android phone use LE OTA App
   9. Copy signed ota image to your Android device
   10. Start LEOTAApp application on Android device
   11. Click on <Tap to select OTA image> and select the <app-name>.ota.bin.signed that you copied in step 9.
   12. Click on <Tap to select a device> and select OTA FW Upgrade device
   13. Tap on Connect button
   14. Tap on Upgrade
 Windows -
   8. Copy the <app-name>.ota.bin.signed to the same folder as WsOtaUpgrade.exe for your OS.
   9. Launch appropriate version of WsOtaUpgrade.exe
   10. Use WsOtaUpgrade application to test over the air upgrade.  Pass the filename of the ota binary created in the build as
       the command-line argument C:..\> WsOtaUpgrade.exe <app-name>.ota.bin.signed
   11. Select the peer device and click on start button to initiate OTA process

Note: On some older Bluetooth Devices on Windows, the OTA might fail because of improper MTU. To work around this,
change the code as below and rebuild the Windows application.
#define MAX_MTU 256 // (instead of 512)
in .\Windows\WsOtaUpgrade\WsOtaDownloader.cpp
