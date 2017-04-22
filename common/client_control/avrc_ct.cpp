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

#include "avrc.h"

tAPP_AVRC_META_ATTRIB     repeat_ct;
tAPP_AVRC_META_ATTRIB     shuffle_ct;

void DumpData(char *description, void* p, unsigned int length, unsigned int max_lines);

static const char *szAvrcPlayStatus[] = { "Stopped", "Playing", "Paused", "Fwd Seek", "Rev Seek" };
const char *repeatStr[] = { "Off", "Single", "All", "Group"};
const char *shuffleStr[] = { "Off", "All", "Group"};


void MainWindow::InitAVRCCT()
{
    // setup signals/slots
    connect(ui->btnCTPlay, SIGNAL(clicked()), this, SLOT(onCTPlay()));
    connect(ui->btnCTStop, SIGNAL(clicked()), this, SLOT(onCTStop()));
    connect(ui->btnCTPause, SIGNAL(clicked()), this, SLOT(onCTPause()));
    connect(ui->btnCTPrev, SIGNAL(clicked()), this, SLOT(onCTPrevious()));
    connect(ui->btnCTNext, SIGNAL(clicked()), this, SLOT(onCTNext()));
    connect(ui->btnCTSeekBack, SIGNAL(clicked()), this, SLOT(onCTSkipBackward()));
    connect(ui->btnCTSeekFwd, SIGNAL(clicked()), this, SLOT(onCTSkipForward()));
    connect(ui->btnCTMute, SIGNAL(clicked()), this, SLOT(onCTMute()));
    connect(ui->btnCTVolumeUp, SIGNAL(clicked()), this, SLOT(onCTVolumeUp()));
    connect(ui->btnCTVolumeDown, SIGNAL(clicked()), this, SLOT(onCTVolumeDown()));
    connect(ui->cbCTRepeat, SIGNAL(currentIndexChanged(int)), this, SLOT(onCTRepeatMode(int)));
    connect(ui->cbCTShuffle, SIGNAL(currentIndexChanged(int)), this, SLOT(onCTShuffleMode(int)));

    connect(ui->cbCTVolume, SIGNAL(currentIndexChanged(int)), this, SLOT(cbCTVolumeChanged(int)));
    connect(ui->btnCTConnect, SIGNAL(clicked()), this, SLOT(onCTConnect()));
    connect(ui->btnCTDisconnect, SIGNAL(clicked()), this, SLOT(onCTDisconnect()));
    connect(ui->btnCTANCSPositive, SIGNAL(clicked()), this, SLOT(OnBnClickedAncsPositive()));
    connect(ui->btnCTANCSNegative, SIGNAL(clicked()), this, SLOT(OnBnClickedAncsNegative()));

    ui->btnCTPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnCTPrev->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->btnCTStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnCTPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->btnCTNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->btnCTSeekFwd->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->btnCTSeekBack->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->btnCTMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));


    // Table for Music media player elements
    ui->tblMetaPlayerCT->horizontalHeader()->setStretchLastSection(true);
    ui->tblMetaPlayerCT->verticalHeader()->setVisible(true);
    ui->tblMetaPlayerCT->setRowCount(7);
    ui->tblMetaPlayerCT->setColumnCount(2);
    ui->tblMetaPlayerCT->setColumnWidth(0,120);
    ui->tblMetaPlayerCT->setColumnWidth(1,330);

    QStringList list;

    m_volMute = true;

    list.clear();
    list<<"Media Element"<<"Value";
    ui->tblMetaPlayerCT->setHorizontalHeaderLabels(list);

    // Mute is not available in stack
    ui->btnCTMute->setVisible(false);

    ui->cbCTRepeat->clear();
    ui->cbCTRepeat->addItem(repeatStr[APP_AVRC_PLAYER_VAL_OFF-1], APP_AVRC_PLAYER_VAL_OFF);
    ui->cbCTRepeat->addItem(repeatStr[APP_AVRC_PLAYER_VAL_SINGLE_REPEAT-1], APP_AVRC_PLAYER_VAL_SINGLE_REPEAT);
    ui->cbCTRepeat->addItem(repeatStr[APP_AVRC_PLAYER_VAL_ALL_REPEAT-1], APP_AVRC_PLAYER_VAL_ALL_REPEAT);
    //ui->cbCTRepeat->addItem(repeatStr[APP_AVRC_PLAYER_VAL_GROUP_REPEAT-1], APP_AVRC_PLAYER_VAL_GROUP_REPEAT);

    ui->cbCTShuffle->clear();
    ui->cbCTShuffle->addItem(shuffleStr[APP_AVRC_PLAYER_VAL_OFF-1], APP_AVRC_PLAYER_VAL_OFF);
    ui->cbCTShuffle->addItem(shuffleStr[APP_AVRC_PLAYER_VAL_ALL_SHUFFLE-1], APP_AVRC_PLAYER_VAL_ALL_SHUFFLE);
    //ui->cbCTShuffle->addItem(shuffleStr[APP_AVRC_PLAYER_VAL_GROUP_SHUFFLE-1], APP_AVRC_PLAYER_VAL_GROUP_SHUFFLE);

    m_current_volume_pct = 50;
    ui->cbCTVolume->clear();
    for(int i = 0; i < 101; i++)
    {
        char s[100];
        sprintf(s, "%d", i);
        ui->cbCTVolume->addItem(s);
    }
    ui->cbTGVolume->setCurrentIndex(m_current_volume_pct);

    setAVRCCTUI();
}

void MainWindow::setAVRCCTUI()
{
    /*
    ui->btnCTPlay->setEnabled(true);
    ui->btnCTStop->setEnabled(true);
    ui->btnCTPause->setEnabled(true);
    ui->btnCTPrev->setEnabled(true);
    ui->btnCTNext->setEnabled(true);
    ui->btnCTSeekBack->setEnabled(true);
    ui->btnCTSeekFwd->setEnabled(true);
    ui->btnCTMute->setEnabled(true);
    ui->btnCTVolumeUp->setEnabled(true);
    ui->btnCTVolumeDown->setEnabled(true);
    */

}

void MainWindow::onCTConnect()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
         return;
    }

    CBtDevice * pDev =(CBtDevice *)GetSelectedDevice();

    if (pDev == NULL)
        return;

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];

    Log("Sending  AVRC CT Connect Command, BDA: %02x:%02x:%02x:%02x:%02x:%02x",
           cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_CONNECT, cmd, commandBytes);
}

void MainWindow::onCTDisconnect()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if (NULL == pDev)
        return;

    cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
    cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;

    Log("Sending AVRC CT Disconnect Command, Handle: 0x%04x", pDev->m_avrc_handle);
    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_avrc_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_AVRC;
}

void MainWindow::onCTPlay()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_PLAY, cmd, commandBytes);
}

void MainWindow::onCTStop()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_STOP, cmd, commandBytes);
}

void MainWindow::onCTPause()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_PAUSE, cmd, commandBytes);
}

void MainWindow::onCTNext()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_NEXT_TRACK, cmd, commandBytes);
}

void MainWindow::onCTPrevious()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_PREVIOUS_TRACK, cmd, commandBytes);
}

void MainWindow::onCTVolumeUp()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_VOLUME_UP, cmd, commandBytes);
}

void MainWindow::onCTVolumeDown()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_VOLUME_DOWN, cmd, commandBytes);
}

void MainWindow::onCTMute()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    m_volMute = !m_volMute;

    ui->btnCTMute->setIcon(style()->standardIcon(m_volMute ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
}

void MainWindow::onCTRepeatMode(int index)
{
    if(index == (repeat_ct.curr_value -1))
        return;

    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    QVariant val = ui->cbCTRepeat->itemData(index);

    repeat_ct.curr_value = val.toUInt();
    cmd[commandBytes++] = (BYTE)repeat_ct.curr_value;

    Log("Sending  AVRCP controller Repeat setting change : %d", repeat_ct.curr_value);

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_SET_REPEAT_MODE, cmd, commandBytes);

}

void MainWindow::onCTShuffleMode(int index)
{
    if(index == (shuffle_ct.curr_value -1))
        return;

    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    QVariant val = ui->cbCTShuffle->itemData(index);
    shuffle_ct.curr_value = val.toUInt();
    cmd[commandBytes++] = (BYTE)shuffle_ct.curr_value;

    Log("Sending AVRCP controller Shuffle setting change  : %d", shuffle_ct.curr_value);
    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_SET_SHUFFLE_MODE, cmd, commandBytes);


}

void MainWindow::cbCTVolumeChanged(int index)
{
    if(m_current_volume_pct == index)
        return;

    m_current_volume_pct = index;

    BYTE    cmd[60];
    int     commandBytes = 0;
    int handle = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
        handle = pDev->m_avrc_handle;

    cmd[commandBytes++] = handle & 0xff;
    cmd[commandBytes++] = (handle >> 8) & 0xff;

    cmd[commandBytes++] = m_current_volume_pct+1;

    Log("Sending Audio volume. Handle: 0x%04x", handle);
    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_VOLUME_LEVEL, cmd, commandBytes);

}

void MainWindow::onCTSkipForward()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_BEGIN_FAST_FORWARD, cmd, commandBytes);
    QThread::sleep(2);
    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_END_FAST_FORWARD, cmd, commandBytes);
}

void MainWindow::onCTSkipBackward()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if(pDev)
    {
        cmd[commandBytes++] = pDev->m_avrc_handle & 0xff;
        cmd[commandBytes++] = (pDev->m_avrc_handle >> 8) & 0xff;
    }

    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_BEGIN_REWIND, cmd, commandBytes);
    QThread::sleep(2);
    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_END_REWIND, cmd, commandBytes);
}

void MainWindow::onHandleWicedEventAVRCCT(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_DEVICE:
        HandleDeviceEventsAVRCCT(opcode, p_data, len);
        break;

    case HCI_CONTROL_GROUP_AVRC_CONTROLLER:
        HandleAVRCControllerEvents(opcode, p_data, len);
        break;

    }
}

void MainWindow::HandleDeviceEventsAVRCCT(DWORD opcode, LPBYTE p_data, DWORD len)
{
    switch (opcode)
    {
        case HCI_CONTROL_EVENT_DEVICE_STARTED:
            break;
    }
    setAVRCCTUI();
}

void MainWindow::HandleAVRCControllerEvents(DWORD opcode, BYTE *p_data, DWORD len)
{
    BYTE     bda[6];
    UINT16 handle;
    uint16_t maxStrLen;
    BYTE     status;
    CBtDevice *device;

    switch (opcode)
    {
     case HCI_CONTROL_AVRC_CONTROLLER_EVENT_CONNECTED:
    {
        if(len >= 9)
        {
            for (int i = 0; i < 6; i++)
                bda[5 - i] = p_data[i];
            p_data += 6;

            status = *p_data++;

            handle = p_data[0] + (p_data[1] << 8);

            // find device in the list with received address and save the connection handle
            if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
                device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

            device->m_avrc_handle = handle;
            device->m_conn_type |= CONNECTION_TYPE_AVRC;

            Log("AVRCP Controller connected %02x:%02x:%02x:%02x:%02x:%02x handle %04x",
                bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], handle);
        }
        else
        {
            Log("AVRCP Controller connection failed");
        }
    }

    break;

    case HCI_CONTROL_AVRC_CONTROLLER_EVENT_DISCONNECTED:
    {
        handle = p_data[0] | (p_data[1] << 8);
        Log("AVRCP Controller disconnected handle %d", handle);
        device = FindInList(CONNECTION_TYPE_AVRC, handle, ui->cbDeviceList);
        if (device)
        {
            device->m_avrc_handle = NULL_HANDLE;
            device->m_conn_type &= ~CONNECTION_TYPE_AVRC;
        }
    }
        break;

    case HCI_CONTROL_AVRC_CONTROLLER_EVENT_PLAYER_CHANGE:
    {
        char player[100];
        int cnt = (len-2) < 100 ? (len-2) : 100;
        memset(player, 0, 100);
        snprintf(player, cnt, (char *)&p_data[2]);
        Log("HCI_CONTROL_AVRC_CONTROLLER_EVENT_PLAYER_CHANGE %s", player);
    }
        break;

    case HCI_CONTROL_AVRC_CONTROLLER_EVENT_SETTING_AVAILABLE:
        {
        Log("AVRCP Controller Player settings available handle: %04x, settings: 0x%x 0x%x 0x%x 0x%x",
                        p_data[0] + (p_data[1] << 8), p_data[2], p_data[3], p_data[4], p_data[5]);

            BYTE *p_setting = &p_data[1];
            const int MAX_RC_APP_SETTING_VALUE   = 4;

            int k;

            for (int i=1; i<=MAX_RC_APP_SETTING_VALUE; i++)
            {
                if (p_setting[i])
                {
                    int value = (int) p_setting[i];

                    if(i == 2)
                    {
                        ui->cbCTRepeat->clear();
                        k = 1;
                        while(value && k < 5)
                        {
                            value -= 1 << k;
                            ui->cbCTRepeat->addItem(repeatStr[k-1], k);
                            k++;
                        }
                    }

                    if(i == 3)
                    {
                        ui->cbCTShuffle->clear();
                        k = 1;
                        while(value && k < 4)
                        {
                            value -= 1 << k;
                            ui->cbCTShuffle->addItem(shuffleStr[k-1], k);
                            k++;
                        }
                    }
                }
            }
        }

        break;

    case HCI_CONTROL_AVRC_CONTROLLER_EVENT_SETTING_CHANGE:               /* AVRCP Controller Player setting changed */
        Log("AVRCP Controller Player setting change handle: %04x ",
                        p_data[0] + (p_data[1] << 8));

        for (int i = 0, j = 3; i < p_data[2]; i++)
        {
            Log("    attr: %02x setting: %02x ", p_data[j], p_data[j + 1]);

            switch (p_data[j++])
            {
            case 2: // Repeat Mode
                repeat_ct.curr_value = p_data[j++];
                Log("Repeat %s", repeatStr[repeat_ct.curr_value-1]);
                ui->cbCTRepeat->setCurrentIndex(repeat_ct.curr_value-1);
             break;

            case 3: // Shuffle Mode
                shuffle_ct.curr_value = p_data[j++];
                Log("Shuffle %s", shuffleStr[shuffle_ct.curr_value-1]);
                ui->cbCTShuffle->setCurrentIndex(shuffle_ct.curr_value-1);
                break;
            }
        }

        break;

    case HCI_CONTROL_AVRC_CONTROLLER_EVENT_PLAY_STATUS:
        Log("AVRCP Controller Play status handle: %04x status:%d", p_data[0] + (p_data[1] << 8), p_data[2]);
        Log(szAvrcPlayStatus[p_data[2]]);

        break;

    case HCI_CONTROL_AVRC_CONTROLLER_EVENT_CURRENT_TRACK_INFO:           /* AVRCP Controller Track Info event */
      {
        Log("AVRCP Controller Track Info handle: %04x status: %d Attr: %d",
            p_data[0] + (p_data[1] << 8), p_data[2], p_data[3]);

        maxStrLen = p_data[4] + (p_data[5] << 8);

        if (maxStrLen > 60)
            maxStrLen = 60;

        QString mediaName;
        QString mediaType;
        QTableWidgetItem *colType;
        QTableWidgetItem *colVal;
        char name[256] = {0};

        switch (p_data[3])
        {
        case 1: /* Title */
            mediaType = "Title";
            break;
        case 2: /* Artist */
            mediaType = "Artist";
            break;
        case 3: /* Album */
            mediaType = "Album";
            break;
        case 4: /* track number */
            mediaType = "Track Num";
            break;
        case 5: /* number of tracks */
            mediaType = "Total Tracks";
            break;
        case 6: /* Genre */
            mediaType = "Genre";
            break;
        case 7: /* playing time/track duration*/
            mediaType = "Duration (sec)";
            break;
        default:
            /* Unhandled */
            break;
        }

        if(!mediaType.length())
            break;

        if(maxStrLen)
            snprintf(name,  maxStrLen, "%s", &p_data[6]);
        else
            sprintf(name, "%s", "<not available>");

        Log("%s - %s", mediaType.toLocal8Bit().data(), name);

        mediaName = name;

        colType = new QTableWidgetItem(mediaType);
        colVal = new QTableWidgetItem(mediaName);

        ui->tblMetaPlayerCT->setItem(p_data[3] -1,0, colType);
        ui->tblMetaPlayerCT->setItem(p_data[3] -1,1, colVal);


    }
        break;

    default:
        /* Unhandled */
        break;
    }

        setAVRCCTUI();
}


void MainWindow::OnBnClickedAncsPositive()
{
    BYTE command[5] = { 0, 0, 0, 0, 0 };
    command[0] = m_notification_uid & 0xff;
    command[1] = (m_notification_uid >> 8) & 0xff;
    command[2] = (m_notification_uid >> 16) & 0xff;
    command[3] = (m_notification_uid >> 24) & 0xff;
    SendWicedCommand(HCI_CONTROL_ANCS_COMMAND_ACTION, command, 5);
}

void MainWindow::OnBnClickedAncsNegative()
{
    BYTE command[5] = { 0, 0, 0, 0, 1 };
    command[0] = m_notification_uid & 0xff;
    command[1] = (m_notification_uid >> 8) & 0xff;
    command[2] = (m_notification_uid >> 16) & 0xff;
    command[3] = (m_notification_uid >> 24) & 0xff;
    SendWicedCommand(HCI_CONTROL_ANCS_COMMAND_ACTION, command, 5);
}

