/*
 * Copyright 2016, Cypress Semiconductor
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cypress Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Cypress Semiconductor.
 */

#include "mbt_usb.h"
#include "stdio.h"

DWORD dwThreadId;

//
//Class UsbHelper Implementation
//
UsbHelper::UsbHelper()
{
    m_handle = INVALID_HANDLE_VALUE;
}

UsbHelper::~UsbHelper()
{
    ClosePort();
}

//
// Open USB device driver
//
BOOL UsbHelper::OpenPort(char* port)
{
    char lpStr[20];
    sprintf_s(lpStr, 20, "\\\\.\\%s", port);

    // open once only
    if (m_handle != NULL&& m_handle != INVALID_HANDLE_VALUE)
        return FALSE;

    m_handle = CreateFile(lpStr,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (m_handle != INVALID_HANDLE_VALUE)
    {
        Flush(0);
        return TRUE;
    }
    return FALSE;
}

BOOL UsbHelper::ClosePort()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
    }
    return TRUE;
}

// read a number of bytes from USB Device Device
// Parameters:
//  lpBytes - Pointer to the buffer
//  dwLen   - number of bytes to read
// Return:  Number of byte read from the device.
//
DWORD UsbHelper::Read(LPBYTE lpBytes, DWORD dwLen)
{
    LPBYTE p = lpBytes;
    DWORD Length = dwLen;
    DWORD dwRead = 0;
    DWORD dwTotalRead = 0;
    ULONG timeout = 1000;   // wait for 1000 msec
    BYTE  data_frame[512];
    DWORD nBytes = sizeof(DWORD);

    nBytes = sizeof(timeout);
    if (!DeviceIoControl(m_handle, IOCTL_BTWUSB_GET_EVENT, &timeout, sizeof(timeout), data_frame, sizeof(data_frame), &nBytes, NULL))
    {
        printf("Get event packet failed - 0x%x...\n", GetLastError());
        return 0;
    }

    if (!nBytes)
    {
        return 0;
    }

    memcpy(lpBytes, data_frame, dwLen < nBytes ? dwLen : nBytes);
    return nBytes;
}


// Write a number of bytes to USB Device
// Parameters:
//  lpBytes - Pointer to the buffer
//  dwLen   - number of bytes to write
// Return:  Number of byte Written to the device.
//
DWORD UsbHelper::Write(LPBYTE lpBytes, DWORD dwLen)
{
    DWORD Length = dwLen;
    DWORD dwTotalWritten = 0;
    DWORD nBytes;

    if (!DeviceIoControl(m_handle, IOCTL_BTWUSB_PUT_CMD, lpBytes, dwLen, NULL, 0, &nBytes, NULL))
    {
        printf("Write command failed - 0x%x...\n", GetLastError());
        return 0;
    }
    return nBytes;
}

// USB device may have a bunch of HCI events sitting in the queue, read them until
// device does not have events to deliver
void UsbHelper::Flush(DWORD dwFlags)
{
    ULONG timeout = 200;   // wait for 200 msec
    BYTE  data_frame[512];
    DWORD nBytes = sizeof(DWORD);

    nBytes = sizeof(timeout);
    while (!DeviceIoControl(m_handle, IOCTL_BTWUSB_GET_EVENT, &timeout, sizeof(timeout), data_frame, sizeof(data_frame), &nBytes, NULL))
    {
    }
    return;
}