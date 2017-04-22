/*
 * Copyright 2016, Cypress Semiconductor
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cypress Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Cypress Semiconductor.
 */

/* mbt.cpp : Defines the entry point for the console application. */
//#include <stdio.h>
#include <conio.h>  //for kbhit()

#include "tchar.h"
#include "mbt_com.h"
#include "mbt_usb.h"
#include "mbt.h"

/* The connectionless_dut_loopback_mode takes the following parameters:
 * opcode - 3 bytes
 * length - 1 byte
 * bd_address - 6 bytes
 * lt_addr - 1 byte
 * no_of_tests - 1 byte
 * accepts 16 tests. each test would take 6 parameters of 1 byte. - 96 bytes.
 * total[sum of all the parameters] - 108 bytes.
*/
#define CONNECTIONLESS_DUT_LOOPBACK_COMMAND_LENGTH 108
#define MAX_BD_ADDRESS_LENGTH 6

typedef unsigned char UINT8;

#define TRANSPORT_TYPE_COM  0
#define TRANSPORT_TYPE_USB  1
int mbt_transport_type;

UINT8 in_buffer[1024];
char* mbt_transport_name = NULL;
DWORD download_baudrate = 0;
DWORD application_baudrate = 0;

//
// print hexadecimal digits of an array of bytes formatted as:
// 0000 < 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F >
// 0010 < 10 11 12 13 14 15 16 1718 19 1A 1B 1C 1D 1E 1F >
//
void HexDump(LPBYTE p, DWORD dwLen)
{
    for (DWORD i = 0; i < dwLen; ++i)
    {
        if (i % 16 == 0)
            printf("%04X <", i);
        printf(" %02X", p[i]);
        if ((i + 1) % 16 == 0)
            printf(" >\n");
    }
    printf(" >\n");
}

static BOOL send_hci_command(TransportHelper *pTransport, LPBYTE cmd, DWORD cmd_len, LPBYTE expected_evt, DWORD evt_len)
{
    /* write HCI response */
    printf("Sending HCI Command:\n");
    HexDump(cmd, cmd_len);

    if (pTransport->Write(cmd, cmd_len) == cmd_len)
    {
        /* read HCI response header */
        DWORD dwRead = pTransport->Read((LPBYTE)&in_buffer[0], evt_len);

        printf("Received HCI Event:\n");
        HexDump(in_buffer, dwRead);

        if (dwRead == evt_len)
        {
            if (memcmp(in_buffer, expected_evt, evt_len) == 0)
            {
                printf("Success\n");
                return TRUE;
            }
        }
    }
    return FALSE;
}

static int execute_change_baudrate(char *szPort, int currentBaudRate, int newBaudRate)
{
    if (mbt_transport_type == TRANSPORT_TYPE_USB)
        return FALSE;

    ComHelper SerialPort;
    BOOL res;

    /* set the current baudrare on Host UART */
    SerialPort.SetBaudRate((DWORD)currentBaudRate);

    if (!SerialPort.OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        return 0;
    }

    UINT8 hci_change_baudrate[] = { 0x18, 0xFC, 0x06, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA };
    UINT8 hci_change_baudrate_cmd_complete_event[] = { 0x0E, 0x04, 0x01, 0x18, 0xFC, 0x00 };

    hci_change_baudrate[5] = newBaudRate & 0xff;
    hci_change_baudrate[6] = (newBaudRate >> 8) & 0xff;
    hci_change_baudrate[7] = (newBaudRate >> 16) & 0xff;
    hci_change_baudrate[8] = (newBaudRate >> 24) & 0xff;

    res = send_hci_command(&SerialPort, hci_change_baudrate, sizeof(hci_change_baudrate), hci_change_baudrate_cmd_complete_event, sizeof(hci_change_baudrate_cmd_complete_event));
    if (!res)
        printf("Failed execute_change_baudrate\n");
    return res;
}

static void print_usage_set_environment_variables(bool full)
{
    printf ("Set Environment Variables:\n");
    printf (" - MBT_TRANSPORT for transport selection, BTWUSB-x or COMxx\n");
    printf (" - DOWNLOAD_BAUDRATE for faster downloading when a COM Port selected (default: 115200)\n");
    printf (" - APPLICATION_BAUDRATE for communication with Application when a COM Port selected (default: 115200)\n");
    if (!full)
        return;
    printf ("eg) on Windows,\n");
    printf (" - set MBT_TRANSPORT=COMxx or MBT_TRANSPORT=BTWUSB-0\n");
    printf (" - set DOWNLOAD_BAUDRATE=4000000\n");
    printf (" - set APPLICATION_BAUDRATE=3000000\n");
    printf ("eg) on Cygwin,\n");
    printf (" - export MBT_TRANSPORT=COMxx or MBT_TRANSPORT=BTWUSB-0\n");
    printf (" - export DOWNLOAD_BAUDRATE=4000000\n");
    printf (" - export APPLICATION_BAUDRATE=3000000\n");
}

static int get_environment_variables()
{
    char* pbaud_rate = NULL;
    size_t size;
    _dupenv_s(&mbt_transport_name, &size, "MBT_TRANSPORT");
    if (mbt_transport_name == NULL)
    {
        print_usage_set_environment_variables(true);
        return 0;
    }
    else
    {
        if (strncmp(mbt_transport_name, "COM", strlen("COM")) == 0)
        {
            /*get the transport type - COMxx*/
            printf("MBT_TRANSPORT: %s\n", mbt_transport_name);
            mbt_transport_type = TRANSPORT_TYPE_COM;
            /*get the download baudrate*/
            _dupenv_s(&pbaud_rate, &size, "DOWNLOAD_BAUDRATE");
            if (pbaud_rate == NULL)
            {
                download_baudrate = 115200;
                printf("DOWNLOAD_BAUDRATE: %d (default)\n", download_baudrate);
            }
            else
            {
                download_baudrate = stoi(pbaud_rate);
                printf("DOWNLOAD_BAUDRATE: %d\n", download_baudrate);
            }
            /*get the application baudrate*/
            _dupenv_s(&pbaud_rate, &size, "APPLICATION_BAUDRATE");
            if (pbaud_rate == NULL)
            {
                application_baudrate = 115200;
                printf("APPLICATION_BAUDRATE: %d (default)\n", application_baudrate);
            }
            else
            {
                application_baudrate = stoi(pbaud_rate);
                printf("APPLICATION_BAUDRATE: %d\n", application_baudrate);
            }
        }
        else if (strncmp(mbt_transport_name, "BTWUSB-", strlen("BTWUSB-")) == 0)
        {
            /*get the transport type - BTWUSB-x*/
            printf("MBT_TRANSPORT: %s\n", mbt_transport_name);
            mbt_transport_type = TRANSPORT_TYPE_USB;
        }
    }
    return 1;
}

static void print_usage_reset(bool full)
{
    printf ("Usage: mbt reset\n");
}

static int execute_reset(char *szPort)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL  res;
    UINT8 hci_reset[] = {0x03, 0x0c, 0x00};
    UINT8 hci_reset_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};

    res = send_hci_command(pTransport, hci_reset, sizeof(hci_reset), hci_reset_cmd_complete_event, sizeof(hci_reset_cmd_complete_event));
    if (!res)
        printf("Failed execute_reset\n");
    delete pTransport;
    return res;
}

static int execute_reset_application(char *szPort)
{
    ComHelper SerialPort;
    BOOL res;

    /*should be the same speed with which Application uses */
    SerialPort.SetBaudRate((DWORD)application_baudrate);

    if (!SerialPort.OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        return 0;
    }

    UINT8 hci_reset[] = {0x03, 0x0c, 0x00};
    UINT8 hci_reset_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};

    res = send_hci_command(&SerialPort, hci_reset, sizeof(hci_reset), hci_reset_cmd_complete_event, sizeof(hci_reset_cmd_complete_event));
    if (!res)
        printf("Failed execute_reset_application\n");
    return res;
}

static void print_usage_write_bd_addr(bool full)
{
    printf("Usage: mbt write_bd_addr <bd_addr>\n");
    if (!full)
        return;
    printf("Example:\n");
    printf("             mbt write_bd_addr 20703A123456\n");
}

static int execute_write_bd_addr(char *szPort, char *bdaddr)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL  res;
    UINT8 hci_write_bd_addr[] = { 0x01, 0xFC, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    UINT8 hci_write_bd_addr_cmd_complete_event[] = { 0xe, 0x04, 0x01, 0x01, 0xFC, 0x00 };
    int   params[6];

    sscanf_s(bdaddr, "%02x%02x%02x%02x%02x%02x", &params[0], &params[1], &params[2], &params[3], &params[4], &params[5]);
    for(int i = 0; i < 6; i++)
    {
        hci_write_bd_addr[i + 3] = (UINT8)params[5 - i];
    }

    res = send_hci_command(pTransport, hci_write_bd_addr, sizeof(hci_write_bd_addr), hci_write_bd_addr_cmd_complete_event, sizeof(hci_write_bd_addr_cmd_complete_event));
    if (!res)
        printf("Failed execute_write_bd_addr\n");
    delete pTransport;
    return res;
}

static void print_usage_read_bd_addr(bool full)
{
    printf("Usage: mbt read_bd_addr\n");
}

static int execute_read_bd_addr(char *szPort)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL  res;
    UINT8 hci_read_bd_addr[] = { 0x09, 0x10, 0x00 };
    UINT8 hci_read_bd_addr_cmd_complete_event[] = { 0xe, 0x0A, 0x01, 0x09, 0x10, 0x00 };

    /* write HCI response */
    printf("Sending HCI Command:\n");
    HexDump(hci_read_bd_addr, sizeof(hci_read_bd_addr));

    if (pTransport->Write(hci_read_bd_addr, sizeof(hci_read_bd_addr)) == sizeof(hci_read_bd_addr))
    {
        /* read HCI response header */
        DWORD dwRead = pTransport->Read((LPBYTE)&in_buffer[0], sizeof(in_buffer));

        printf("Received HCI Event:\n");
        HexDump(in_buffer, dwRead);

        if (dwRead == (sizeof(hci_read_bd_addr_cmd_complete_event) + 6))
        {
            if (memcmp(in_buffer, hci_read_bd_addr_cmd_complete_event, sizeof(hci_read_bd_addr_cmd_complete_event)) == 0)
            {
                printf("Success BD_ADDR %02X%02X%02X%02X%02X%02X\n", in_buffer[11], in_buffer[10], in_buffer[9], in_buffer[8], in_buffer[7], in_buffer[6]);
                delete pTransport;
                return TRUE;
            }
        }
    }
    return FALSE;
    res = send_hci_command(pTransport, hci_read_bd_addr, sizeof(hci_read_bd_addr), hci_read_bd_addr_cmd_complete_event, sizeof(hci_read_bd_addr_cmd_complete_event));
    if (!res)
        printf("Failed execute_read_bd_addr\n");
    delete pTransport;
    return res;
}

static void print_usage_le_receiver_test(bool full)
{
    printf ("Usage: mbt le_receiver_test <rx_channel>\n");
    if (!full)
        return;
    printf ("                rx_channel = (F - 2402) / 2\n");
    printf ("                    Range: 0 - 39. Frequency Range : 2402 MHz to 2480 MHz\n");
}

static int execute_le_receiver_test(char *szPort, UINT8 chan_number)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL  res;
    UINT8 hci_le_receiver_test[] = {0x01D, 0x20, 0x01, 0x00};
    UINT8 hci_le_receiver_test_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x01D, 0x20, 0x00};
    hci_le_receiver_test[3] = chan_number;

    res = send_hci_command(pTransport, hci_le_receiver_test, sizeof(hci_le_receiver_test), hci_le_receiver_test_cmd_complete_event, sizeof(hci_le_receiver_test_cmd_complete_event));
    if (!res)
        printf("Failed execute_le_receiver_test\n");
    delete pTransport;
    return res;
}

static void print_usage_le_transmitter_test(bool full)
{
    printf ("Usage: mbt le_transmitter_test <tx_channel> <data_length> <packet_payload>\n");
    if (!full)
        return;
    printf ("                tx_channel = (F - 2402) / 2\n");
    printf ("                    Range: 0 - 39. Frequency Range : 2402 MHz to 2480 MHz\n");
    printf ("                data_length: (0 - 37)\n");
    printf ("                data_pattern: (0 - 9)\n");
    printf ("                    0 Pseudo-Random bit sequence 9\n");
    printf ("                    1 Pattern of alternating bits '11110000'\n");
    printf ("                    2 Pattern of alternating bits '10101010'\n");
    printf ("                    3 Pseudo-Random bit sequence 15\n");
    printf ("                    4 Pattern of All '1' bits\n");
    printf ("                    5 Pattern of All '0' bits\n");
    printf ("                    6 Pattern of alternating bits '00001111'\n");
    printf ("                    7 Pattern of alternating bits '0101'\n");
}

static int execute_le_transmitter_test(char *szPort, UINT8 chan_number, UINT8 length, UINT8 pattern)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL res;
    UINT8 hci_le_transmitter_test[] = {0x01E, 0x20, 0x03, 0x00, 0x00, 0x00};
    UINT8 hci_le_transmitter_test_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x01E, 0x20, 0x00};
    hci_le_transmitter_test[3] = chan_number;
    hci_le_transmitter_test[4] = length;
    hci_le_transmitter_test[5] = pattern;

    res = send_hci_command(pTransport, hci_le_transmitter_test, sizeof(hci_le_transmitter_test), hci_le_transmitter_test_cmd_complete_event, sizeof(hci_le_transmitter_test_cmd_complete_event));
    if (!res)
        printf("Failed execute_le_transmitter_test\n");
    delete pTransport;
    return res;
}

static void print_usage_le_test_end(bool full)
{
    printf ("Usage: mbt le_test_end\n");
}

static int execute_le_test_end(char *szPort)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    UINT8 hci_le_test_end[] = {0x1f, 0x20, 0x00};
    UINT8 hci_le_test_end_cmd_complete_event[] = {0x0e, 0x06, 0x01, 0x1f, 0x20, 0x00};

    printf ("Sending HCI Command:\n");
    HexDump(hci_le_test_end, sizeof(hci_le_test_end));

    /* write HCI response */
    if (pTransport->Write(hci_le_test_end, sizeof(hci_le_test_end)) != sizeof(hci_le_test_end))
    {
        delete pTransport;
        return FALSE;
    }
    /* read HCI response header */
    DWORD dwRead = pTransport->Read((LPBYTE)&in_buffer[0], sizeof(in_buffer));

    printf("Received HCI Event:\n");
    HexDump(in_buffer, dwRead);
    if ((dwRead == (sizeof(hci_le_test_end_cmd_complete_event) + 2))
        && (memcmp(in_buffer, hci_le_test_end_cmd_complete_event, sizeof(hci_le_test_end_cmd_complete_event)) == 0))
    {
        printf("Success num_packets_received %d\n", in_buffer[6] + (in_buffer[7] << 8));
        delete pTransport;
        return TRUE;
    }
    delete pTransport;
    return FALSE;
}

static void print_usage_set_tx_frequency_arm(bool full)
{
    printf ("Usage: mbt set_tx_frequency_arm <carrier on/off> <tx_frequency> <tx_mode> <tx_modulation_type> <tx_power>\n");
    if (!full)
        return;
    printf ("                carrier on/off: 1 - carrier on, 0 - carrier_off\n");
    printf ("                tx_frequency = (2402 - 2480) transmit frequency, in MHz\n");
    printf ("                tx_mode = (0 - 5) unmodulated or modulated with pattern\n");
    printf ("                    0: Unmodulated\n");
    printf ("                    1: PRBS9\n");
    printf ("                    2: PRBS15\n");
    printf ("                    3: All Zeros\n");
    printf ("                    4: All Ones\n");
    printf ("                    5: Incrementing Symbols\n");
    printf ("                tx_modulation_type = (0 - 3) ignored if the mode is 'Unmodulated' selected\n");
    printf ("                    0: GFSK\n");
    printf ("                    1: QPSK\n");
    printf ("                    2: 8PSK\n");
    printf ("                    3: LE\n");
    printf ("                tx_power = (-25 - +3) transmit power in dbm\n");
}

static int execute_set_tx_frequency_arm(char *szPort, UINT8 carrier_on, UINT16 tx_frequency, int tx_mode, int tx_modulation_type, int tx_power)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL res;
    int chan_num = tx_frequency - 2400;
    UINT8 hci_set_tx_frequency_arm[] = {0x014, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    UINT8 hci_set_tx_frequency_arm_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x014, 0xfc, 0x00};

    hci_set_tx_frequency_arm[3] = (carrier_on == 0) ? 1 : 0;
    hci_set_tx_frequency_arm[4] = (carrier_on == 1) ? chan_num : 2;
    hci_set_tx_frequency_arm[5] = tx_mode;                    // unmodulated
    hci_set_tx_frequency_arm[6] = tx_modulation_type;         // modulation type
    hci_set_tx_frequency_arm[7] = (carrier_on == 1) ? 8 : 0;  // power in dBm
    hci_set_tx_frequency_arm[8] = tx_power;

    res = send_hci_command(pTransport, hci_set_tx_frequency_arm, sizeof(hci_set_tx_frequency_arm), hci_set_tx_frequency_arm_cmd_complete_event, sizeof(hci_set_tx_frequency_arm_cmd_complete_event));
    if (!res)
        printf("Failed execute_set_tx_frequency_arm\n");
    delete pTransport;
    return res;
}

static void print_usage_receive_only_test(bool full)
{
    printf ("Usage: mbt receive_only <rx_frequency>\n");
    if (!full)
        return;
    printf ("                rx_frequency = (2402 - 2480) receiver frequency, in MHz\n");
}

static int execute_receive_only(char *szPort, UINT16 rx_frequency)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL res;
    int chan_num = rx_frequency - 2400;
    UINT8 hci_receive_only[] = {0x02b, 0xfc, 0x01, 0x00};
    UINT8 hci_receive_only_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x02b, 0xfc, 0x00};
    hci_receive_only[3] = chan_num;

    res = send_hci_command(pTransport, hci_receive_only, sizeof(hci_receive_only), hci_receive_only_cmd_complete_event, sizeof(hci_receive_only_cmd_complete_event));
    if (!res)
        printf("Failed execute_receive_only\n");
    delete pTransport;
    return res;
}

static void print_usage_radio_tx_test(bool full)
{
    printf ("Usage: mbt radio_tx_test <bd_addr> <frequency> <modulation_type> <logical_channel> <bb_packet_type> <packet_length> <tx_power>\n");
    if (!full)
        return;
    printf ("                bd_addr: BD_ADDR of Tx device (6 bytes, no space between bytes)\n");
    printf ("                frequency: 0 for hopping or (2402 - 2480) transmit frequency in MHz\n");
    printf ("                    0: normal Bluetooth hopping sequence (79 channels)\n");
    printf ("                    2402 - 2480: single frequency without hopping\n");
    printf ("                modulation_type: sets the data pattern\n");
    printf ("                    0: 0x00 8-bit Pattern\n");
    printf ("                    1: 0xFF 8-bit Pattern\n");
    printf ("                    2: 0xAA 8-bit Pattern\n");
    printf ("                    3: 0xF0 8-bit Pattern\n");
    printf ("                    4: PRBS9 Pattern\n");
    printf ("                logical_channel: sets the logical channel to Basic Rate (BR) or Enhanced Data Rate (EDR) for ACL packets\n");
    printf ("                    0: EDR\n");
    printf ("                    1: BR\n");
    printf ("                bb_packet_type: baseband packet type to use\n");
    printf ("                    3: DM1\n");
    printf ("                    4: DH1 / 2-DH1\n");
    printf ("                    8: 3-DH1\n");
    printf ("                    10: DM3 / 2-DH3\n");
    printf ("                    11: DH3 / 3-DH3\n");
    printf ("                    12: EV4 / 2-EV5\n");
    printf ("                    13: EV5 / 3-EV5\n");
    printf ("                    14: DM5 / 2-DH5\n");
    printf ("                    15: DH5 / 3-DH5\n");
    printf ("                packet_length: 0 - 65535. Device will limit the length to the max for the baseband packet type\n");
    printf ("                tx_power = (-25 - +3) transmit power in dbm\n");
}

static int execute_radio_tx_test(char *szPort, char *bdaddr, int frequency, int modulation_type, int logical_channel, int bb_packet_type, int packet_length, int tx_power)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL res;
    int params[6];

    sscanf_s(bdaddr, "%02x%02x%02x%02x%02x%02x", &params[0], &params[1], &params[2], &params[3], &params[4], &params[5]);

    int chan_num = frequency - 2402;
    UINT8 hci_radio_tx_test[] = {0x051, 0xfc, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    UINT8 hci_radio_tx_test_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x051, 0xfc, 0x00};
    for( int i = 0; i < 6; i++ )
    {
        hci_radio_tx_test[i + 3] = params[5 - i];    //bd address
    }
    hci_radio_tx_test[9]  = (frequency == 0) ? 0 : 1;      //hopping mode (0: hopping, 1: single frequency)
    hci_radio_tx_test[10] = (frequency == 0) ? 0 : (frequency - 2402);  //0: hopping 0-79:channel number (0: 2402 MHz)
    hci_radio_tx_test[11] = modulation_type;               //data pattern (3: 0xAA  8-bit Pattern)
    hci_radio_tx_test[12] = logical_channel;               //logical_Channel (0:ACL EDR, 1:ACL Basic)
    hci_radio_tx_test[13] = bb_packet_type;                //modulation type (BB_Packet_Type. 3:DM1, 4: DH1 / 2-DH1)
    hci_radio_tx_test[14] = packet_length & 0xff;          //low byte of packet_length
    hci_radio_tx_test[15] = (packet_length >> 8) & 0xff;   //high byte of packet_length
    hci_radio_tx_test[16] = 8;                             //power in dBm
    hci_radio_tx_test[17] = tx_power;                      //dBm
    hci_radio_tx_test[18] = 0;                             //power table index

    res = send_hci_command(pTransport, hci_radio_tx_test, sizeof(hci_radio_tx_test), hci_radio_tx_test_cmd_complete_event, sizeof(hci_radio_tx_test_cmd_complete_event));
    if (!res)
        printf("Failed execute_set_tx_frequency_arm\n");
    delete pTransport;
    return res;
}

static void print_usage_radio_rx_test(bool full)
{
    printf ("Usage: mbt radio_rx_test <bd_addr> <frequency> <modulation_type> <logical_channel> <bb_packet_type> <packet_length>\n");
    if (!full)
        return;
    printf ("                bd_addr: BD_ADDR of Tx device (6 bytes, no space between bytes)\n");
    printf ("                frequency = (2402 - 2480) receive frequency in MHz\n");
    printf ("                modulation_type: sets the data pattern\n");
    printf ("                    0: 0x00 8-bit Pattern\n");
    printf ("                    1: 0xFF 8-bit Pattern\n");
    printf ("                    2: 0xAA 8-bit Pattern\n");
    printf ("                    3: 0xF0 8-bit Pattern\n");
    printf ("                    4: PRBS9 Pattern\n");
    printf ("                logical_channel: sets the logical channel to Basic Rate (BR) or Enhanced Data Rate (EDR) for ACL packets\n");
    printf ("                    0: EDR\n");
    printf ("                    1: BR\n");
    printf ("                bb_packet_type: baseband packet type to use\n");
    printf ("                    3: DM1\n");
    printf ("                    4: DH1 / 2-DH1\n");
    printf ("                    8: 3-DH1\n");
    printf ("                    10: DM3 / 2-DH3\n");
    printf ("                    11: DH3 / 3-DH3\n");
    printf ("                    12: EV4 / 2-EV5\n");
    printf ("                    13: EV5 / 3-EV5\n");
    printf ("                    14: DM5 / 2-DH5\n");
    printf ("                    15: DH5 / 3-DH5\n");
    printf ("                packet_length: 0 - 65535. Device will limit the length to the max for the baseband packet type\n");
}

static int execute_radio_rx_test(char *szPort, char *bdaddr, int frequency, int modulation_type, int logical_channel, int bb_packet_type, int packet_length)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL res;
    int params[6];
    sscanf_s(bdaddr, "%02x%02x%02x%02x%02x%02x", &params[0], &params[1], &params[2], &params[3], &params[4], &params[5]);

    UINT8 hci_radio_rx_test[] = {0x52, 0xfc, 0x0e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    UINT8 hci_radio_rx_test_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x52, 0xfc, 0x00};
    for( int i = 0; i < 6; i++ )
    {
        hci_radio_rx_test[i + 3] = (UINT8)params[5 - i];
    }
    hci_radio_rx_test[9]  = 0xe8;                          //low byte of report perioe in ms (1sec = 1000ms, 0x03e8)
    hci_radio_rx_test[10] = 0x03;                          //high byte
    hci_radio_rx_test[11] = frequency - 2402;
    hci_radio_rx_test[12] = modulation_type;               //data pattern (3: 0xAA 8-bit Pattern)
    hci_radio_rx_test[13] = logical_channel;               //logical_Channel (0:ACL EDR, 1:ACL Basic)
    hci_radio_rx_test[14] = bb_packet_type;                //modulation type (BB_Packet_Type. 3:DM1, 4: DH1 / 2-DH1)
    hci_radio_rx_test[15] = packet_length & 0xff;          //low byte of packet_length
    hci_radio_rx_test[16] = (packet_length >> 8) & 0xff;     //high byte of packet_length

    res = send_hci_command(pTransport, hci_radio_rx_test, sizeof(hci_radio_rx_test), hci_radio_rx_test_cmd_complete_event, sizeof(hci_radio_rx_test_cmd_complete_event));
    if (!res)
        printf("Failed execute_radio_rx_test\n");

    printf("\nRadio RX Test is running. Press any key to stop the test.\n");

    /*loop and handle the Rx Test statistics report until the 'q' key pressed*/
    while (TRUE)
    {
        /* read statistics report*/
        DWORD dwRead = pTransport->Read((LPBYTE)&in_buffer[0], 35);

        printf("Statistics Report received:\n");
        HexDump(in_buffer, dwRead);

        if ((dwRead == 35) && (in_buffer[0] == 0xFF) && (in_buffer[1] == 0x21) && (in_buffer[2] == 0x07))
        {
            printf("  [Rx Test statistics]\n");
            printf("    Sync_Timeout_Count:     0x%x\n",in_buffer[3]  | in_buffer[4] << 8  | in_buffer[5] << 16  | in_buffer[6]  << 24);
            printf("    HEC_Error_Count:        0x%x\n",in_buffer[7]  | in_buffer[8] << 8  | in_buffer[9] << 16  | in_buffer[10] << 24);
            printf("    Total_Received_Packets: 0x%x\n",in_buffer[11] | in_buffer[12] << 8 | in_buffer[13] << 16 | in_buffer[14] << 24);
            printf("    Good_Packets:           0x%x\n",in_buffer[15] | in_buffer[16] << 8 | in_buffer[17] << 16 | in_buffer[18] << 24);
            printf("    CRC_Error_Packets:      0x%x\n",in_buffer[19] | in_buffer[20] << 8 | in_buffer[21] << 16 | in_buffer[22] << 24);
            printf("    Total_Received_Bits:    0x%x\n",in_buffer[23] | in_buffer[24] << 8 | in_buffer[25] << 16 | in_buffer[26] << 24);
            printf("    Good_Bits:              0x%x\n",in_buffer[27] | in_buffer[28] << 8 | in_buffer[29] << 16 | in_buffer[30] << 24);
            printf("    Error_Bits:             0x%x\n",in_buffer[31] | in_buffer[32] << 8 | in_buffer[33] << 16 | in_buffer[34] << 24);
        }
        else
        {
            if (mbt_transport_type == TRANSPORT_TYPE_COM)
                pTransport->Flush(0x0008); //flush data in fifo (PURGE_RXCLEAR = 0x0008)
        }

        if (_kbhit() /* && getchar() == 'q'*/)  //press any key to stop the test.
        {
            UINT8 hci_reset[] = {0x03, 0x0c, 0x00};
            UINT8 hci_reset_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};

            printf("A key pressed. Stop the Radio Rx Test.\n");
            if (mbt_transport_type == TRANSPORT_TYPE_COM)
                pTransport->Flush(0x0008); //flush data in fifo (PURGE_RXCLEAR = 0x0008)

            res = send_hci_command(pTransport, hci_reset, sizeof(hci_reset), hci_reset_cmd_complete_event, sizeof(hci_reset_cmd_complete_event));
            if (!res)
            {
                printf("Failed execute_reset\n");
                if (mbt_transport_type == TRANSPORT_TYPE_COM)
                    pTransport->Flush(0x0008); //flush data in fifo (PURGE_RXCLEAR = 0x0008)
                res = send_hci_command(pTransport, hci_reset, sizeof(hci_reset), hci_reset_cmd_complete_event, sizeof(hci_reset_cmd_complete_event));
            }
            delete pTransport;
            return res;
        }
    }
    delete pTransport;
    return res;
}

static void print_usage_connectionless_dut_loopback_mode(bool full)
{
    printf("Usage: mbt connectionless_dut_loopback_mode\n");
}

static int execute_connectionless_dut_loopback_mode(char *szPort)
{
    TransportHelper *pTransport = (mbt_transport_type == TRANSPORT_TYPE_COM) ? (TransportHelper *)new ComHelper : (TransportHelper *)new UsbHelper;
    if (!pTransport->OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        delete pTransport;
        return 0;
    }

    BOOL res;
    UINT8 hci_lp[CONNECTIONLESS_DUT_LOOPBACK_COMMAND_LENGTH];
    UINT8 hci_lp_cmd_complete_event[] = {0x0e, 0x04, 0x01, 0x54, 0xfc, 0x00};
    int i, j, x, y;
    int bd[MAX_BD_ADDRESS_LENGTH], lt_addr;
    int no_of_tests, retry_offset, no_of_packets;
    int pkt_type, ret_time, test, count = 0;
    int len = 0;

    /* opcode */
    hci_lp[0] = 0x54;
    hci_lp[1] = 0xfc;

    /* length */
    hci_lp[2] = 0x00;

    printf("Enter the following parameters in hexadecimal [0xXX]\n");
    printf("Enter BD address[6 bytes of length]:\n");
    for (j = 0; j < 6; j++)
    {
        scanf_s("%x",&bd[j]);
    }
    /* copy bd address to hci_lp in reversed way */
    for (i = 3, j = 5; j >= 0; i++, j--)
    {
        hci_lp[i] = bd[j];
    }
    printf("enter Lt address [0x01-0x07]:\n");
    scanf_s("%x", &lt_addr);
    printf("enter the number of tests[0x01-0x10]:\n");
    scanf_s("%x", &no_of_tests);

    /* copy lt address and no of tests to hci_lp */
    hci_lp[9]  = lt_addr;
    hci_lp[10] = no_of_tests;

    /* loop for the number of tests */
    for (j = 0, i = 11; j < no_of_tests; j++, i+=6)
    {
        printf("enter the retry offset[%d]-[0x01-0x3f]:\n",j+1);
        scanf_s("%x", &retry_offset);
        hci_lp[i] = 0x50;

        printf("enter no of packets[%d]-[0x01-0x7fff]:\n",j+1);
        scanf_s("%x", &no_of_packets);
        /* multiplied with 8 to get total number in bytes */
        x = no_of_packets * 8;
        y = no_of_packets * 8;
        x = (x & 0x00ff);
        hci_lp[i + 1] = x;
        y = ((y & 0xff00) >> 8);
        hci_lp[i + 2] = y;

        printf("enter the packet table type[%d][0-Basic][1-enhanced]:\n",j+1);
        scanf_s("%x",&pkt_type);
        hci_lp[i + 3] = 0x04 * retry_offset;

        printf("enter retry time out[%d]:\n",j+1);
        scanf_s("%x",&ret_time);
        hci_lp[i + 4] = ret_time;

        printf("enter test_scenarios[%d]:\n",j+1);
        scanf_s("%x",&test);
        hci_lp[i + 5] = test;
        count++;
    }

    /* length added here */
    hci_lp[2] = (count * 6) + 8;    // length of the parameters
    len = hci_lp[2] + 3;            // total length of the packet (header + parameters

    res = send_hci_command(pTransport, hci_lp, len, hci_lp_cmd_complete_event, sizeof(hci_lp_cmd_complete_event));
    if (!res)
        printf("Failed execute_connectionless_dut_loopback_mode\n");
    delete pTransport;
    return res;
}

static void print_usage_download(bool full)
{
    printf("Usage: mbt download <hcd_pathname>\n");
}

static void print_usage_download_usb(void)
{
    printf("To change the firmware file that is executed on the USB device copy the\n");
    printf("new .hex file to the C:\\Windows\\System32\\drivers directory.  Then \n");
    printf("using regedit modify the RAMPatchFileName value in the\n");
    printf("HKLM\\CurrentControlSet\\Enum\\USB\\VID_<VID>&PID_<PID>\\<BDADDR>\\Device Parameters\n");
    printf("key.  Device restart, for example unplug and plug back is required for the\n");
    printf("changes to take effect.\n");
}

static int SendDownloadMinidriver(char *szPort)
{
    ComHelper SerialPort;
    BOOL res;

    /* set HOST UART to the download_baudrate */
    SerialPort.SetBaudRate((DWORD)download_baudrate);

    if (!SerialPort.OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        return 0;
    }
    BYTE arHciCommandTx[]    = { 0x2E, 0xFC, 0x00 };
    BYTE arBytesExpectedRx[] = { 0x0E, 0x04, 0x01, 0x2E, 0xFC, 0x00 };

    res = send_hci_command(&SerialPort, arHciCommandTx, sizeof(arHciCommandTx), arBytesExpectedRx, sizeof(arBytesExpectedRx));
    if (!res)
        printf("Failed SendDownloadMinidriver\n");
    return res;
}

static int SendHcdRecord(ComHelper* pSerialPort, ULONG nAddr, ULONG nHCDRecSize, BYTE * arHCDDataBuffer)
{
    BOOL res;

    BYTE arHciCommandTx[261] = { 0x4C, 0xFC, 0x00 };
    BYTE arBytesExpectedRx[] = { 0x0E, 0x04, 0x01, 0x4C, 0xFC, 0x00 };

    arHciCommandTx[2] = (BYTE)(4 + nHCDRecSize);
    arHciCommandTx[3] = (nAddr & 0xff);
    arHciCommandTx[4] = (nAddr >> 8) & 0xff;
    arHciCommandTx[5] = (nAddr >> 16) & 0xff;
    arHciCommandTx[6] = (nAddr >> 24) & 0xff;
    memcpy(&arHciCommandTx[7], arHCDDataBuffer, nHCDRecSize);

    printf("sending record at:0x%x\n", nAddr);
    res = send_hci_command(pSerialPort, arHciCommandTx, 3 + 4 + nHCDRecSize, arBytesExpectedRx, sizeof(arBytesExpectedRx));
    if (!res)
        printf("Failed SendHcdRecord\n");
    return res;
}

BOOL ReadNextHCDRecord(FILE * fHCD, ULONG * nAddr, ULONG * nHCDRecSize, UINT8 * arHCDDataBuffer, BOOL * bIsLaunch)
{
    const   int HCD_LAUNCH_COMMAND = 0x4E;
    const   int HCD_WRITE_COMMAND  = 0x4C;
    const   int HCD_COMMAND_BYTE2  = 0xFC;

    BYTE     arRecHeader[3];
    BYTE     byRecLen;
    BYTE     arAddress[4];

    *bIsLaunch = FALSE;

    if (fread(arRecHeader, 1, 3, fHCD) != 3)               // Unexpected EOF
        return false;

    byRecLen = arRecHeader[2];

    if ((byRecLen < 4) || (arRecHeader[1] != HCD_COMMAND_BYTE2) ||
        ((arRecHeader[0] != HCD_WRITE_COMMAND) && (arRecHeader[0] != HCD_LAUNCH_COMMAND)))
    {
        printf("Wrong HCD file format trying to read the command information\n");
        return FALSE;
    }

    if (fread(arAddress, sizeof(arAddress), 1, fHCD) != 1)      // Unexpected EOF
    {
        printf("Wrong HCD file format trying to read 32-bit address\n");
        return FALSE;
    }

    *nAddr = arAddress[0] + (arAddress[1] << 8) + (arAddress[2] << 16) + (arAddress[3] << 24);

    *bIsLaunch = (arRecHeader[0] == HCD_LAUNCH_COMMAND);

    *nHCDRecSize = byRecLen - 4;

    if (*nHCDRecSize > 0)
    {
        if (fread(arHCDDataBuffer, 1, *nHCDRecSize, fHCD) != *nHCDRecSize)   // Unexpected EOF
        {
            printf("Not enough HCD data bytes in record\n");
            return FALSE;
        }
    }

    return TRUE;
}

static int SendLaunchRam(char *szPort)
{
    ComHelper SerialPort;
    BOOL res;

    /* set HOST UART to the download_baudrate */
    SerialPort.SetBaudRate((DWORD)download_baudrate);

    if (!SerialPort.OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        return 0;
    }
    BYTE arHciCommandTx[] = { 0x4E, 0xFC, 0x04, 0xFF, 0xFF, 0xFF, 0xFF };
    BYTE arBytesExpectedRx[] = { 0x0E, 0x04, 0x01, 0x4E, 0xFC, 0x00 };

    res = send_hci_command(&SerialPort, arHciCommandTx, sizeof(arHciCommandTx), arBytesExpectedRx, sizeof(arBytesExpectedRx));
    if (!res)
        printf("Failed SendLaunchRam\n");
    return res;
}

static int SendHCDFile(char *szPort, char *pathname)
{
    FILE *  fHCD = NULL;
    ULONG   nAddr, nHCDRecSize;
    BYTE    arHCDDataBuffer[256];
    BOOL    bIsLaunch = FALSE;
    ComHelper SerialPort;

    fopen_s(&fHCD, pathname, "rb");
    if (!fHCD)
    {
        printf("Failed to open HCD file %s\n", pathname);
        return 0;
    }

    /* set HOST UART to the download_baudrate */
    SerialPort.SetBaudRate((DWORD)download_baudrate);

    if (!SerialPort.OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        fclose(fHCD);
        return 0;
    }

    while (ReadNextHCDRecord(fHCD, &nAddr, &nHCDRecSize, arHCDDataBuffer, &bIsLaunch))
    {
        if (!SendHcdRecord(&SerialPort, nAddr, nHCDRecSize, arHCDDataBuffer))
        {
            printf("Failed to send hcd portion at %x\n", nAddr);
            if (fHCD)
                fclose(fHCD);
            return 0;
        }
        if (bIsLaunch)
            break;
    }

    if (fHCD)
        fclose(fHCD);

    return 1;
}

static int execute_download(char *szPort, char *pathname)
{
    if (!execute_reset(szPort))
    {
        if (!execute_reset(szPort))
        {
            printf("Failed to HCI Reset\n");
            return 0;
        }
    }
    printf("HCI Reset success\n");

    /* switch the Device baudrate to higher speed for faster downloading, download_baudrate eg) 3M or 4M*/
    if (!execute_change_baudrate(szPort, 115200, download_baudrate))
    {
        printf("Failed to change the Baudrate to downloading speed\n");
        return 0;
    }
    printf("Baudrate change to download speed success\n");

    if (!SendDownloadMinidriver(szPort))
    {
        printf("Failed to send download minidriver\n");
        return 0;
    }
    printf("Download minidriver success, continue downloading .hcd file\n");

    /* downloading FW patch or Application(*.hcd format) */
    if (!SendHCDFile(szPort, pathname))
    {
        printf("Failed to send *.hcd file\n");
        return 0;
    }
    printf("Download .hcd file success\n");

    /* send Launch Ram */
    if (!SendLaunchRam(szPort))
    {
        printf("Failed to send launch RAM\n");
    }
    printf("Launch RAM success\n");

    /*wait for device to launch/run the Application/Firmware downloaded*/
    Sleep (50);

    /* HCI Reset after downloading, use APPLICATION_BAUDRATE*/
    printf("Issue HCI Reset after downloading Application\n");
    if (!execute_reset_application(szPort))
    {
        if (!execute_reset_application(szPort))
        {
            printf("Failed to HCI Reset after LaunchRam\n");
            return 0;
        }
    }

    /*switch  the device baudrare to 115200 for testing commands and events*/
    printf("switch the device baudrate to 115200. test commands use 115200\n");
    if (!execute_change_baudrate(szPort, application_baudrate, 115200))
    {
        printf("Failed to change the device baudrate to 115200\n");
        return 0;
    }

    return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
    int chan_num = 0;
    int pattern  = 0;
    int length   = 0;
    char *path;

    if (!get_environment_variables())
        return 0;

    if ((argc >= 2) && (_stricmp(argv[1], "reset") == 0))
    {
        if (argc == 2)
        {
            return (execute_reset(mbt_transport_name));
        }
        print_usage_reset(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "le_receiver_test") == 0))
    {
        if (argc == 3)
        {
            chan_num = atoi(argv[2]);
            if ((chan_num >= 0) && (chan_num <= 39))
            {
                return (execute_le_receiver_test(mbt_transport_name, chan_num));
            }
        }
        print_usage_le_receiver_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "le_test_end") == 0))
    {
        if (argc == 2)
        {
            return (execute_le_test_end(mbt_transport_name));
        }
        print_usage_le_test_end(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "le_transmitter_test") == 0))
    {
        if (argc == 5)
        {
            chan_num = atoi(argv[2]);
            if ((chan_num >= 0) && (chan_num <= 39))
            {
                length = atoi(argv[3]);
                if ((length > 0) && (chan_num <= 255))
                {
                    pattern = atoi(argv[4]);
                    if ((pattern >= 0) && (pattern < 7))
                    {
                        return (execute_le_transmitter_test(mbt_transport_name, chan_num, length, pattern));
                    }
                }
            }
        }
        print_usage_le_transmitter_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "set_tx_frequency_arm") == 0))
    {
        if (argc > 2)
        {
            UINT8 carrier_on = atoi(argv[2]);
            if ((carrier_on == 0) || (carrier_on == 1))
            {
                if (carrier_on == 0)
                {
                    return execute_set_tx_frequency_arm(mbt_transport_name, carrier_on, 2402, 0, 0, 0);
                }
                else if (argc == 7)
                {
                    int tx_frequency = atoi(argv[3]);
                    if ((tx_frequency >= 2402) && (tx_frequency <= 2480))
                    {
                        int tx_mode = atoi(argv[4]);
                        if ((tx_mode >= 0) && (tx_mode <= 5))
                        {
                            int tx_modulation_type = atoi(argv[5]);
                            if ((tx_modulation_type >= 0) && (tx_modulation_type <= 3))
                            {
                                int tx_power = atoi(argv[6]);
                                if ((tx_power >= -25) && (tx_power <= 3))
                                {
                                    return execute_set_tx_frequency_arm(mbt_transport_name, carrier_on, tx_frequency, tx_mode, tx_modulation_type, tx_power);
                                }
                            }
                        }
                    }
                }
            }
        }
        print_usage_set_tx_frequency_arm(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "receive_only") == 0))
    {
        if (argc == 3)
        {
            chan_num = atoi(argv[2]);
            if ((chan_num >= 2402) && (chan_num <= 2480))
            {
                return (execute_receive_only(mbt_transport_name, chan_num));
            }
        }
        print_usage_receive_only_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "write_bd_addr") == 0))
    {
        if ((argc == 3) && (strlen(argv[2]) == 12))
        {
            return(execute_write_bd_addr(mbt_transport_name, argv[2]));
        }
        print_usage_write_bd_addr(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "read_bd_addr") == 0))
    {
        if (argc == 2)
        {
            return(execute_read_bd_addr(mbt_transport_name));
        }
        print_usage_read_bd_addr(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "radio_tx_test") == 0))
    {
        if (argc == 9)
        {
            if(strlen(argv[2]) == 12)
            {
                int frequency = atoi(argv[3]);
                if ((frequency == 0) || (frequency >= 2402) && (frequency <= 2480))
                {
                    int modulation_type = atoi(argv[4]);
                    if ((modulation_type >= 0) && (modulation_type <= 4))
                    {
                        int logical_channel = atoi(argv[5]);
                        if ((logical_channel >= 0) && (logical_channel <= 1))
                        {
                             int bb_packet_type = atoi(argv[6]);
                             if ((bb_packet_type >= 3) && (bb_packet_type <= 15))
                             {
                                 int packet_length = atoi(argv[7]);
                                 if ((packet_length >= 0) && (packet_length <= 0xffff))
                                 {
                                     int tx_power = atoi(argv[8]);
                                     if ((tx_power >= -25) && (tx_power <= 3))
                                     {
                                         return execute_radio_tx_test(mbt_transport_name, argv[2], frequency, modulation_type, logical_channel, bb_packet_type, packet_length, tx_power);
                                     }
                                 }
                             }
                        }
                    }
                }
            }
        }
        print_usage_radio_tx_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "radio_rx_test") == 0))
    {
        if (argc == 8)
        {
            if(strlen(argv[2]) == 12)
            {
                int frequency = atoi(argv[3]);
                if ((frequency >= 2402) && (frequency <= 2480))
                {
                    int modulation_type = atoi(argv[4]);
                    if ((modulation_type >= 0) && (modulation_type <= 4))
                    {
                        int logical_channel = atoi(argv[5]);
                        if ((logical_channel >= 0) && (logical_channel <= 1))
                        {
                            int bb_packet_type = atoi(argv[6]);
                            if ((bb_packet_type >= 3) && (bb_packet_type <= 15))
                            {
                                int packet_length = atoi(argv[7]);
                                if ((packet_length >= 0) && (packet_length <= 0xffff))
                                {
                                    return execute_radio_rx_test(mbt_transport_name, argv[2], frequency, modulation_type, logical_channel, bb_packet_type, packet_length);
                                }
                            }
                        }
                    }
                }
            }
        }
        print_usage_radio_rx_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "connectionless_dut_loopback_mode") == 0))
    {
        if (argc == 2)
        {
            return (execute_connectionless_dut_loopback_mode(mbt_transport_name));

        }
        print_usage_connectionless_dut_loopback_mode(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "download") == 0))
    {
        if (mbt_transport_type == TRANSPORT_TYPE_USB)
        {
            print_usage_download_usb();
            return 0;
        }
        if (argc == 3)
        {
            path = (argv[2]);
            return (execute_download(mbt_transport_name, path));
        }
        print_usage_download(true);
        return 0;
    }
    else
    {
        printf("Usage: mbt help\n");
        print_usage_reset(false);
        print_usage_le_receiver_test(false);
        print_usage_le_transmitter_test(false);
        print_usage_le_test_end(false);
        print_usage_set_tx_frequency_arm(false);
        print_usage_receive_only_test(false);
        print_usage_read_bd_addr(false);
        print_usage_write_bd_addr(false);
        print_usage_radio_tx_test(false);
        print_usage_radio_rx_test(false);
        print_usage_connectionless_dut_loopback_mode(false);
        print_usage_download(false);
        printf("\nCheck Bluetooth Core 4.1 spec vol. 2 Sections 7.8.28-7.2.30\nfor details of LE Transmitter and Receiver tests\n");
    }
    return 0;
}

