/*
 * Copyright 2016, Cypress Semiconductor
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cypress Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Cypress Semiconductor.
 */

#include <Windows.h>
#include <string>
#include "mbt.h"
using namespace std;

//**************************************************************************************************
//*** Definitions for BTW Serial Bus
//**************************************************************************************************

// Helper class to print debug messages to Debug Console
class DebugHelper
{
public:
    void DebugOut() { OutputDebugStringA(m_Buffer); }
    void DebugOut(LPCSTR v) { OutputDebugStringA(v); }
    void DebugOut(LPCSTR fmt, LPCSTR v);
    void DebugOut(LPCSTR fmt, DWORD v1);
    void DebugOut(LPCSTR fmt, DWORD v1, DWORD v2);
    void PrintCommProp(COMMPROP& commProp);
    void PrintCommState(DCB& serial_config);

    static char m_Buffer[1024];
};

//
// Serial Bus class, use this class to read/write from/to the serial bus device
//
class ComHelper : public TransportHelper
{
public:
    ComHelper();
    virtual ~ComHelper();

    // open serialbus driver to access device
    BOOL OpenPort(char* argv);
    // read data from device
    DWORD Read(LPBYTE b, DWORD dwLen);
    // write data to device
    DWORD Write(LPBYTE b, DWORD dwLen);
    DWORD current_baudrate;
    void SetBaudRate(DWORD BaudRate);
    void Flush(DWORD dwFlags);

private:
    DWORD ReadBytes(LPBYTE b, DWORD dwLen);
    // overlap IO for Read and Write
    OVERLAPPED m_OverlapRead;
    OVERLAPPED m_OverlapWrite;
};

