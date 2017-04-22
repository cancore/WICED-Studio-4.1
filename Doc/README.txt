=====================================================================
Cypress WICED STUDIO SDK Software Development Kit 4.0 - README
=====================================================================

The WICED STUDIO SDK provides systems and APIs needed to build, 
design and implement applications for Wi-Fi, Bluetooth Classic (BR/EDR),
Bluetooth low energy (BLE), and ZigBee devices.

Major features of the WICED STUDIO SDK include ...

- Support for 20706, 20735 and 20719 based Bluetooth platforms.
- Support for 43374, 4339, 43903, 43907, 43909, 4343w, 4356 based Wi-Fi platforms.
- Support for 20729 based ZigBee platform.
- API to access Bluetooth stack including GAP, ATT, GATT, SMP, L2CAP,
  RFCOMM, SDP, A2DP and AVRC in the ROM.
- A generic profile level API that abstracts Bluetooth stack layer API.
- Reference applications for profiles defined by the Bluetooth SIG.
- API and drivers to access on board peripherals like I2C, SPI, UART, 
  ADC, PWM, Keyscan and IR HW blocks.
- Segger J-Link debugger using J-Link GDB server and GCC GDB client.
 
The WICED STUDIO SDK release is structured as follows:
Apps          : Example & Test Applications
Doc           : API & Reference Documentation, Eval Board & Module Schematics
Include       : WICED API, constants, and defaults
Platforms     : Configuration files and information for supported hardware platforms
test          : Tools provided for automation testing
Tools         : Build tools, compilers, programming tools etc.
wiced_tools   : Build tools, compilers, programming tools etc.
WICED-BT      : WICED core Bluetooth components
README.txt    : This file
CHANGELOG.txt : A log of changes for each SDK revision
LICENSE.txt   : Licenses applicable to the SDK & IDE
version.txt   : Version of the WICED STUDIO SDK
 
Getting Started
---------------------------------------------------------------------
If you are unfamiliar with the WICED STUDIO SDK, please refer to the 
WICED STUDIO SDK Quickstart Guide located here: <Controller>/Doc/WICED XX Quick-Start-Guide.pdf
The WICED STUDIO Quickstart Guide documents the process to setup a computer for
use with the WICED STUDIO SDK, IDE and WICED STUDIO Evaluation Board. 

The WICED STUDIO SDK includes lots of sample applications in the <Controller>/Apps directory.
Applications included with the SDK are outlined below.
 Apps : Applications in the ROM, applications that extend the built in ROM apps and
    sample applications that are run from on-chip RAM, and may also access on-chip ROM functions
  
To obtain a complete list of build commands and options double click the "help" item in the Make Target pane.

To compile, download and run the hello_sensor application on the Cypress 20706 Eval B49 platform
double click "hello_sensor-BCM920706_P49 download" item. 

To compile, download and run the hello_sensor application on the Cypress 20739 EVAL Q40 platform
double click "hello_sensor-BCM920739EVAL_Q40 download" item. 

Header files and reference information for supported platforms is available 
in the <Controller>/platforms directory.
Platform implementations are available in the <Controller>/Platform directory.


Supported Features
---------------------------------------------------------------------
Application Features
 * Peripheral interfaces
   * GPIO
   * Timer (Software)
   * PWM
   * UART (two instances - one for application download and another for application use).
   * SPI (two instances - one for serial flash and another for application use).
   * I2C (master only).
   * RTC (Real Time Clock)
   * Keyscan
   * ADC (12 bit)
 * Generic profile level abstraction API
 * API to access NV storage areas.

* WICED Application Framework
   * OTA upgrade
   * Overlay support to load code from NV storage on demand (NV storage dependent latency and power).

Toolchains
 * GNU make
 * ARM RealView compiler toolchain and debugger

Hardware Platforms
 * 20706TAG_Q32  : Cypress 20706 based evaluation Tag board.
 * 20739EVAL_Q40  : Cypress 20739 based evaluation board with a 40-pin QFN.

Known Limitations & Notes
---------------------------------------------------------------------
  * SDK File Permissions
      In Linux, the SDK is extracted using the default permissions 
      for the current user. Users may wish to change the access permissions on the 
      SDK files. This can be done either on a one-time basis using ‘chmod –R’, or more 
      permanently for all user programs using the ‘umask’ command in a shell startup 
      script such as .bashrc or /etc/launchd-user.conf
         eg. At a prompt  : $WICED Smart-SDK> chmod -R g+w
         eg. In ~/.bashrc : umask u=rwx,g=rwx,o=
  * Programming and Debugging
      Programming is currently enabled with Cypress download tools included with
      WICED Smart SDK. Debugging is enabled by ARM RealView with Serial Wire Debug
      interface. Debugging is also enabled with Segger J-Link debug probes.
  * Application download via USB-serial/serial port and application mode are mutually
      exclusive. The serial port must be disconnected from the board for the application
      to initialize
  * ARM RealView is not currently supported out of the box with the WICED Smart SDK.  

Tools
---------------------------------------------------------------------
The GNU ARM toolchain is from Yagarto, http://yagarto.de

The SDK also supports ARM RealView 4.1 and above compiler toolchain, http://www.arm.com

The standard WICED Evaluation board (BCM92070XTAG_B49, BCM920737TAG_Q32) provides single USB-serial 
port for programming.

The debug interface is ARM Serial Wire Debug (SWD) and shares pins with download
serial lines TXd (SWDCLK) and RXd (SWDIO).

Building, programming and debugging of applications is achieved using either a 
command line interface or the WICED STUDIO IDE as described in the Quickstart Guide.

                     
WICED Technical Support
---------------------------------------------------------------------
WICED support is available on the Cypress forum at https://community.cypress.com/welcome
Access to the WICED forum is restricted to bona-fide WICED customers only.

Cypress provides customer access to a wide range of additional information, including 
technical documentation, schematic diagrams, product bill of materials, PCB layout 
information, and software updates through its customer support portal. For a CSP account, 
contact your Cypress Sales or Engineering support representative.

                     
Further Information
---------------------------------------------------------------------
Further information about WICED and the WICED Development System is
available on the WICED website at http://www.cypress.com/products/wireless-connectivity or
by contacting Cypress support at http://www.cypress.com/support
