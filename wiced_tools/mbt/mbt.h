/*
 * Copyright 2016, Cypress Semiconductor
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cypress Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Cypress Semiconductor.
 */

#pragma once

class TransportHelper
{
public:
    TransportHelper() {};
    virtual ~TransportHelper() {};

    virtual BOOL OpenPort(char* argv) = 0;
    virtual DWORD Read(LPBYTE b, DWORD dwLen) = 0;
    virtual DWORD Write(LPBYTE b, DWORD dwLen) = 0;
    virtual void Flush(DWORD dwFlags) = 0;

    HANDLE m_handle;
};
