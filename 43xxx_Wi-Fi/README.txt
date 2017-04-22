=====================================================================
Cypress WICED Software Development Kit 4.1 - README
=====================================================================

The WICED SDK provides a full compliment of application level APIs, 
libraries and tools needed to design & implement secure embedded wireless
networking applications. 

Major features of the WICED SDK include ...
  - Low-footprint embedded Wi-Fi Driver with Client (STA), softAP and Wi-Fi Direct
  - Wi-Fi <-> Bluetooth SmartBridge and Bluetooth Internet Gateway 
  - Various RTOS/TCP stack options including
    - ThreadX/NetX (IPv4), ThreadX/NetX Duo (IPv6)
  - Support for various Cypress Wi-Fi & combo chips
    - 4390X (43909, 43907 and 43903) integrated MCU + Wi-Fi SoC
    - 4336X (43362, 43364) Wi-Fi SoC
    - 4343X (43438, 4343W) Wi-Fi SoC
    - 43340 Wi-Fi + Bluetooth combo SoC
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
      - OTA2 Failsafe - if OTA2 extraction is interrupted, the update will recover 
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
  - Added Handsfree profile support to Bluetooth embedded stack for BCM20706A2 chips (when used in conjunction with the BCM43907 SoC).
  - Enhanced AMQP to support SASL plain authentication to enable connectivity to Azure cloud servic
  - HomeKit Tunnel Accessory support for Bluetooth LE accessories on 43907+20707 based platforms
  - Verified PEAPv0 (with MSCHAPv2) and EAP-TLS Enterprise security protocol with FreeRadius, IAS and ACS RADIUS servers
  - Added gSPI protocol support (slave) on BCM4390x
  - Implemented USB device mode support on BCM43907


The WICED SDK release is structured as follows:
  apps          : Example & Test Applications
  doc           : API & Reference Documentation
  include       : WICED API, constants, and defaults 
  libraries     : Bluetooth, daemons, drivers, file systems, inputs, and protocols
  platforms     : Evaluation board support package, including Eval Board and Module Schematics
  resources     : Binary and text based objects including scripts, images, and certificates
  tools         : Build tools, compilers, debugger, makefiles, programming tools etc.
  tools/drivers : Drivers for WICED evaluation boards
  WICED         : WICED core components (RTOS, Network Stack, Wi-Fi Driver, Security & Platform libraries)
  WICED/WWD     : The WICED Wi-Fi Driver (equivalent to the Wiced directory in previous SDK-1.x releases) 
  README.txt    : This file
  CHANGELOG.txt : A log of changes for each SDK revision
 

Getting Started
---------------------------------------------------------------------
If you are unfamiliar with the WICED SDK, please refer to the 
WICED Quickstart Guide located here: <WICED-SDK>/doc/WICED-QSG2xx-R.pdf
The WICED Quickstart Guide documents the process to setup a computer for
use with the WICED SDK, IDE and WICED Evaluation Board. 

The WICED SDK includes lots of sample applications in the <WICED-SDK>/Apps directory.
Applications included with the SDK are outlined below.

 apps/demo : Demonstration Applications
   - Applications demonstrating how to integrate various WICED API features 
   
 apps/snip : Application Snippets
   - Various applications to demonstrate usage of individual WICED APIs        

 apps/test : Test & Utility Applications
   - console      : Provides various test features including Iperf for throughput testing 
   - mfg_test     : Manufacturing Test application to enable radio performance and certification testing

 apps/waf  : WICED Application Framework
   - bootloader   : Bootloader application used in conjunction with the WICED Application Framework
   - sflash_write : Serial flash application used to write data into external serial flash
   
 apps/wwd : Wiced Wi-Fi Driver Applications to demonstrate advanced usage of the low layer Wi-Fi driver
    
To obtain a complete list of build commands and options, enter the following text in the
base WICED SDK directory on a command line:
$> make

To compile, download and run the Wi-Fi scan application on 943362WCD4 evaluation platform, 
enter the following text on a command line (a period character is used to reference applications 
in sub-directories) :
$> make snip.scan-BCM943362WCD4 download run

The default RTOS and Network Stack components are defined in the WICED configuration makefile  
at <WICED-SDK>/tools/makefiles/wiced_config.mk. The default I/O bus component is defined in the platform
makefile at <WICED-SDK>/platforms/<Platform>/<Platform>.mk. Defaults may be bypassed by specifying the 
component as part of the build string if desired as shown in the following example.
$> make snip.scan-BCM943362WCD4-ThreadX-NetX-SDIO download run
       
Source code, headers and reference information for supported platforms are available 
in the <WICED-SDK>/platforms directory. Source code, headers, linker scripts etc that 
are common to all platforms are available in the <WICED-SDK>/WICED/platform directory.


Supported Features
---------------------------------------------------------------------
Wi-Fi & Bluetooth SmartBridge Features
 * Scan and associate to Wi-Fi access points
 * Authenticate to Wi-Fi Access Points with the following security types:
   Open, WEP-40, WEP-104, WPA (AES & TKIP), WPA2 (AES, TKIP & Mixed mode)
 * AP mode with support for security types : Open, WPA, WPA2
 * Concurrent AP & STA mode (AP mode limited to 3 concurrent connected clients)
 * Wi-Fi Direct
 * WPS 1.0 & 2.0 Enrollee & Registrar (Internal Registrar only)
 * Wi-Fi APIs : Network keep alive, packet filters
 * Host <-> Wi-Fi SDIO & SPI interface
 * Bluetooth SmartBridge with multiple connections including the
   following features: Whitelist, Bond Storage, Attribute Caching, 
   GATT Procedures, Configurable Maximum Concurrent Connections, Directed 
   Advertisements, Device address initialisation, Passkey entry
 * Host <-> Wi-Fi via Memory to Memory DMA engine

Bluetooth Features
 * Bluetooth Dual-mode stack support - classic BR/EDR and BLE modes
   - Also available BLE-only stack library with reduced memory footprint
 * Generic Attribute (GATT) profile
   - Sample BLE applications - Hello sensor and Proximity reporter
 * A2DP v1.2 (Advanced Audio Distribution Profile)
   - A2DP Sink Functionality
   - SBC Decoder
 * AVRCP (Audio/Video Remote Control Profile)
   - AVRCP Controller v1.3
   - AVRCP Target v1.5 (Absolute Volume)
 * Man-Machine-Interface via buttons
   - AVRCP play/pause/Skip-forward/Skip-backward
   - A2DP Volume Up/Down
 * SDAP (Service Discovery Application Profile)
 * GAP (Generic Access Profile)


RTOS & Network Stack Support
 * ThreadX  / NetX    (object file; free for use with WICED *ONLY*)
 * ThreadX  / NetXDuo (object file; free for use with WICED *ONLY*)
 * NuttX (Supported only for 4390x platforms)

Networking Features (IPv4 & IPv6)
 * ICMP (Ping)
 * ARP
 * TCP
 * UDP 
 * IGMP (Multicast)
 * IPv6 NDP, Multicast
 * DHCP (Client & Server)
 * DNS (Client & Redirect Server)
 * mDNS/DNS-SD Zeroconf Network Discovery (Gedday)
 * TLS1.0/1.1/1.2 and DTLS (object file with host abstraction layer; free for use with WICED *ONLY*)
 * HTTP / HTTPS (Client & Server)
 * SNTP
 * SMTP
 * SSDP

Application Features
 * Apple AirPlay (requires Apple authentication co-processor; available to Apple MFi licensees *ONLY*) 
 * Apple HomeKit (available to Apple MFi licensees *ONLY*)
 * Apollo Wireless Audio distribution
 * Bluetooth Audio and BLE
 * Peripheral interfaces
   * GPIO
   * Timer / PWM
   * UART
   * SPI
   * I2C
   * RTC (Real Time Clock)
 * Enterprise security 
   * Support for EAP-TLS Enterprise security protocol with SteelBelt, FreeRadius, IAS, ACS and Device
   * Support for PEAPv0 Enterprise security protocol with SteelBelt, FreeRadius and Device
 * Xively "Internet of Things" protocol
 * COAP (Constrained Application Protocol) integrated with DTLS for secure connections
 * MQTT (MQ Telemetry Transport) with AWS sample application
 * AMQP (v0.9 and v1.0) and SASL plain support along with Azure sample application
 * BLE Wi-Fi introducer WICED app along with iOS Application that uses BLE proximity pairing to introduce Wi-Fi network to new device and assist in on-boarding connection to Wi-Fi network.
 * Alibaba cloud (Aliyun) messaging service sample application over HTTP

* WICED Application Framework
   * Bootloader
   * Device Configuration Table (region in flash to store AP, security credentials, TLS certs, serial number, Wi-Fi country code, etc)
   * OTA upgrade
   * Factory reset
   * Automated configuration via softAP & webserver
   * Apple Wireless Accessory Configuration (WAC) protocol (available to Apple MFi licensees *ONLY*)
   * System Monitor to manage the watchdog

Toolchains
 * GNU

Hardware Platforms
 4336X
   * BCM943362WCD4  : WICED Module with STM32F205 MCU mounted on BCM9WCD1EVAL1
   * BCM943362WCD6  : WICED Module with STM32F415 MCU mounted on BCM9WCD1EVAL1
   * BCM943362WCD8  : WICED Module with Atmel SAM4S16B MCU mounted on BCM9WCD1EVAL1
   * BCM9WCDPLUS114 : WICED+ Eval Board (includes BCM43362+STM32F205 WICED+ Module and BCM20702 Bluetooth module)
   * BCM943364WCD1  : WICED Module with STM32F215 MCU mounted on BCM9WCD1EVAL1
 943340
   * BCM943340WCD1  : BCM43340-based WICED Module with STM32F417 MCU mounted on BCM9WCD5EVAL1
 94343X
   * BCM943438WCD1  : BCM43438-based WICED Module with STM32F411 MCU mounted on BCM9WCD9EVAL1
   * BCM94343WWCD1  : BCM4343W-based WICED Module with STM32F411 MCU mounted on BCM9WCD9EVAL1
   * BCM94343WWCD2  : BCM4343W-based WICED Module with STM32F412 MCU mounted on BCM9WCD9EVAL1
 4390X
   * BCM943907WAE2_1     : BCM43907WCD2 SiP-based WICED Module on BCM943907WAE_1
   * BCM943909WCD1_3     : BCM43909 SiP-based WICED Module on BCM943909WCDEVAL_2
   * BCM943907AEVAL1F_1  : BCM43907WLCSPR SiP-based WICED Module on BCM943907AEVAL1F_1
   * BCM943907WAE_1      : BCM43907WCD2 SiP-based WICED Module on BCM943907WAE_1
   * BCM943903PS         : BCM43903 Postage Stamp WICED Module on BCM943903WCD2_1
   * BCM943907WCD1       : BCM43907WCD1 SiP-based WICED Module on BCM943909WCDEVAL_2
   * BCM943907WCD2       : BCM43907WCD2 SiP-based WICED Module on BCM943909WCDEVAL_2
 FM4
   * GAIA : FM4-176L-S6E2GM Pioneer Kit with BCM943438WLPTH_2
 
Known Limitations & Notes
---------------------------------------------------------------------

 * Features not yet supported in WICED-SDK-4.1
   - IAR Embedded Workspace native support
   - WAC procedure is not supported for AP mode with WEP security

   * Platform Restrictions:
       BCM943340WCD1
         - Wi-Fi Direct not support
         - SPI bus not supported
     
   * Wi-Fi Powersave (1)
       The WLAN chip requires an external 32kHz sleep clock input during powersave.
       Platforms that do not support Wi-Fi powersave (per the table above) are
       not capable of driving the WLAN sleep clock. An external 32kHz clock is 
       required for these platforms.


 * libc does not include support for printing uint64_t (long long)
   
 * RTOS detection may cause OpenOCD to crash in the following situation:
     GDB has an elf containing a known RTOS loaded, and your app is using the 
     RTOS memory for other purposes. This situation may occur while debugging 
     the bootloader
     SOLUTION : Remove " -rtos auto " from the <WICED-SDK>/Tools/OpenCD/OpenOCD 
                .cfg file that matches your hardware debugger 
                (ie. BCM9WCD1EVAL1.cfg for WICED Evaluation Boards) 

 * For the 43362, AP mode when running with Wi-Fi Direct is limited to 2 STA clients

 * P2P feature
     If the board's MAC address is locally administered then it needs to be modified
     to become a global address or P2P will not initialize.
     First build the application using the SDK, then find the
     generated_mac_address.txt file and modify the MAC address.
     Ex: Change text "macaddr=02:0A:F7:9c:76:f2" to "macaddr=00:0A:F7:9c:76:f2"
 
Tools
---------------------------------------------------------------------
The GNU ARM toolchain is from Yagarto, http://yagarto.de

Programming and debugging is enabled by OpenOCD, http://openocd.berlios.de

The standard WICED Evaluation board (BCM9WCD1EVAL1) provides two physical 
programming/debug interfaces for the STM32 host microprocessor: USB-JTAG and direct 
JTAG. The WICED Evaluation board driver additionally provides a single USB-serial 
port for debug printing or UART console purposes.

The USB-JTAG interface is enabled by the libftdi/libusb open source driver,
http://intra2net.com/en/developer/libftdi. 
The direct JTAG interface works with third party JTAG programmers including 
Segger, IAR J-Link and Olimex ARM-USB-TINY series. OpenOCD works with the libftdi/libusb 
USB-JTAG driver shipped with the WICED SDK and commercially available JTAG drivers 
available from third party vendors.

Building, programming and debugging of applications is achieved using either a 
command line interface or the WICED IDE as described in the Quickstart Guide.
                     
WICED Technical Support
---------------------------------------------------------------------
WICED support is available on Cypress Developer Community: https://community.cypress.com 

Cypress provides customer access to a wide range of additional information, including 
technical documentation, schematic diagrams, product bill of materials, PCB layout 
information, and software updates. Please contact your Cypress Sales or Engineering 
support representative or Cypress support at http://www.cypress.com/support.
