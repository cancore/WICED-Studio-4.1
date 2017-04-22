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



void MainWindow::InitAudioSnk()
{


}


void MainWindow::on_btnAvSinkDisconnect_clicked()
{
    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
        return;
    }

    CBtDevice * pDev =(CBtDevice *)GetConnectedAudioSnkDevice();
    if (NULL == pDev)
        return;

    BYTE    cmd[60];
    int     commandBytes = 0;
    USHORT  nHandle = pDev->m_avk_handle;

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending Audio Snk Disconnect Command, Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_AUDIO_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_avk_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_AVK;
}

void MainWindow::on_btnAVSinkConnect_clicked()
{
    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
        return;
    }

    // connect audio snk
    unsigned char    cmd[60];
    int     commandBytes = 0;    


    CBtDevice * pDev =(CBtDevice *)GetSelectedDevice();

    if (pDev == NULL)
        return;

    if(pDev->m_avk_handle != NULL_HANDLE)
    {
        Log("AV SNK already connected for selected device Handle: 0x%04x", pDev->m_avk_handle);
        return;
    }

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];


    SendWicedCommand(HCI_CONTROL_AUDIO_COMMAND_CONNECT, cmd, commandBytes);

}

void MainWindow::onHandleWicedEventAudioSnk(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {

    case HCI_CONTROL_GROUP_AUDIO:
        HandleA2DPEventsAudioSnk(opcode, p_data, len);
        break;
    }
}


\

void MainWindow::HandleA2DPEventsAudioSnk(DWORD opcode, BYTE *p_data, DWORD len)
{
    BYTE       bda[6];
    CBtDevice *device;
    UINT16     handle;

    switch (opcode)
    {
    case HCI_CONTROL_AUDIO_EVENT_CONNECTED:
    {
        Log("Audio snk connected");
        int i = 0;
        for (i = 0; i < 6; i++)
            bda[5 - i] = p_data[i];        

        handle = p_data[i++] + (p_data[i++] << 8);

        // find device in the list with received address and save the connection handle
        if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
            device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

        device->m_avk_handle = handle;
        device->m_conn_type |= CONNECTION_TYPE_AVK;

        SelectDevice(ui->cbDeviceList, bda);


        Log("Audio Snk Connected, Handle: 0x%04x", handle);

    }
        break;

    case HCI_CONTROL_AUDIO_EVENT_DISCONNECTED:
    {

        handle = p_data[0] | (p_data[1] << 8);
        CBtDevice * pDev = FindInList(CONNECTION_TYPE_AVK, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_avk_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_AVK;
        }

        Log("Audio sink disconnected, Handle: 0x%04x", handle);
    }
        break;

    case HCI_CONTROL_AUDIO_EVENT_STARTED:
        Log("Audio snk started");

        break;

    case HCI_CONTROL_AUDIO_EVENT_STOPPED:
        Log("Audio snk stopped");

        break;

    case HCI_CONTROL_AUDIO_EVENT_REQUEST_DATA:
;
        break;

    case HCI_CONTROL_AUDIO_EVENT_COMMAND_COMPLETE:
        Log("Audio snk event command complete");
        break;

    case HCI_CONTROL_AUDIO_EVENT_COMMAND_STATUS:
        Log("Audio snk event command status");
        break;

    case HCI_CONTROL_AUDIO_EVENT_CONNECTION_FAILED:
        Log("Audio snk event connection attempt failed (0x%X)", opcode);
        break;

    default:
        Log("Rcvd cmd: %d (0x%X)", opcode, opcode);
        /* Unhandled */
        break;
    }
}

CBtDevice* MainWindow::GetConnectedAudioSnkDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_avk_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as AV SNK");
        return NULL;
    }

    return pDev;
}
