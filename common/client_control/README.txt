QT based "WICED Client Control" application
===========================================

This application was developed using QT IDE. 

Supported OS: 
- Windows: x86, x64 (Windows 7, Windows 8.1, Windows 10)
- Linux: x64 (64 bit Ubuntu 16.04)
- Linux: x86 (32-bit Fedora 23)

Version: 
- QT version 5.7

Instructions for running the application
----------------------------------------
To run the built-in "Client Control" sample application, follow these steps -
1. On Windows PC, double click "ClientControl.exe" in 'Windows' folder.
2. On Linux PC, execute the script "RunClientControl.sh" in the Linux folder.
3. Plug the WICED Studio Evaluation Board into the computer using a USB cable
4. On Linux PC, additional step may be required to enable serial port access, see 
   the WICED-207xx-BT-Quick-Start-Guide.pdf in 'doc' folder. 
   (Appendix C: Connecting to Linux Platforms).
5. In the Eclipse IDE, double-click on the desired target in 'Make Target' window to build
   and download an embedded application to the WICED Studio evaluation board.   
6. In the "Client Control" UI, select the serial (COM) port for the WICED Studio Evaluation 
   Board and open the port.
7. Once the "Client Control" app is able to communicate with the embedded Bluetooth application, 
   use the "Client Control" UI to perform Bluetooth operations.
8. Bluetooth application and protocol traces will be displayed in BTSpy.exe (wiced_tools\BTSpy)
   
Instructions for building the application
-----------------------------------------
To make a change to the sample "Client Control" application and rebuild the application, 
follow these steps -
1. On Linux OS, install gcc, gcc-c++, kernel-devel-$(uname -r), "Development Tools", 
   "Development Libraries", libGLU-devel, before installing QT Creator
2. Download installer for QT Creator from https://www.qt.io/download/
3. On Windows OS, select the option for MinGW 32 bit compiler
4. Open the QT project file "ClientControl.pro" with QT Creator
5. Build the QT project
   
User Interface
--------------

Device Manager:
++++++++++++++
- Open port
  This button is used to open or close serial port used by by WICED Eval board.
  Opening the serial port is the first step for using the application. 
  Select the "Baud rate" and "Flow Control" options as appropriate for embedded app.
- Pair-able
  This option enables or disables pairing from peer device.
- Connectable
  This option enables or disables connectablity from peer device.
- Discoverable
  This option enables or disables discoverability from peer device.
- Reset Device  
  Close the serial port and rest the device. The embedded application may need to to be 
  downloaded again.
- BLE Discovery
  Search for BLE devices.
- BR/EDR Discovery
  Search for BR/EDR devices.
- Unbond
  Un-pair selected device. Paired devices will show 'link' icon.
- Patch file download
  Browse for a patch (.hcd) file containing embedded application and download it on the 
  WICED Eval board.   The Maketarget should contain DIRECT_LOAD=1 to create .hcd file. 
  For example "hci_hid_device-BCM920706_P49 DIRECT_LOAD=1 build". The .hcd file is 
  created in apps\build folder for the WICED platform. The 'Local BD Address' will be 
  used during the patch file download. 
  Before clicking on 'Download' button, close the serial port if open and select the desired port
  number in the drop down box. Prepare the WICED Eval board using the reset/recovery procedure.
  Then click the 'Download' button.
- Reset Device
  Reset the device. After device reset, embedded WICED application may need to be downloaded again.
  
AV Source (A2DP Source):
+++++++++++++++++++++++
Setup: Local apps - 20706-A2 'watch'. Peer device - headset, speaker, car-kit
- Connect
  Connect to selected BR/EDR device to create A2DP connection. Peer selected device 
  should be audio sink capable device such as headset, speaker, or car-kit. Select 
  the 'Media' and 'Mode' to stream before creating A2DP connection.  
- Disconnect
  Disconnect existing A2DP connection from selected BR/EDR.

AVRC TG (AVRCP Target):
+++++++++++++++++++++++
Setup: Local apps - 20706-A2 'watch', Peer device - headset, speaker, car-kit
- Connect
  Connect to selected BR/EDR device to create AVRCP connection. Peer selected device
  should support AVRCP Controller role, such as headset, speaker, or car-kit. After 
  creating A2DP Source connection, the peer device may automatically create AVRCP 
  connection. This option is available only "PTS_TEST_ONLY" compiler option is enabled
  in the embedded app.
- Disconnect
  Disconnect existing AVRCP connection from selected BR/EDR. This option is available
  only "PTS_TEST_ONLY" compiler option is enabled in the embedded app.
- Play, Pause, Stop, Forward, Back
  These buttons sent 'player status' and 'track info' attribute notifications to the peer device.
  The peer device should support AVRCP 1.3 or higher. Note that these buttons are not controlling
  the media in AV Source UI, and are meant for demonstrating the API usage. The forward/back
  buttons will change the current track of the built in play list displayed in the AVRCP UI. 
  Note that the built-in playlist is for display only and does not match the streaming media
  in A2DP Source. The current track is displayed by peer device supporting AVRCP Controller 1.3 
  or higher. 
- Shuffle/Repeat
  These controls change AVRCP player settings and are used when peer device supports AVRCP
  Controller 1.3 or higher. These controls do not change media in A2DP source UI.
- Volume
  This control sets or displays the absolute volume. (Used when peer device supports AVRCP
  Controller 1.4 or higher). 
 
AV Sink:
++++++++
Not implemented in the current release
 
AVRC CT (AVRCP Controller):
++++++++++++++++++++++++++
Setup: Local apps - 20706-A2 'watch', Peer device - iPhone or Android phone
Discover phone from "BR/EDR Discovery" control and pair with phone
- Connect
  Create AVRC connection with peer device. Note that some peer devices may not
  support AVRC CT connection without A2DP Sink profile
- Disconnect
  Disconnect AVRCP connection with peer device.
- Repeat, Shuffle
  These controls change AVRCP player settings and are used when peer device supports AVRCP
  Controller 1.3 or higher. 
- Volume
  This control displays the absolute volume. (Used when peer device supports AVRCP
  Controller 1.4 or higher). 
- AMS and ANCS
  For AMS and ANCS, reset the board and download the 'watch' app. From 'GATT' tab
  Start Advertisements. From iPhone app such as 'Light Blue', discover the 'watch' app
  and pair. Play media on iPhone and control it with the AVRC CT controls. Incoming 
  calls and messages will be displayed on the ANCS controls.
  
Audio Gateway:
++++++++++++++
Setup: Local apps - 20706-A2 'hci_ag_plus', Peer device - headset, speaker, car-kit supporting hands-free profile
- Connect
  Connect to peer device supporting HF profile
- Disconnect
  Disconnect with peer device
- Audio Connect
  Open SCO audio channel with peer device
  
Hands-free:
+++++++++++
Setup: Local apps - 20706-A2 'hci_handsfree_plus', Peer device - phone or device supporting audio gatewau profile
- Connect
  Connect to peer device supporting AG profile
- Disconnect
  Disconnect with peer device
- Connect Audio
  Open SCO audio channel with peer device
- Hangup, Answer, Redial, etc.
  These controls send the HF profile AT commands for various operations.
  
Serial Port Profile:
+++++++++++++++++++
Setup: Local apps - 20706-A2 'hci_handsfree_plus' or 'hci_ag_plus', Peer device - any device supporting SPP server role
- Connect
  Connect to SPP server
- Disconnect
  Disconnect from SPP server
- Send
  Send characters typed in edit control or file to peer device
- Receive
  Receive data in edit control (first 50 bytes) or receive and save data to file
  
HID Host:
+++++++++
Setup: Local apps - 20706-A2 'hci_hid_host', Peer device - BT keyboard or mouse
- Connect
  Connect to HID device such as keyboard or mouse. After connection, the input from 
  HID device will be displayed in BTSpy.exe
- Disconnect
  Disconnect from peer device
- Get Desc
  Get descriptor from the peer device. The output is displayed on BTSpy.exe traces
- HID Protocol
  Set the protocol as Report mode or Boot mode 
  
HID Device:
++++++++++
Setup: Local apps - 20706-A2 'hci_hid_device' or BR-EDR or 'hci_ble_hid_dev' for BLE HOGP, 
Peer device - Windows PC or any HID host
- Enter Pairing Mode
  Sets the local device to pair-able
- Connect
  Connect with HID host
- Send Key
  Sends the specified key from drop down with options such as button up, Caps lock, etc.
- Send Report
  Send report for Interrupt or Control channel. 
  
GATT:
+++++
Setup: Local apps - 20706-A2 'watch'. Peer device - BLE device or iPhone Light Blue app, Android BLE app, etc.
GATT controls are provided for advertisements, discovering services, connecting to GATT server, read/write values of handle,
discover characteristics and descriptors of handles.

HomeKit:
++++++++
Setup: Local apps - btle_homekit2_lightbulb, blte_homekit2_lock, peer device : iPhone, iOS 10.2, My Home app
Note: This app is available only for Apple MFI licensees
Sample applications are provided for 'Lock' and 'Light Bulb' control. 

iAP2:
+++++
Setup: Local apps - hci_iap2_spp, peer device: iPhone, iOS 10.2 or SPP server capable device
Note: This app is available only for Apple MFI licensees
iAP2 app sends and receive data over RFCOMM for iOS devices and using SPP for non iOS devices. For UI description,
see "Serial Port Profile" above.

BLE Serial Gatt:
++++++++++++++++
Setup: Local apps - serial_gatt_serivce, peer device : Windows 10, Android or iPhone running 'peerapps' application 
found in the WICED SDK under 'serial_gatt_serivce' folder. 
This application uses Cypress BLE GATT service to sent and receive data over GATT. This is similar to Serial Port Profile application. 
For UI description, see "Serial Port Profile" above.