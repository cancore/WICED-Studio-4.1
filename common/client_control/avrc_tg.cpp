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


const char *table_rows[] =
{
    "Title",
    "Artist",
    "Album",
    "Genre",
    "Track num",
    "Num Tracks",
    "Playing Time",
};

/* Sample song, first song */
tAPP_AVRC_ITEM_MEDIA song1 =
{
    //.attr_count =
    APP_AVRC_MAX_NUM_MEDIA_ATTR_ID,
    {
        //.p_attr_list[0] =
        {   APP_AVRC_MEDIA_ATTR_ID_TITLE,
            {   /* The player name, name length and character set id.*/
                8,
                (uint8_t *)"Beat It ",
            },
        },

        //.p_attr_list[1] =
        {
            APP_AVRC_MEDIA_ATTR_ID_ARTIST,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"Michael Jackson "
            },
        },

        //.p_attr_list[2] =
        {
            APP_AVRC_MEDIA_ATTR_ID_ALBUM,
            {   /* The player name, name length and character set id.*/
                8,
                (uint8_t *)"Thriller"
            },
        },

        //.p_attr_list[3] =
        {
            APP_AVRC_MEDIA_ATTR_ID_GENRE,
            {   /* The player name, name length and character set id.*/
                8,
                (uint8_t *)"SoftRock"
            },
        },

        //.p_attr_list[4] =
        {
            APP_AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
            {   /* The player name, name length and character set id.*/
                1,
                (uint8_t *)"3"
            },
        },

        //.p_attr_list[5] =
        {
            APP_AVRC_MEDIA_ATTR_ID_TRACK_NUM,
            {   /* The player name, name length and character set id.*/
                1,
                (uint8_t *)"1"
            },
        },

        //.p_attr_list[6] =
        {
            APP_AVRC_MEDIA_ATTR_ID_PLAYING_TIME,
            {   /* The player name, name length and character set id.*/
                6,
                (uint8_t *)"269000"
            },
        },
    },

};

/* Sample song, second song */
tAPP_AVRC_ITEM_MEDIA song2 =
{
    //.attr_count =
    APP_AVRC_MAX_NUM_MEDIA_ATTR_ID,

     {
        //.p_attr_list[0] =
        {
            APP_AVRC_MEDIA_ATTR_ID_TITLE,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"Happy Nation    "
            },
        },

        //.p_attr_list[1] =
        {
            APP_AVRC_MEDIA_ATTR_ID_ARTIST,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"Jonas, Jenny    "
            },
        },


        //.p_attr_list[2] =
        {
            APP_AVRC_MEDIA_ATTR_ID_ALBUM,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"Ace of Base     "
            },
        },

        //.p_attr_list[3] =
        {
            APP_AVRC_MEDIA_ATTR_ID_GENRE,
            {   /* The player name, name length and character set id.*/
                4,
                (uint8_t *)"Pop "
            },
        },

        //.p_attr_list[4] =
        {
            APP_AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
            {   /* The player name, name length and character set id.*/
                1,
                (uint8_t *)"3"
            },
        },

        //.p_attr_list[5] =
        {
            APP_AVRC_MEDIA_ATTR_ID_TRACK_NUM,
            {   /* The player name, name length and character set id.*/
                1,
                (uint8_t *)"2"
            },
        },

        //.p_attr_list[6] =
        {
            APP_AVRC_MEDIA_ATTR_ID_PLAYING_TIME,
            {   /* The player name, name length and character set id.*/
                6,
                (uint8_t *)"255000"
            },
        },
     },


};

/* Sample song, third song */
tAPP_AVRC_ITEM_MEDIA song3 =
{

    //.attr_count =
    APP_AVRC_MAX_NUM_MEDIA_ATTR_ID,

    {
        // .p_attr_list[0] =
        {
            APP_AVRC_MEDIA_ATTR_ID_TITLE,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"River of Dreams "
            },
        },


    //.p_attr_list[1] =

        {
           APP_AVRC_MEDIA_ATTR_ID_ARTIST,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"Billy Joel      "
            },
        },


        //.p_attr_list[2] =
        {
            APP_AVRC_MEDIA_ATTR_ID_ALBUM,
            {   /* The player name, name length and character set id.*/
                16,
                (uint8_t *)"River of Dreams "
            },
        },

        //.p_attr_list[3] =
        {
            APP_AVRC_MEDIA_ATTR_ID_GENRE,
            {   /* The player name, name length and character set id.*/
                4,
                (uint8_t *)"Soul"
            },
        },

        //.p_attr_list[4] =
        {
            APP_AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
            {   /* The player name, name length and character set id.*/
                1,
                (uint8_t *)"3"
            },
        },

        //.p_attr_list[5] =
        {
            APP_AVRC_MEDIA_ATTR_ID_TRACK_NUM,
            {   /* The player name, name length and character set id.*/
                1,
                (uint8_t *)"3"
            },
        },

        //.p_attr_list[6] =
        {
            APP_AVRC_MEDIA_ATTR_ID_PLAYING_TIME,
            {   /* The player name, name length and character set id.*/
                6,
                (uint8_t *)"211000"
            },
        },
     },
};

/* list of songs */
tAPP_AVRC_ITEM_MEDIA * app_avrc_songs_list[] =
{
    &song1,
    &song2,
    &song3
};


/* current play state */
uint8_t avrc_app_play_state = APP_AVRC_PLAYSTATE_STOPPED;

tAPP_AVRC_META_ATTRIB     repeat_tg;
tAPP_AVRC_META_ATTRIB     shuffle_tg;

/* Currently playing song */
uint8_t app_avrc_cur_play_index = 0;
//*************************************************************************************/

void DumpData(char *description, void* p, unsigned int length, unsigned int max_lines);


void MainWindow::InitAVRCTG()
{
    ui->btnTGPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnTGPrev->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->btnTGStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnTGPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->btnTGNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    // setup signals/slots
    connect(ui->btnTGPlay, SIGNAL(clicked()), this, SLOT(onTGPlay()));
    connect(ui->btnTGStop, SIGNAL(clicked()), this, SLOT(onTGStop()));
    connect(ui->btnTGPause, SIGNAL(clicked()), this, SLOT(onTGPause()));
    connect(ui->btnTGPrev, SIGNAL(clicked()), this, SLOT(onTGPrevious()));
    connect(ui->btnTGNext, SIGNAL(clicked()), this, SLOT(onTGNext()));
    connect(ui->btnTGConnect, SIGNAL(clicked()), this, SLOT(onTGConnect()));
    connect(ui->btnTGDisconnect, SIGNAL(clicked()), this, SLOT(onTGDisconnect()));

    connect(ui->cbTGRepeat, SIGNAL(currentIndexChanged(int)), this, SLOT(oncbTGRepeatCurrentIndexChanged(int)));
    connect(ui->cbTGShuffle, SIGNAL(currentIndexChanged(int)), this, SLOT(oncbTGShuffleCurrentIndexChanged(int)));

    // Table for Music media player elements
    ui->tblMetaPlayerTG->horizontalHeader()->setStretchLastSection(true);
    ui->tblMetaPlayerTG->verticalHeader()->setVisible(true);
    ui->tblMetaPlayerTG->setRowCount(7);
    ui->tblMetaPlayerTG->setColumnCount(2);
    ui->tblMetaPlayerTG->setColumnWidth(0,120);
    ui->tblMetaPlayerTG->setColumnWidth(1,330);


    QStringList list;

    list.clear();
    list<<"Media Element"<<"Value";
    ui->tblMetaPlayerTG->setHorizontalHeaderLabels(list);

    ui->cbTGRepeat->clear();
    ui->cbTGRepeat->addItem("OFF", APP_AVRC_PLAYER_VAL_OFF);
    ui->cbTGRepeat->addItem("Single", APP_AVRC_PLAYER_VAL_SINGLE_REPEAT);
    ui->cbTGRepeat->addItem("All", APP_AVRC_PLAYER_VAL_ALL_REPEAT);
    ui->cbTGRepeat->addItem("Group", APP_AVRC_PLAYER_VAL_GROUP_REPEAT);
    ui->cbTGRepeat->setCurrentIndex(0);

    ui->cbTGShuffle->clear();
    ui->cbTGShuffle->addItem("OFF", APP_AVRC_PLAYER_VAL_OFF);
    ui->cbTGShuffle->addItem("All", APP_AVRC_PLAYER_VAL_ALL_SHUFFLE);
    ui->cbTGShuffle->addItem("Group", APP_AVRC_PLAYER_VAL_GROUP_SHUFFLE);
    ui->cbTGShuffle->setCurrentIndex(0);

    m_current_volume_pct = 50;
    ui->cbTGVolume->clear();
    for(int i = 0; i < 101; i++)
    {
        char s[100];
        sprintf(s, "%d", i);
        ui->cbTGVolume->addItem(s);
    }
    ui->cbTGVolume->setCurrentIndex(m_current_volume_pct);

    SetTrack();


    setAVRCTGUI();
}

void MainWindow::setAVRCTGUI()
{
    //(!m_audio_started & m_audio_connected)
    ui->btnTGPlay->setEnabled(true);
    ui->btnTGStop->setEnabled(true);
    ui->btnTGPause->setEnabled(true);
    ui->btnTGPrev->setEnabled(true);
    ui->btnTGNext->setEnabled(true);
    ui->btnTGConnect->setEnabled(true);
    ui->btnTGDisconnect->setEnabled(true);
}

void MainWindow::onTGConnect()
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

    SendWicedCommand(HCI_CONTROL_AVRC_TARGET_COMMAND_CONNECT, cmd, commandBytes);
}

void MainWindow::onTGDisconnect()
{
    BYTE    cmd[60];
    int     commandBytes = 0;

    CBtDevice * pDev =(CBtDevice *)GetConnectedAVRCDevice();
    if (NULL == pDev)
        return;

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];

    SendWicedCommand(HCI_CONTROL_AVRC_TARGET_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_avrc_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_AVRC;
}

void MainWindow::onTGPlay()
{
    avrc_app_play_state = APP_AVRC_PLAYSTATE_PLAYING;
    PlayerStatus();
    onStartAudio();
    Log("Starting to play");


}

void MainWindow::onTGStop()
{
//    if (!m_audio_started)
//        return;
    avrc_app_play_state = APP_AVRC_PLAYSTATE_STOPPED;
    PlayerStatus();
    onStopAudio();
    Log("Audio stop");


}

void MainWindow::onTGPause()
{
    Log("Pausing playback");
    avrc_app_play_state = APP_AVRC_PLAYSTATE_PAUSED;
    PlayerStatus();
    onStopAudio();

}

// User clicked Previous button
void MainWindow::onTGPrevious()
{
    Log("Setting PREV TRACK");

    if(app_avrc_cur_play_index == 0)
        app_avrc_cur_play_index = APP_AVRC_NUMSONGS-1;
    else
        app_avrc_cur_play_index--;
    /* show track info on UI, and send the updated track info to the embedded app */
    SetTrack();
    TrackInfo();

}

// User clicked Next button
void MainWindow::onTGNext()
{
    Log("Setting NEXT TRACK");

    app_avrc_cur_play_index++;
    if(app_avrc_cur_play_index > APP_AVRC_NUMSONGS-1)
        app_avrc_cur_play_index = 0;
    /* show track info on UI, and send the updated track info to the embedded app */
    SetTrack();
    TrackInfo();
}

void MainWindow::oncbTGRepeatCurrentIndexChanged(int index)
{
    BYTE cmd[1];
    int  commandBytes = 0;

    QVariant val = ui->cbTGRepeat->itemData(index);

    repeat_tg.curr_value = val.toUInt();
    cmd[commandBytes++] = (BYTE)repeat_tg.curr_value;

    Log("Sending  AVRCP target Repeat setting change : %d", repeat_tg.curr_value);

    SendWicedCommand(HCI_CONTROL_AVRC_TARGET_COMMAND_REPEAT_MODE_CHANGE, cmd, commandBytes);

}

void MainWindow::oncbTGShuffleCurrentIndexChanged(int index)
{
    BYTE cmd[1];
    int    commandBytes = 0;

    QVariant val = ui->cbTGShuffle->itemData(index);
    shuffle_tg.curr_value = val.toUInt();
    cmd[commandBytes++] = (BYTE)shuffle_tg.curr_value;


    Log("Sending AVRCP target Shuffle setting change  : %d", shuffle_tg.curr_value);
    SendWicedCommand(HCI_CONTROL_AVRC_TARGET_COMMAND_SHUFFLE_MODE_CHANGE, cmd, commandBytes);
}

/* Send the current track info to the embedded app */
void MainWindow::TrackInfo()
{
    BYTE     cmd[1024];
    uint16_t commandBytes = 0;

    memset(&cmd[0], 0, sizeof(cmd));

    /* send the track information for currently playing song */
    for (int xx=0; xx < app_avrc_songs_list[app_avrc_cur_play_index]->attr_count; xx++)
    {
        cmd[commandBytes++] = (uint8_t)app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[xx].attr_id;;
        cmd[commandBytes++] = (uint8_t)app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[xx].name.str_len;;

        memcpy( &cmd[commandBytes],
                app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[xx].name.p_str,
                app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[xx].name.str_len);
        commandBytes += app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[xx].name.str_len;
    }

    Log("TrackInfo : [%x] commandBytes: %d", HCI_CONTROL_AVRC_TARGET_COMMAND_TRACK_INFO, commandBytes);

    SendWicedCommand(HCI_CONTROL_AVRC_TARGET_COMMAND_TRACK_INFO, cmd, commandBytes);
}

void MainWindow::onHandleWicedEventAVRCTG(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_DEVICE:
        HandleDeviceEventsAVRCTG(opcode, p_data, len);
        break;

    case HCI_CONTROL_GROUP_AVRC_TARGET:
        HandleAVRCTargetEvents(opcode, p_data, len);
        break;
    }
}

void MainWindow::HandleDeviceEventsAVRCTG(DWORD opcode, LPBYTE p_data, DWORD len)
{
    switch (opcode)
    {
        case HCI_CONTROL_EVENT_DEVICE_STARTED:
//            m_audio_connected = false;
//            m_audio_started = false;
            break;
    }
    setAVRCTGUI();
}

void MainWindow::HandleAVRCTargetEvents(DWORD opcode, BYTE* p_data, DWORD len)
{
    BYTE     bda[6];
    UINT16 handle;
    CBtDevice *device;
    char   trace[1024];

    switch (opcode)
    {
    case HCI_CONTROL_AVRC_TARGET_EVENT_CONNECTED:
        {
            for (int i = 0; i < 6; i++)
                bda[5 - i] = p_data[i];
            p_data += 6;
            handle = p_data[0] + (p_data[1] << 8);

            // find device in the list with received address and save the connection handle
            if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
                device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

            device->m_avrc_handle = handle;
            device->m_conn_type |= CONNECTION_TYPE_AVRC;

            Log("AVRC connected %02x:%02x:%02x:%02x:%02x:%02x handle %04x",
                     bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], handle);

            /* When AVRC is connected, send current application info to embedded app */
            TrackInfo();
            PlayerStatus();
        }
        break;
    case HCI_CONTROL_AVRC_TARGET_EVENT_DISCONNECTED:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "[Handle: %u] Rcvd Event 0x12 - HCI_CONTROL_AVRC_TARGET_EVENT_DISCONNECTED", handle);
        Log(trace);
        CBtDevice * pDev = FindInList(CONNECTION_TYPE_AVRC, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_avrc_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_AVRC;
        }

        break;
    }
    case HCI_CONTROL_AVRC_TARGET_EVENT_PLAY:
        Log("AVRC Play handle:%04x", p_data[0] + (p_data[1] << 8));
        //m_trace->SetCurSel(m_trace->AddString(trace));

        avrc_app_play_state = APP_AVRC_PLAYSTATE_PLAYING;

        // If audio streaming is not started send a start
        onTGPlay();

        break;
    case HCI_CONTROL_AVRC_TARGET_EVENT_PAUSE:
        Log("AVRC Pause handle:%04x", p_data[0] + (p_data[1] << 8));      

        avrc_app_play_state = APP_AVRC_PLAYSTATE_PAUSED;

        // If audio streaming is started send a stop
        onTGStop();
        break;
    case HCI_CONTROL_AVRC_TARGET_EVENT_STOP:
        Log("AVRC Stop handle:%04x", p_data[0] + (p_data[1] << 8));        

        avrc_app_play_state = APP_AVRC_PLAYSTATE_STOPPED;

        // If audio streaming is started send a stop
        onTGStop();
        break;


    case HCI_CONTROL_AVRC_TARGET_EVENT_NEXT_TRACK:
        Log("AVRC Next handle:%04x", p_data[0] + (p_data[1] << 8));

        app_avrc_cur_play_index++;
        if(app_avrc_cur_play_index > (APP_AVRC_NUMSONGS-1))
            app_avrc_cur_play_index = 0;
        /* show track info on UI, and send the updated track info to the embedded app */
        SetTrack();
        TrackInfo();

        break;
    case HCI_CONTROL_AVRC_TARGET_EVENT_PREVIOUS_TRACK:
        Log("AVRC Previous handle:%04x", p_data[0] + (p_data[1] << 8));

        if(app_avrc_cur_play_index == 0)
            app_avrc_cur_play_index = APP_AVRC_NUMSONGS-1;
        else
            app_avrc_cur_play_index--;
        /* show track info on UI, and send the updated track info to the embedded app */
        SetTrack();
        TrackInfo();

        break;

    case HCI_CONTROL_AVRC_TARGET_EVENT_VOLUME_LEVEL:
    {
        Log("AVRC Volume handle:%04x Level: %d", p_data[0] + (p_data[1] << 8), p_data[2]);

        int vol = p_data[2];
        if(vol != m_current_volume_pct)
        {
            m_current_volume_pct = vol;
            ui->cbTGVolume->setCurrentIndex(m_current_volume_pct);

            ui->cbCTVolume->setCurrentIndex(m_current_volume_pct);
        }
        break;
    }

    case HCI_CONTROL_AVRC_TARGET_EVENT_REPEAT_SETTINGS:
        {
            /* Peer device changed repeat settings*/
            repeat_tg.curr_value = p_data[0];

            // Peer changed Player Application Settings. Update the UI to reflect the new value.
            // find the combo box item corresponding to the new value and set that item as selected
            int icnt ;
            icnt = ui->cbTGRepeat->count();

            for(int i = 0; i < icnt; i++)
            {
                QVariant val = ui->cbTGRepeat->itemData(i);
                if(val.toUInt() == repeat_tg.curr_value)
                {
                    ui->cbTGRepeat->setCurrentIndex(i);
                    break;
                }
            }
            Log("Repeat value changed by peer, %d", repeat_tg.curr_value);

       }
       break;

    case HCI_CONTROL_AVRC_TARGET_EVENT_SHUFFLE_SETTINGS:
        {
            /* Peer device changed shuffle settings*/            
            shuffle_tg.curr_value = p_data[0];

            // Peer changed Player Application Settings. Update the UI to reflect the new value.
            // find the combo box item corresponding to the new value and set that item as selected
            int icnt ;
            icnt = ui->cbTGShuffle->count();

            for(int i = 0; i < icnt; i++)
            {
                QVariant val = ui->cbTGShuffle->itemData(i);
                if(val.toUInt() == shuffle_tg.curr_value)
                {
                    ui->cbTGShuffle->setCurrentIndex(i);
                    break;
                }
            }
            Log("Shuffle value changed by peer, %d", shuffle_tg.curr_value);
        }
        break;

    default:
        /* Unhandled */
        break;
    }

    setAVRCTGUI();
}

/* Show the track info on the UI */
void MainWindow::SetTrack()
{
    QString strTitle = (char *)app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[0].name.p_str;

    for(int i = 0; i < 7; i++)
    {
        QString strCol1 = table_rows[i];
        QTableWidgetItem *col1 = new QTableWidgetItem(strCol1);
        QString strCol2 = (char *)app_avrc_songs_list[app_avrc_cur_play_index]->p_attr_list[i].name.p_str;
        QTableWidgetItem *col2 = new QTableWidgetItem(strCol2);
        ui->tblMetaPlayerTG->setItem(i, 0, col1);
        ui->tblMetaPlayerTG->setItem(i, 1, col2);
    }

    Log("Song: %s", strTitle.toLocal8Bit().data());

}

/* Send player status info to the embedded app */
void MainWindow::PlayerStatus()
{
    /* send play status info to the application */
    BYTE     cmd[16];
    uint16_t commandBytes = 0;
    uint32_t songlen, songpos;

    int sample_sel = ui->cbSineFreq->currentIndex();; // (uint32_t)((CComboBox *)GetDlgItem(IDC_AUDIO_FREQUENCY))->GetCurSel();


    uint32_t sample_freq = (sample_sel == 0) ? 44100 :
                           (sample_sel == 1) ? 48000 : 48000;

    songlen = m_uAudio.m_dwChunkLen / ((sample_freq*2*2)/1000);
    songpos = 0; //APP_AVRC_ATTR_SONG_POS;

    Log("PlayStatus : [%x] state: %d posn: %d len %d",
        HCI_CONTROL_AVRC_TARGET_COMMAND_PLAYER_STATUS, avrc_app_play_state, songpos, songlen);

    cmd[commandBytes++] = avrc_app_play_state;

    uint32_t *p_cmdbyte = (uint32_t *)&cmd[commandBytes];

    p_cmdbyte[0] = songlen;
    p_cmdbyte[1] = songpos;

    commandBytes += 8;

    SendWicedCommand(HCI_CONTROL_AVRC_TARGET_COMMAND_PLAYER_STATUS, cmd, commandBytes);
}

CBtDevice* MainWindow::GetConnectedAVRCDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_avrc_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as AVRC");
        return NULL;
    }

    return pDev;
}

void MainWindow::on_cbTGVolume_currentIndexChanged(int index)
{    
    if(m_current_volume_pct == index)
        return;

    m_current_volume_pct = index;

    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedAVRCDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_avrc_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;
    cmd[commandBytes++] = m_current_volume_pct+1;


    Log("Sending Audio volume. Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_AVRC_CONTROLLER_COMMAND_VOLUME_LEVEL, cmd, commandBytes);
}
