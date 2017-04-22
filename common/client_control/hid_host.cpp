#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QCloseEvent>
#include <QObject>
#include "hci_control_api.h"
#include <QFileDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDir>
#include <QSettings>
#include <QTextStream>
#include <QFile>


void MainWindow::InitHIDH()
{
    ui->cbHIDHProtocol->addItem("Report Mode", 0);
    ui->cbHIDHProtocol->addItem("Boot Mode", 1);
    ui->cbHIDHProtocol->setCurrentIndex(0);
}

void MainWindow::on_btnHIDHConnect_clicked()
{
    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
        return;
    }

    CBtDevice * pDev =(CBtDevice *)GetSelectedDevice();
    if (NULL == pDev)
        return;

    if(pDev->m_hidh_handle != NULL_HANDLE)
    {
        Log("HIDH already connected for selected device");
        return;
    }

    BYTE    cmd[60];
    int     commandBytes = 0;

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];


    Log("HIDH Connect Command, BDA: %02x:%02x:%02x:%02x:%02x:%02x",
           cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);

    SendWicedCommand(HCI_CONTROL_HIDH_COMMAND_CONNECT, cmd, 6);

}


void MainWindow::on_btnHIDHDisconnect_clicked()
{
    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedHIDHDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_hidh_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending HIDH Disconnect Command, Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_HIDH_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_hidh_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_HIDH;
}


void MainWindow::on_btnHIDHGetDesc_clicked()
{
    CBtDevice * pDev = GetConnectedHIDHDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_hidh_handle;
    BYTE    cmd[60];
    int     commandBytes = 0;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending Get HID Descriptor Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_HIDH_COMMAND_GET_DESCRIPTOR, cmd, commandBytes);
}

void MainWindow::on_cbHIDHProtocol_currentIndexChanged(int index)
{
    CBtDevice * pDev = GetConnectedHIDHDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_hidh_handle;

    BYTE    cmd[60];
    int     commandBytes = 0;

    int protocol = ui->cbHIDHProtocol->currentIndex();

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;
    cmd[commandBytes++] = protocol & 0xFF;

    Log("Sending HID Protocol Handle: 0x%04x Protocol:%d", nHandle, protocol);
    SendWicedCommand(HCI_CONTROL_HIDH_COMMAND_SET_PROTOCOL, cmd, commandBytes);
}

void MainWindow::HidhVirtualUnplug(uint16_t handle)
{
    BYTE   cmd[60];
    CBtDevice * pDev = FindInList(CONNECTION_TYPE_HIDH, handle, ui->cbDeviceList);
    if (pDev == NULL)
        return;

    cmd[0] = (uint8_t)(handle & 0xFF);
    cmd[1] = (uint8_t)(handle >> 8);
    SendWicedCommand(HCI_CONTROL_HIDH_COMMAND_REMOVE, cmd, 2);

    for (int i = 0; i < 6; i++)
        cmd[i] = (uint8_t)pDev->m_address[5 - i];

    // go through the registry to check find appropriate nvram chunk and tell controller to delete
    /*
    HKEY hKey;
    char szKey[MAX_PATH];
    strcpy_s(szKey, MAX_PATH, szRegKeyPrefix);
    strcat_s(szKey, szAppName);

    if (RegOpenKeyExA(HKEY_CURRENT_USER, szKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwIndex = 0;
        char  szValueName[60];
        BYTE  lpValue[MAX_PATH];
        DWORD cbValueName = sizeof(szValueName);
        DWORD cbValue = sizeof(lpValue);
        while (TRUE)
        {
            DWORD rc = RegEnumValueA(hKey, dwIndex++, szValueName, &cbValueName, NULL, NULL, lpValue, &cbValue);
            if (rc != ERROR_SUCCESS)
            {
                break;
            }
            int nvram_id;
            if ((sscanf_s(szValueName, "NVRAM %d", &nvram_id) == 1) && (cbValue <= 255))
            {
                if (memcmp(lpValue, p_device->address, 6) == 0)
                {
                    cmd[0] = (BYTE)nvram_id;
                    cmd[1] = (BYTE)(nvram_id >> 8);
                    m_ComHelper->SendWicedCommand(HCI_CONTROL_COMMAND_DELETE_NVRAM_DATA, cmd, 2);

                    RegDeleteValueA(hKey, szValueName);
                    RegCloseKey(hKey);

                    delete p_device;

                    if (m_br_edr_devices->GetCount())
                    {
                        m_br_edr_devices->SetCurSel(0);
                        OnCbnSelchangeDevices();
                    }
                    else
                        m_br_edr_devices->SetCurSel(-1);
                    return;
                }
            }
            cbValue = sizeof(lpValue);
            cbValueName = sizeof(szValueName);
        }
        RegCloseKey(hKey);
    }
    */
}

void MainWindow::DumpMemory(BYTE * p_buf, int length)
{
    char trace[1024];
    int i;

    memset(trace, 0, sizeof(trace));
    for (i = 0; i < length; i++)
    {
        sprintf(&trace[strlen(trace)], "%02X ", p_buf[i + 3]);
        if (i && (i % 32) == 0)
        {
            Log(trace);
            memset(trace, 0, sizeof(trace));
        }
    }
    if (i % 32)
    {
        Log(trace);
    }
}

void MainWindow::onHandleWicedEventHIDH(unsigned int opcode, unsigned char *p_data, unsigned int len)
{

    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_HIDH:
        HandleHIDHEvents(opcode, p_data, len);
        break;
    }
}


void MainWindow::HandleHIDHEvents(DWORD opcode, LPBYTE p_data, DWORD len)
{
    char      trace[1024];
    BYTE       bda[6];
    CBtDevice *device;
    UINT16  handle;

    switch (opcode)
    {
    case HCI_CONTROL_HIDH_EVENT_CONNECTED:
    {       
        for (int i = 0; i < 6; i++)
            bda[5 - i] = p_data[i + 1];
        sprintf(trace, "HIDH Connected status:%d address %02x:%02x:%02x:%02x:%02x:%02x handle:%d",
            p_data[0], bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], p_data[7] + (p_data[8] << 8));
        Log(trace);
        handle = p_data[7] + (p_data[8] << 8);
        // find device in the list with received address and save the connection handle
        if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
            device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

        device->m_hidh_handle = handle;
        device->m_conn_type |= CONNECTION_TYPE_HIDH;

        if (p_data[0] == 0)
        {
            sprintf(trace, "HIDH Add Device address %02x:%02x:%02x:%02x:%02x:%02x",
                bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
            Log(trace);
            SendWicedCommand(HCI_CONTROL_HIDH_COMMAND_ADD, &p_data[1], BD_ADDR_LEN);
        }

        SelectDevice(ui->cbDeviceList, bda);
    }
        break;

    case HCI_CONTROL_HIDH_EVENT_DISCONNECTED:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "HIDH Connection Closed handle:%d reason:%d ", handle, p_data[2]);
        CBtDevice * pDev = FindInList(CONNECTION_TYPE_HIDH, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_hidh_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_HIDH;
        }
        Log(trace);
    }
        break;

    case HCI_CONTROL_HIDH_EVENT_REPORT:
        sprintf(trace, "HIDH Report handle:%d ReportId:%02X data:", p_data[0] + (p_data[1] << 8), p_data[2]);
        for (uint i = 0; i < len - 3; i++)
            sprintf(&trace[strlen(trace)], "%02x ", p_data[i + 3]);
        Log(trace);
        break;

    case HCI_CONTROL_HIDH_EVENT_STATUS:
        sprintf(trace, "HIDH Cmd Status:%d ", p_data[0]);
        Log(trace);
        break;

    case HCI_CONTROL_HIDH_EVENT_DESCRIPTOR:
        sprintf(trace, "HIDH Descriptor handle:%d status:%d length:%d\n", p_data[0] + (p_data[1] << 8), p_data[2], len - 3);
        Log(trace);
        DumpMemory(&p_data[3], len - 3);
        break;

    case HCI_CONTROL_HIDH_EVENT_VIRTUAL_UNPLUG:
        sprintf(trace, "HIDH Virtual Unplug handle:%d ", p_data[0] + (p_data[1] << 8));
        Log(trace);
        HidhVirtualUnplug(p_data[0] + (p_data[1] << 8));
        break;

    case HCI_CONTROL_HIDH_EVENT_SET_PROTOCOL:
        sprintf(trace, "HIDH Set Prococol handle:%d status:%d", p_data[0] + (p_data[1] << 8), p_data[2]);
        Log(trace);
        break;

    default:
        sprintf(trace, "Rcvd Unknown HIDH OpCode: 0x%04x", opcode);
        Log(trace);
        break;
    }
}

CBtDevice* MainWindow::GetConnectedHIDHDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_hidh_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as HIDH");
        return NULL;
    }

    return pDev;
}
