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

void MainWindow::InitAG()
{
    m_audio_connection_active = false;
}

void MainWindow::on_btnAGConnect_clicked()
{
    if (m_CommPort == NULL)
        return;

    CBtDevice * pDev =(CBtDevice *)GetSelectedDevice();
    if (NULL == pDev)
        return;

    if(pDev->m_ag_handle != NULL_HANDLE)
    {
        Log("AG already connected for selected device");
        return;
    }

    BYTE    cmd[60];
    int     commandBytes = 0;
    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];


    Log("Sending AG Connect Command, BDA: %02x:%02x:%02x:%02x:%02x:%02x",
           cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);

    SendWicedCommand(HCI_CONTROL_AG_COMMAND_CONNECT, cmd, 6);
}

void MainWindow::on_btnAGDisconnect_clicked()
{
    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedAGDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_ag_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending AG Disconnect Command, Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_AG_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_ag_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_AG;
}

void MainWindow::on_btnAGAudioConnect_clicked()
{
    BYTE    cmd[60];
    int     commandBytes = 0;
    CBtDevice * pDev = GetConnectedAGDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_ag_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    if (!m_audio_connection_active)
    {
        SendWicedCommand(HCI_CONTROL_AG_COMMAND_OPEN_AUDIO, cmd, commandBytes);
        Log("Sending Audio Connect Command, Handle: 0x%04x", nHandle);
    }
    else
    {
        SendWicedCommand(HCI_CONTROL_AG_COMMAND_CLOSE_AUDIO, cmd, commandBytes);
        Log("Sending Audio Disconnect Command, Handle: 0x%04x", nHandle);
    }
}

void MainWindow::onHandleWicedEventAG(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_AG:
        HandleAgEvents(opcode, p_data, len);
        break;
    }
}

void MainWindow::HandleAgEvents(DWORD opcode, LPBYTE p_data, DWORD len)
{
    char   trace[1024];
    CBtDevice *device;
    BYTE    bda[6];

    UINT16  handle, features;


    switch (opcode)
    {
    case HCI_CONTROL_AG_EVENT_OPEN:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd HCI_CONTROL_AG_EVENT_OPEN   BDA: %02x:%02x:%02x:%02x:%02x:%02x  Status: %u",
            handle, p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7], p_data[8]);
        Log(trace);

        if (p_data[8] == HCI_CONTROL_HF_STATUS_SUCCESS)
        {
            for (int i = 0; i < 6; i++)
                bda[5 - i] = p_data[2 + i];

            // find device in the list with received address and save the connection handle
            if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
                device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

            device->m_ag_handle = handle;
            device->m_conn_type |= CONNECTION_TYPE_AG;

            SelectDevice(ui->cbDeviceList, bda);

        }
    }
        break;
    case HCI_CONTROL_AG_EVENT_CLOSE:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x12 - HCI_CONTROL_AG_EVENT_CLOSE", handle);
        Log(trace);        

        ui->btnAGAudioConnect->setText("Audio Connect");
        CBtDevice * pDev = FindInList(CONNECTION_TYPE_AG, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_hf_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_AG;
            m_audio_connection_active = false;
        }
    }

        break;
    case HCI_CONTROL_AG_EVENT_CONNECTED:
        handle   = p_data[0] | (p_data[1] << 8);
        features = p_data[2] | (p_data[3] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x13 - HCI_CONTROL_AG_EVENT_CONNECTED  Features: 0x%04x", handle, features);
        Log(trace);
        break;
    case HCI_CONTROL_AG_EVENT_AUDIO_OPEN:
        handle   = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x14 - HCI_CONTROL_AG_EVENT_AUDIO_OPEN", handle);
        Log(trace);        
        ui->btnAGAudioConnect->setText("Audio Disconnect");
        m_audio_connection_active = true;
        break;
    case HCI_CONTROL_AG_EVENT_AUDIO_CLOSE:
        handle   = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x15 - HCI_CONTROL_AG_EVENT_AUDIO_CLOSE", handle);
        Log(trace);        
        ui->btnAGAudioConnect->setText("Audio Connect");
        m_audio_connection_active = false;
        break;
    }
}

CBtDevice* MainWindow::GetConnectedAGDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_ag_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as AG");
        return NULL;
    }

    return pDev;
}
