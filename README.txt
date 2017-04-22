==========================================================
Cypress WICED Studio Software Development Kit 4.1 - README
==========================================================

WICED Studio provides systems and APIs needed to build, design and implement
applications for Wi-Fi, Bluetooth Classic (BR/EDR), Bluetooth low energy (BLE),
and ZigBee devices.

WICED Studio platforms include support for -
- 20706-A2, 20735-B0 and 20719-B0 based Bluetooth platforms.
- Support for various Cypress Wi-Fi & combo chips
   - 4390X (43909, 43907 and 43903) integrated MCU + Wi-Fi SoC
   - 4336X (43362, 43364) Wi-Fi SoC
   - 4343X (43438, 4343W) Wi-Fi SoC
   - 43340 Wi-Fi + Bluetooth combo SoC
- 20729-B0 based ZigBee platforms.

Bluetooth Features:
- API to access Bluetooth stack including GAP, ATT, GATT, SMP, L2CAP,
  RFCOMM, SDP, A2DP and AVRC in the ROM.
- A generic profile level API that abstracts the Bluetooth stack layer API.
- Reference applications for profiles defined by the Bluetooth SIG.
- WICED Bluetooth Designer wizard for quickly creating BLE applications.
- API and drivers to access on board peripherals like I2C, SPI, UART, 
  ADC, PWM, Keyscan and IR HW blocks.
- Segger J-Link debugger using J-Link GDB server and GCC GDB client.
- Support for HomeKit and iAP2 protocol

Wi-Fi Features:
 - Low-footprint embedded Wi-Fi Driver with Client (STA), softAP and Wi-Fi Direct
  - Wi-Fi <-> Bluetooth SmartBridge and Bluetooth Internet Gateway 
  - Various RTOS/TCP stack options including
    - ThreadX/NetX (IPv4), ThreadX/NetX Duo (IPv6)
  - Support for various MCU host platforms
    - ST Microelectronics : STM32F2xx, STM32F4xx
    - Cypress: FM4
    - Atmel : AT91SAM4S16B
    - Freescale : K61
    - NXP : LPC17xx, LPC18xx
  - RTOS & Network abstraction layer with a simple API for UDP, TCP, HTTP, HTTPS communications
  - SSL/TLS Security Library integrated with an HTTPS library for secure web transactions
  - DTLS security library integrated with CoAP library
  - IoT protocols - HTTP/HTTPS, CoAP, AMQP v1.0 and MQTT
  - WICED Application Framework including Bootloader, OTA Upgrade and Factory Reset
    - Second flavor of OTA and Factory Reset (called OTA2)
  - Automated Wi-Fi Easy Setup using one of several methods
    - SoftAP & Secure HTTP server
    - Wi-Fi Protected Setup
    - Apple Wireless Accessory Configuration (WAC) Protocol
  - Simple API to provide access to MCU peripherals including UART, SPI, I2C, Timers, RTC, ADCs, DACs, etc
  - Support for multiple toolchains including GNU and IAR
  - Support for Apple AirPlay and HomeKit
  - Audio Applications for Internet streaming and BT/Wi-Fi Rebroadcast
  - Unified Bluetooth BTEWICED stack for Dual Mode and Low-Enery only modes - Applications may pick the desired Bluetooth stack binary at link time.
  - Integrated Bluetooth embedded stack support for BCM20706A2 chips (when used in conjunction with the BCM43907 SoC)

  
ZigBee Features:
- ZigBee sample apps
 
WICED Studio is structured as follows:
apps               : Example & Test Applications
doc                : API & Reference Documentation, Eval Board & Module Schematics
drivers            : Drivers for USB serial converter
include            : WICED API, constants, and defaults
libraries          : Bluetooth libraries, daemons, drivers, file systems, inputs, and protocols
platforms          : Configuration files and information for supported hardware platforms
resources          : Binary and text based objects including scripts, images, and certificates
test               : Tools provided for automation testing
tools, wiced_tools : Build tools, compilers, programming tools etc.
WICED              : Core WICED components (Bluetooth, Zigbee, Wi-Fi, etc)
README.txt         : WICED Platform specifc README file
version.txt        : Version of WICED Studio


Getting Started
---------------------------------------------------------------------
If you are unfamiliar with WICED Studio, please refer to the WICED Studio
Quick Start Guide for your WICED Platform under the Eclipse IDE 'Project Explorer', 
located here: <WICED Platform>/doc/WICED-XX-Quick-Start-Guide.pdf
For example: 20706-A2_Bluetooth/doc/WICED-BT-SDK-Quick-Start-Guide.pdf
The WICED Studio Quick Start Guide documents the process to setup a computer for
use with the WICED Studio IDE and WICED Evaluation Board. 

The currently active project, including associated support files and make
targets specific to each platfrom, may be switched using the "WICED Platform"
drop down menu of the Ecliipse IDE. 

Please see the  <WICED Platform>/README.txt for each WICED Platform to get detailed 
descriptions for each platform.

WICED Studio includes lots of sample applications in the <WICED Platform>/apps
directory.  See the <WICED Platform>/apps/README.txt for more detailed
descriptions of the apps.


Tools
---------------------------------------------------------------------
The GNU ARM toolchain is from Yagarto, http://yagarto.de

WICED Studio also supports ARM RealView 4.1 and above compiler toolchain:
http://www.arm.com

The standard WICED Evaluation board provides single USB-serial port for programming.

The debug interface is ARM Serial Wire Debug (SWD) and shares pins with download
serial lines TXd (SWDCLK) and RXd (SWDIO).

Building, programming and debugging of applications is achieved using either a 
command line interface or the WICED Studio IDE as described in the Quick Start Guide.
    

WICED Technical Support
---------------------------------------------------------------------
WICED support is available on the Cypress forum at 
https://community.cypress.com/welcome
Access to the WICED forum is restricted to bona-fide WICED customers only.

Cypress provides customer access to a wide range of additional 
information, including technical documentation, schematic diagrams, 
product bill of materials, PCB layout information, and software 
updates. Please contact your Cypress Sales or Engineering support 
representative or Cypress support at http://www.cypress.com/support.


Further Information
---------------------------------------------------------------------
Further information about WICED and the WICED Development System is
available on the WICED website at http://www.cypress.com/products/wireless-connectivity
or by contacting Cypress support at http://www.cypress.com/support
