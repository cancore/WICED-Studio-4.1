Manufacturing Bluetooth Test Tool

Overview
--------
The manufacturing Bluetooth test tool (MBT) is used to test and verify the RF
performance of the BCM2070x family of SoC Bluetooth BR/EDR/BLE devices.
Each test sends an HCI command to the device and then waits for an HCI Command
Complete event from the device.

MBT can be configured to control a BT device over the USB or serial interface.
To run over USB the BTWUSB driver package should be installed and the device
should be enumerated in the "Bluetooth Devices" section of the Windows Device
Manager.

Environment Variables
---------------------
To configure MBT to run over USB set environment variable
MBT_TRANSPORT=BTWUSB-0
If more than one device is plugged in, the consecutive device can be testing
by setting environment variable to BTWUSB-1 and so on.

To configure MBT to run over serial set environment variables
MBT_TRANSPORT=COMxx
DOWNLOAD_BAUDRATE=4000000
APPLICATION_BAUDRATE=3000000
where COMxx corresponds to the COM port assigned to the device's HCI UART.
when a COM port is selected, a higher baudrate can be used for faster downloading by
setting DOWNLOAD_BAUDRATE. The download speed is depend on platform such as 4000000.
if no DOWNLOAD_BAUDRATE is set the 115200 speed will be used by default.
when a COM port is selected, the APPLICATION_BAUDRATE need to be set. the
baudrate should be the same speed as the application uses.
if no APPLICATION_BAUDRATE is set the 115200 speed will be used by default.

Reset Test
----------
This test verifies that the device is correctly configured and connected to the PC.

Usage: mbt reset

The example below sends HCI_Reset command to the device and processes the HCI
Command Complete event (BLUETOOTH SPECIFICATION Version 4.1 [Vol 2], Section
7.3.2 for details).

WICED-SmartReady-SDK\Tools\mbt\win32>mbt reset
Sending HCI Command:
0000 < 03 0C 00 >
Received HCI Event:
0000 < 0E 04 01 03 0C 00 >
Success

The last byte of the HCI Command Complete event is the operation status, where
0 signifies success.

LE Receiver Test
----------------
This test configures the BCM2070x to receive reference packets at a fixed
interval. External test equipment should be used to generate the reference
packets.

The channel on which the device listens for the packets is passed as a
parameter. BLE devices use 40 channels, each of which is 2 MHz wide. Channel
0 maps to 2402 MHz and Channel 39 maps to 2480 MHz (see BLUETOOTH
SPECIFICATION Version 4.1 [Vol 2], Section 7.8.28 for details).

Usage: mbt le_receiver_test <rx_channel>
where:
    rx_channel = receive frequency minus 2402 divided by 2. For example,
    if the desired receive frequency is 2406MHz then the rx_channel =
    (2406 – 2402) / 2 = 2.
    The channel range is 0–39 (2402–2480 MHz).

The example below starts the LE receiver test on Channel 2 (2406 MHz).

WICED-SmartReady-SDK\Tools\mbt\win32>mbt le_receiver_test 2
Sending HCI Command:
0000 < 1D 20 01 02 >
Received HCI Event:
0000 < 0E 04 01 1D 20 00 >
Success
LE Receiver Test running, to stop execute mbt le_test_end

The last byte of the HCI Command Complete event is the operation status,
where 0 signifies success. Use mbt le_test_end to complete the test and
print the number of received packets.

Note: This test will fail if the device is running another test: use
le_test_end or reset to put the BCM2070x in idle state before running this
test.

LE Transmitter Test
-------------------
The LE Transmitter Test configures the BCM2070x to send test packets at a
fixed interval. External test equipment may be used to receive and analyze
the reference packets.

The channel on which the BCM2070x transmits the packets  is passed as a
parameter. BLE devices use 40 channels, each of which is 2 MHz wide. Channel 0
maps to 2402 MHz and Channel 39 maps to 2480 MHz.

The other two parameters specify the length of the test data and the data
pattern to be used (see BLUETOOTH SPECIFICATION Version 4.1 [Vol 2], Section
7.8.29 for details).

Usage: mbt le_transmitter_test <tx_channel> <data_length> <packet_payload>
where:
    tx_channel = transmit frequency minus 2402 divided by 2. For example, if
    the transmit frequency is 2404 MHz then the tx_channel = (2404 – 2402) / 2 = 1.
    The channel range is 0–39 (2402–2480 MHz).

    data_length = 0–37

    data_pattern = 0–7
        0 = Pseudo-random bit sequence 9
        1 = Pattern of alternating bits: 11110000
        2 = Pattern of alternating bits: 10101010
        3 = Pseudo-random bit sequence 15
        4 = Pattern of all 1s
        5 = Pattern of all 0s
        6 = Pattern of alternating bits: 00001111
        7 = Pattern of alternating bits: 0101

The example below starts the test and instructs the device to transmit packets
on Channel 2 (2406 MHz), with a 10-byte payload of all ones (1s).

WICED-SmartReady-SDK\Tools\mbt\win32>mbt le_transmitter_test 2 10 4
Sending HCI Command:
0000 < 1E 20 03 02 0A 04 >
Received HCI Event:
0000 < 0E 04 01 1E 20 00 >
Success
LE Transmitter Test running, to stop execute mbt le_test_end

The last byte of the HCI Command Complete event is the status of the operation,
where 0 signifies the success.  Use mbt le_test_end to complete the test.

Note: This test will fail if the device is running another test: use le_test_end
or reset to put the BCM2070x in idle state before running this test.

LE Test End
-----------
This command stops the LE Transmitter or LE Receiver Test that is in progress
on the BCM2070x.  The number of packets received during the test is reported
by the device and printed out. The value will always be zero (0) if the LE
Transmitter Test was active (see BLUETOOTH SPECIFICATION Version 4.1 [Vol 2],
Section 7.8.30 for details).

Usage: mbt le_test_end

The example below stops the active test.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt le_test_end
Sending HCI Command:
0000 < 1F 20 00 >
Received HCI Event:
0000 < 0E 06 01 1F 20 00 00 00 >
Success num_packets_received 0

Continuous Transmit Test
------------------------
Note: Unlike the LE tests, this test uses 79 frequencies, each 1 MHz wide.

This test configures the BCM2070x to turn the carrier ON or OFF. When the
carrier is ON the device transmits the carrier which specified modulation mode
and type on the specified frequency at a specified power level.


The modulation mode, modulation type, frequency, and power level to be used by
the BCM2070x are passed as parameters.

Usage: mbt set_tx_frequency_arm <carrier on/off> <tx_frequency> <tx_mode> <tx_modulation_type> <tx_power>
where:

    carrier on/off:
        1 = carrier ON
        0 = carrier OFF
    tx_frequency = (2402 – 2480) transmit frequency, in MHz
    tx_mode:
        0: Unmodulated
        1: PRBS9
        2: PRBS15
        3: All Zeros
        4: All Ones
        5: Incrementing Symbols
    tx_modulation_type:
        0: GFSK
        1: QPSK
        2: 8PSK
        3: LE
    tx_power = (–25 to +3) transmit power, in dBm

The example below turns the carrier ON and instructs the BCM2070x to transmit the PRBS9 mode, 8PSK type pattern on 2402 MHz at 3 dBm.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt set_tx_frequency_arm 1 2402 1 2 3
Sending HCI Command:
0000 < 14 FC 07 00 02 01 02 08 03 00 >
Received HCI Event:
0000 < 0E 04 01 14 FC 00 >
Success

To stop the test, send the command a second time with the carrier on/off parameter set to zero (0).
No other parameters are used.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt set_tx_frequency_arm 0
Sending HCI Command:
0000 < 14 FC 07 01 02 00 00 00 00 00 >
Received HCI Event:
0000 < 0E 04 01 14 FC 00 >
Success

Continuous Receive Test
------------------------
This test configures the BCM2070x to set the specified receive frequency and turns on the receiver.

Usage: mbt receive_only <rx_frequency>
where:
    Rx_frequency = (2402 – 2480) receive frequency, in MHz

The example below sets the receive frequency on 2402 MHz and truns on the receiver.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt receive_only 2402
Sending HCI Command:
0000 < 2B FC 01 02 >
Received HCI Event:
0000 < 0E 04 01 2B FC 00 >
Success

To stop the test send the reset command.

Read BD Address
---------------
This test reads the bd address of local BCM2070x device.

Usage: mbt read_bd_addr

The example below reads the local bd address.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt read_bd_addr
Sending HCI Command:
0000 < 09 10 00 >
Received HCI Event:
0000 < 0E 0A 01 09 10 00 AC 1F 01 3A 70 20 >
Success BD_ADDR 20703A011FAC

Write BD Address
----------------
This test writes the bd address to the local BCM2070x device.

Usage: mbt write_bd_addr <bd_addr>
where:
    bd_addr = 6 bytes of bd address (no space between bytes)

The example below writes the bd address, 20703A011FAC, to local device.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt write_bd_addr 20703A011FAC
Sending HCI Command:
0000 < 01 FC 06 AC 1F 01 3A 70 20 >
Received HCI Event:
0000 < 0E 04 01 01 FC 00 >
Success

Radio Tx Test
-------------
Note: Connectionless transmit test to send Bluetooth packets

This test configures the BCM2070x to transmit the selected data pattern which is on the specified frequency
and specified logical channel at a specified power level.

This test generates the write_bd_addr command to write bd_addr passed as a parameter.
The frequency, modulation_type, logical channel, bb_packet_type, packet_length, and power level to be used by
the BCM2070x are passed as parameters.

Usage: mbt radio_tx_test <bdaddr> <frequency> <modulation_type> <logical_channel> <bb_packet_type> <packet_length> <tx_power>
where:
    bd_addr: BD_ADDR of Tx device (6 bytes, no space between bytes)
    frequency: 0 or transmit frequency (2402 – 2480) in MHz
        0: normal Bluetooth hopping sequence (79 channels)
        2402 - 2480: single frequency without hopping
    modulation_type:
        1: 0x00 8-bit Pattern
        2: 0xFF 8-bit Pattern
        3: 0xAA 8-bit Pattern
        9: 0xF0 8-bit Pattern
        4: PRBS9 Pattern
    logical_channel:
        0: EDR
        1: BR
    bb_packet_type:
        3: DM1
        4: DH1 / 2-DH1
        8: 3-DH1
        10: DM3 / 2-DH3
        11: DH3 / 3-DH3
        14: DM5 / 2-DH5
        15: DH5 / 3-DH5
    packet_length: 0 – 65535. Device will limit the length to the max for the baseband packet type.
        eg) if DM1 packets are sent, the maximum packet size is 17 bytes.
    tx_power = (–25 to +3) transmit power, in dBm.

The example below instructs the BCM2070x to transmit 0xAA 8-bit Pattern on the 2402 MHz and ACL Basic
with DM1 packet (max 17 bytes) type at -3 dBm.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt radio_tx_test 20703A012345 2402 3 1 3 0 -3
Sending HCI Command:
0000 < 01 FC 06 45 23 01 3A 70 20 >
Received HCI Event:
0000 < 0E 04 01 01 FC 00 >
Success
Sending HCI Command:
0000 < 51 FC 10 45 23 01 3A 70 20 01 00 02 01 03 11 >
0010 < 00 08 FD 00 >
Received HCI Event:
0000 < 0E 04 01 51 FC 00 >
Success

The last byte of the HCI Command Complete event is the operation status.
where 0 signifies that operation was successful and test started to run.
The test continues to run until device is reset (mbt reset).

Radio Rx Test
-------------
Note: Connectionless receive test for Bluetooth packets

This test issues a command to the BCM2070x to set radio to camp on a specified frequency.
While test is running the BCM2070x periodically sends reports about received packets.

Usage: mbt radio_rx_test <bd_addr> <frequency> <modulation_type> <logical_channel> <bb_packet_type> <packet_length>
where:
    bd_addr: BD_ADDR for the remote Tx device (6 bytes, no space between bytes)
    frequency = receive frequency (2402 – 2480) in MHz
    modulation_type:
        0: 0x00 8-bit Pattern
        1: 0xFF 8-bit Pattern
        2: 0xAA 8-bit Pattern
        3: 0xF0 8-bit Pattern
        4: PRBS9 Pattern
    logical_channel:
        0: EDR
        1: BR
    bb_packet_type:
        3: DM1
        4: DH1 / 2-DH1
        8: 3-DH1
        10: DM3 / 2-DH3
        11: DH3 / 3-DH3
        14: DM5 / 2-DH5
        15: DH5 / 3-DH5
    packet_length: 0 – 65535.
        Device will compare length of the received packets with the specified packet_length.

BCM2070x will generate the statistics report of the Rx Test every second.

The example below instructs the BCM2070x to receive 0xAA 8-bit Pattern on the 2402 MHz and ACL Basic with DM1 packet type.

WICED-SmartReady-SDK\Tools\mbt\win32>mbt radio_rx_test 20703A012345 2402 3 1 3 0
Sending HCI Command:
0000 < 52 FC 0E 45 23 01 3A 70 20 E8 03 00 02 01 03 >
0010 < 11 00 >
Received HCI Event:
0000 < 0E 04 01 52 FC 00 >
Success

Mbt reports connectionless Rx Test statistics every second.

The example below shows the Rx Test Statistics report
Received HCI Event:
0000 < FF 21 07 01 00 00 00 00 00 00 00 DE 15 00 00 >
0010 < DE 15 00 00 00 00 00 00 00 00 00 00 00 00 00 00 >
0020 < 00 00 00 00 >
  [report Rx Test statistics]
    Sync_Timeout_Count:     0x1
    HEC_Error_Count:        0x0
    Total_Received_Packets: 0x15de
    Good_Packets:           0x15de
    CRC_Error_Packets:      0x0
    Total_Received_Bits:    0x0
    Good_Bits:              0x0
    Error_Bits:             0x0

Press any key to stop the test.

Connectionless_DUT_Loopback_Test
--------------------------------
The basic concept for this test is derived from loopback mode. The tester will
transmit a specific packet to the DUT which is retrasmitted. This structure
will enable the tester to analyse both tx and rx characteristics.

Usage: mbt connectionless_dut_loopback_mode

When executed, the parameters must be entered in hexa-decimal format.[0xXX]
BD-Address should be of 6 bytes. No_of_tests must a value greater than  zero
and less than 16.

The example below takes one test to perform.

WICED-SDK\Tools\mbt\win32>mbt connectionless_dut_loopback_mode
BD-Address : 0x00 0x00 0x00 0x01 0x02 0x03
lt_addr : 0x07
no_of_tests: 0x01
retry_offset : 0x01
no_of_packets: 0x01
pkt_type: 0x01
retry_timeout: 0x01
test_scenario: 0x00
Sending HCI Command:
0000 < 54 FC 0E 03 02 01 00 00 00 07 01 50 01 00 01 01 00 >
Received HCI Event:
0000 < 0E 04 01 54 FC 00 >
Success

Downloading Application
-----------------------
This command downloads the Firmware or Application to BCM2070x device and uses following 2 environment Variables,
DOWNLOAD_BAUDRATE: used for faster downloaing.
APPLICATION_BAUDRATE: used for reset the device after downloading and changes the baudrate to 115200 speed.
After downloading the MBT will the baudrate 115200 for all testing commands.

Usage: mbt download <hcd_filename>
where:
    hcd_filename: Firmware or Application file name including the path.

The example below downloads the firmware file, BCM20703A1_firmware.hcd, to BCM20703A1 device

WICED-SmartReady-SDK\Tools\mbt\win32>./mbt download BCM20703A1_firmware.hcd
Sending HCI Command:
0000 < 52 FC 0E 45 23 01 3A 70 20 E8 03 00 02 01 03 >
.... downloading
00B0 < 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 >
Received HCI Event:
0000 < 0E 04 01 4C FC 00 >
Success
sending record at:0xffffffff
Sending HCI Command:
0000 < 4C FC 04 FF FF FF FF >
Received HCI Event:
0000 < 0E 04 01 4C FC 00 >
Success
Download .hcd file success
Sending HCI Command:
0000 < 4E FC 04 FF FF FF FF >
Received HCI Event:
0000 < 0E 04 01 4E FC 00 >
Success
Launch RAM success
Issue HCI Reset after downloading Application
Sending HCI Command:
0000 < 03 0C 00 >
Received HCI Event:
0000 < 0E 04 01 03 0C 00 >
Success
switch the device baudrate to 115200. test commands use 115200
Sending HCI Command:
0000 < 18 FC 06 00 00 00 C2 01 00 >
Received HCI Event:
0000 < 0E 04 01 18 FC 00 >
Success

