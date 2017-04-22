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

const char *hf_ag_response[] =
{
    "OK",
    "ERROR",
    "+CME ERROR:",
    "RING",
    "+VGS:",
    "+VGM:",
    "+CCWA:",
    "+CHLD:",
    "+CIND:",
    "+CLIP:",
    "+CIEV:",
    "+BINP:",
    "+BVRA:",
    "+BSIR:",
    "+CNUM:",
    "+BTRH:",
    "+COPS:",
    "+CLCC:",
    "+BIND:",
    "UNKNOWN AT"
};

const char *hf_ag_command[] =
{
    "+VGS",
    "+VGM",
    "A",
    "+BINP",
    "+BVRA",
    "+BLDN",
    "+CHLD",
    "+CHUP",
    "+CIND",
    "+CNUM",
    "D",
    "+NREC",
    "+VTS",
    "+BTRH",
    "+COPS",
    "+CMEE",
    "+CLCC",
    "+BIA",
    "+BIEV",
    ""
};


void MainWindow::InitHF()
{
    m_audio_connection_active = false;    
    m_mic_cur_pos = 8;
    m_speaker_cur_pos = 8;
    ui->horizontalSliderHFMic->setRange(0, 15);
    ui->horizontalSliderHFSpeaker->setRange(0, 15);
    ui->horizontalSliderHFMic->setSliderPosition(m_mic_cur_pos);
    ui->horizontalSliderHFSpeaker->setSliderPosition(m_speaker_cur_pos);
    ui->cbHFDTMF->setCurrentIndex(11);
    ui->cbHFHeldCalls->setCurrentIndex(0);
}


void MainWindow::on_btnConnectHF_clicked()
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

    if(pDev->m_hf_handle != NULL_HANDLE)
    {
        Log("HF already connected for selected device");
        return;
    }

    BYTE    cmd[60];
    int     commandBytes = 0;

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];


    Log("Sending HFP Connect Command, BDA: %02x:%02x:%02x:%02x:%02x:%02x",
           cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);

    SendWicedCommand(HCI_CONTROL_HF_COMMAND_CONNECT, cmd, 6);

}

void MainWindow::on_btnDisconnectHF_clicked()
{
    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedHFDevice();
    if (pDev == NULL)
        return;

    USHORT nHandle = pDev->m_hf_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending HFP Disconnect Command, Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_HF_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_hf_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_HF;
}

void MainWindow::on_btnHFConnectAudio_clicked()
{
    BYTE    cmd[60];
    int     commandBytes = 0;
    CBtDevice * pDev = GetConnectedHFDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_hf_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    if (!m_audio_connection_active)
    {
        SendWicedCommand(HCI_CONTROL_HF_COMMAND_OPEN_AUDIO, cmd, commandBytes);
        Log("Sending Audio Connect Command, Handle: 0x%04x", nHandle);
    }
    else
    {
        SendWicedCommand(HCI_CONTROL_HF_COMMAND_CLOSE_AUDIO, cmd, commandBytes);
        Log("Sending Audio Disconnect Command, Handle: 0x%04x", nHandle);
    }
}

void MainWindow::on_btnHFHangup_clicked()
{
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_CHUP, 0, NULL);
}

void MainWindow::on_btnHFAnswer_clicked()
{
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_A, 0, NULL);
}

void MainWindow::on_btnHFRedial_clicked()
{
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_BLDN, 0, NULL);
}


void MainWindow::on_btnHFDial_clicked()
{
    char    atStr[201] = { 0 };    

    memset(atStr, 0, sizeof(atStr));
    QString str = ui->lineEditHFDial->text();
    strncpy(atStr, str.toStdString().c_str(), 200);
    if (atStr[strlen(atStr)] != ';')
        atStr[strlen(atStr)] = ';';
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_D, 0, atStr);

}

void MainWindow::on_btnHFDTMF_clicked()
{
    QString str = ui->cbHFDTMF->itemText(ui->cbHFDTMF->currentIndex());

    char    atStr[201] = { 0 };

    memset(atStr, 0, sizeof(atStr));
    strcpy(atStr, str.toStdString().c_str());


    char digit[2] = { 0, 0 };
    digit[0] = (char) atStr[0];
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_VTS, 0, digit);
}

void MainWindow::on_btnHFVoiceReco_clicked()
{
    static BYTE voice_recognition_enabled = FALSE;
    voice_recognition_enabled ^= 1;
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_BVRA, voice_recognition_enabled, NULL);
    ui->btnHFVoiceReco->setText((voice_recognition_enabled ? "Stop Voice Recognition" : "Start Voice Recognition"));

}

void MainWindow::on_btnHFCallHeld_clicked()
{
    int chld_action = ui->cbHFHeldCalls->currentIndex();
    if (chld_action == 0)
        return;
    SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_CHLD, chld_action - 1, NULL);

}

void MainWindow::on_horizontalSliderHFMic_sliderMoved(int position)
{
    if (m_mic_cur_pos != position)
    {
        m_mic_cur_pos = position;
        SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_MIC, m_mic_cur_pos, NULL);
    }
}

void MainWindow::on_horizontalSliderHFSpeaker_sliderMoved(int position)
{
    if (m_speaker_cur_pos != position)
    {
        m_speaker_cur_pos = position;
        SendAtCmd(HCI_CONTROL_HF_AT_COMMAND_SPK, m_speaker_cur_pos, NULL);
    }
}


void MainWindow::onHandleWicedEventHF(unsigned int opcode, unsigned char *p_data, unsigned int len)
{

    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_HF:
        HandleHFEvents(opcode, p_data, len);
        break;
    }
}

void MainWindow::HandleHFEvents(DWORD opcode, LPBYTE p_data, DWORD len)
{
    char   trace[1024];
    CBtDevice *device;
    BYTE    bda[6];

    UINT16  handle, features, num;
    const char   *pAtStr;

    switch (opcode)
    {
    case HCI_CONTROL_HF_EVENT_OPEN:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd HCI_CONTROL_HF_EVENT_OPEN   BDA: %02x:%02x:%02x:%02x:%02x:%02x  Status: %u",
            handle, p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7], p_data[8]);
        Log(trace);

        if (p_data[8] == HCI_CONTROL_HF_STATUS_SUCCESS)
        {
            for (int i = 0; i < 6; i++)
                bda[5 - i] = p_data[2 + i];

            // find device in the list with received address and save the connection handle
            if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
                device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

            device->m_hf_handle = handle;
            device->m_conn_type |= CONNECTION_TYPE_HF;

            SelectDevice(ui->cbDeviceList, bda);

        }
    }
        break;
    case HCI_CONTROL_HF_EVENT_CLOSE:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x12 - HCI_CONTROL_HF_EVENT_CLOSE", handle);
        Log(trace);
        m_audio_connection_active = false;
        ui->btnHFConnectAudio->setText("Audio Connect");

        CBtDevice * pDev = FindInList(CONNECTION_TYPE_HF, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_hf_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_HF;
        }
    }

        break;
    case HCI_CONTROL_HF_EVENT_CONNECTED:
        handle   = p_data[0] | (p_data[1] << 8);
        features = p_data[2] | (p_data[3] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x13 - HCI_CONTROL_HF_EVENT_CONN  Features: 0x%04x", handle, features);
        Log(trace);
        break;
    case HCI_CONTROL_HF_EVENT_AUDIO_OPEN:
        handle   = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x14 - HCI_CONTROL_HF_EVENT_AUDIO_OPEN", handle);
        Log(trace);
        m_audio_connection_active = TRUE;
        ui->btnHFConnectAudio->setText("Audio Disconnect");
        break;
    case HCI_CONTROL_HF_EVENT_AUDIO_CLOSE:
        handle   = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x15 - HCI_CONTROL_HF_EVENT_AUDIO_CLOSE", handle);
        Log(trace);
        m_audio_connection_active = FALSE;
        ui->btnHFConnectAudio->setText("Audio Connect");
        break;

    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_OK:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_ERROR:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_RING:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_VGS:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_VGM:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CCWA:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CHLD:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CIND:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CLIP:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CIEV:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_BINP:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_BVRA:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_BSIR:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CNUM:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_BTRH:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_COPS:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CMEE:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_CLCC:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_BIND:
    case HCI_CONTROL_HF_AT_EVENT_BASE + HCI_CONTROL_HF_AT_EVENT_UNAT:
        {
            char rsp[512] = { 0 };
            handle = p_data[0] | (p_data[1] << 8);
            pAtStr = hf_ag_response[opcode - HCI_CONTROL_HF_AT_EVENT_BASE];
            num = p_data[2] | (p_data[3] << 8);

            sprintf(trace, "[Handle: %u] Rcvd Event 0x%02x - AG Response: %s  Num: %u  Params: ", handle, opcode - HCI_CONTROL_HF_AT_EVENT_BASE, pAtStr, num);
            strncpy(rsp, (const char *)&p_data[4], len - 4);
            strcat(trace, rsp);
            Log(trace);
            break;
        }
    }
}

void MainWindow::SendAtCmd(int nAtCmd, int num, char *atStr)
{
    CBtDevice * pDev = GetConnectedHFDevice();
    if (pDev == NULL)
        return;

    int     nHandle = pDev->m_hf_handle;
    BYTE    cmd[300]={0};
    int     commandBytes = 0;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;
    cmd[commandBytes++] = num & 0xff;
    cmd[commandBytes++] = (num >> 8) & 0xff;

    if (atStr)
        strncpy((char *)&cmd[commandBytes], atStr, sizeof(cmd) - commandBytes-1);
        //strcpy_s((char *)&cmd[commandBytes], sizeof(cmd) - commandBytes, atStr);

    char trace[300];

    if (atStr)
    {
        char rsp[256];
        memset(rsp, 0, sizeof(rsp));
        strcpy(rsp, atStr);
        sprintf(trace, "Sending HFP AT Command: %u  %s Handle: %d  Num : %u  AT Param: %s", nAtCmd, hf_ag_command[nAtCmd], nHandle, num, rsp);
        SendWicedCommand(HCI_CONTROL_HF_AT_COMMAND_BASE + nAtCmd, cmd, commandBytes + strlen(atStr));
    }
    else
    {
        sprintf(trace, "Sending HFP AT Command: %u  %s Handle: %d  Num : %u", nAtCmd, hf_ag_command[nAtCmd], nHandle, num);
        SendWicedCommand(HCI_CONTROL_HF_AT_COMMAND_BASE + nAtCmd, cmd, commandBytes);
    }
    Log(trace);
}

CBtDevice* MainWindow::GetConnectedHFDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_hf_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as HF");
        return NULL;
    }

    return pDev;
}
