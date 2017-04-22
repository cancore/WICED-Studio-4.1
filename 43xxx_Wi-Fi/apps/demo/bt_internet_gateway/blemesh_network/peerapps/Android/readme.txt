
Introduction
------------
As part of the BIG Mesh Ligting solution, there are 2 apps namely MeshService and MeshLighting:

    MeshService.apk : is an Andriod service , which manages all the network layer information related to mesh network.
    This app manages functionality such as creating a network , adding devices and provides data pipes to talk to mesh devices

    MeshLighting.apk : is an Android ligthing app which runs on top of MeshService. This app allows users to create a Lighting Network, add
    lights and control them for "inside home" and "outside home" usecase scenarios.


Installation and Usage
----------------------
1>  Install MeshService.apk and MeshLighting.apk on an Android phone and reboot the phone
    (The preferred version would be Android 5.0 or 6.0)
3>  Install WICED SMART SDK and download blemesh app on to 20737 tag 03 boards (These are the mesh lighting tags)
4>  Build and run blemesh_network demo application located under bluetooth_internet_gateway on WICED SDK
5>  When WICED BIG mesh powers up, the device will connect to a configured WiFi AP, and is ready to be provisioned
6>  Using the Android Ligthing App, create a lighting network and start adding Lights to the network
    a>  Create a room and add lights to the room
    b>  On getting added (provisioned), each light will beep twice
    c>  The first light that is being provisioned shall beep thrice. The third beep ensures a proxy connection is available on the network
7>  Click Settings icon on the home screen and select the option "Add BIG". This will provision BIG and add this device to the mesh network
    a> When prompted, user shall select the BD Address of BIG and add BIG's IP address
8>  "Inside home" usecase
    a> The MeshLighting app shall automatically assume that the user is "inside home". User can control brightness of individaul lights or an entire room.
    b> User can also add and remove lights/rooms inside home
9>  "Outside home" usecase
    a>  Once BIG is provisioned, ensure that the phone is also connected to the same WiFi AP as that of BIG
    b>  The app settings have an option to toggle between "Home" and "Away" modes.
    c>  When user selects "Away", the app will set the current transport to REST and disconnect an existing proxy connection. The light bulbs shall be managed via REST APIs.
    d>  User can control brightness of rooms/lights; however, user is not allowed to add/delete lights and room outside home
    e>  Similarly, BIG mesh device cannot be deleted in "Away" mode

Build instructions
------------------
Android Application source and build procedure for the app shall be made available in a future release.

Limitations
-----------
The below limitations shall be fixed in the next version of the application
1>  Deletion of a created network is not supported
2>  Creation of groups within a room is not supported
