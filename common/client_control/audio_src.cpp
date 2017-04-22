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

// thread for reading WAV file

WaveFileWriter::WaveFileWriter(MainWindow * pParent) : QThread()
{
    m_pParent = pParent;
    moveToThread(this);
}


uint8_t *gu8AudioBuffer = NULL;
int gs32AudioBufferSize = 0;

// Send wav packets to serial port
void WaveFileWriter::SendNextWav(hci_audio_sample_t * puHci, int bytesPerPacket)
{
    int remaining = puHci->m_dwChunkLen - puHci->m_dwWavSent;

    BYTE au8Hdr[5] =
        { HCI_WICED_PKT, (BYTE)(HCI_CONTROL_AUDIO_DATA & 0xff), (BYTE)((HCI_CONTROL_AUDIO_DATA >> 8) & 0xff), (BYTE)(bytesPerPacket & 0xff), (BYTE)((bytesPerPacket >> 8) & 0xff) };

    int headerLen = 0;
    int written = 0;

    if (gs32AudioBufferSize < bytesPerPacket){
        if (gu8AudioBuffer){
            free(gu8AudioBuffer);
            gu8AudioBuffer = NULL;
        }
        gu8AudioBuffer = (uint8_t *)malloc(bytesPerPacket + sizeof(au8Hdr) + headerLen);
        gs32AudioBufferSize = bytesPerPacket;
    }

    memcpy(gu8AudioBuffer + written, au8Hdr, sizeof(au8Hdr));
    written += sizeof(au8Hdr);

    if (remaining >= bytesPerPacket){
        memcpy(gu8AudioBuffer + written, puHci->m_pData + puHci->m_dwWavSent, bytesPerPacket);
        written += bytesPerPacket;

        puHci->m_dwWavSent += bytesPerPacket;
    }
    else{
        memcpy(gu8AudioBuffer + written, puHci->m_pData + puHci->m_dwWavSent, remaining);
        written += remaining;

        // resetting the wav file to the origin.
        puHci->m_dwWavSent = 0;

        memcpy(gu8AudioBuffer + written, puHci->m_pData + puHci->m_dwWavSent, bytesPerPacket - remaining);
        written += bytesPerPacket - remaining;

        puHci->m_dwWavSent += bytesPerPacket - remaining;
    }

    m_pParent->m_audio_packets.lock();
    int sent = 0;
    int max = written;

    while ((sent + max) < written)
    {
        if  (-1 == m_pParent->SendWicedCommand(0, gu8AudioBuffer + sent, max))
        {
            m_pParent->Log("audio write failed");
            m_pParent->m_audio_packets.unlock();
            return;
        }        
        sent += max;
    }

    if  (-1 == m_pParent->SendWicedCommand(0, gu8AudioBuffer + sent, written - sent))
    {
        m_pParent->Log("audio write failed");
        m_pParent->m_audio_packets.unlock();
        return;
    }

    m_pParent->m_audio_packets.unlock();

    if (puHci->m_dwWavSent >= puHci->m_dwChunkLen){
        puHci->m_dwWavSent = 0;
    }
}

// Loop till the embedded app asks for audio data
void WaveFileWriter::run()
{
    int packetsToSend = 0;
    QMutex mutex;
    while (1)
    {
        mutex.lock();
        // wait for Event requesting an audio buffer
        m_pParent->audio_tx_wait.wait(&mutex);

        m_pParent->m_audio_packets.lock();

        packetsToSend = m_pParent->m_uAudio.m_PacketsToSend;

        m_pParent->m_audio_packets.unlock();

        while (packetsToSend > m_pParent->m_uAudio.m_PacketsSent)
        {
          if (m_pParent->m_uAudio.m_BytesPerPacket)
          {
              SendNextWav(&m_pParent->m_uAudio, m_pParent->m_uAudio.m_BytesPerPacket);
          }
          m_pParent->m_uAudio.m_PacketsSent++;
        }

        mutex.unlock();
    }
}

BYTE* MainWindow::ReadFile(const char* FilePathName, DWORD *pdwWavDataLen)
    {
    BYTE* res = NULL;
    FILE *fp = NULL;

    if (NULL == (fp = fopen(FilePathName, "rb")))
        qDebug("ReadFile: fopen failed. file=%hs\n", FilePathName);
    else
    {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (res = (BYTE*)malloc(size))
        {
            if (fread(res, 1, size, fp) != size)
    {
                free(res);
                res = NULL;
            }
            else
                *pdwWavDataLen = size;
        }
        fclose(fp);
    }
    return res;
    }

BYTE* MainWindow::GetWavDataDataChunk(BYTE *pWavData, DWORD dwWavDataLen, DWORD *pdwDataLen)
{
    BYTE* pData;
    DWORD dwChunkLen;

    do
    {
        // skip "RIFF", check RIFF chunk lenght and skip type ID (WAVE)
        if (dwWavDataLen < 12)
            break;
        if (0 != memcmp(pWavData, "RIFF", 4))
            break;
        dwChunkLen = LE_DWORD(pWavData + 4);
        if (dwChunkLen + 8 > dwWavDataLen)
            break;
        if (0 != memcmp(pWavData + 8, "WAVE", 4))
            break;
        pWavData += 12;
        dwWavDataLen -= 12;
        // find data chunk and return it to caller
        while (dwWavDataLen > 8)
        {
            dwChunkLen = LE_DWORD(pWavData + 4);
            if (dwChunkLen + 8 > dwWavDataLen)
                break;
            if (0 == memcmp(pWavData, "data", 4))
    {
                pData = pWavData + 8;
                *pdwDataLen = dwChunkLen;
                break;
            }
            pWavData += 8 + dwChunkLen;
            dwWavDataLen -= 8 + dwChunkLen;
        }
    } while (false);
    return pData;
    }


BYTE * MainWindow::ExecuteSetWavFile(char *pcFileName)
{
    // Read file with WAV data

    if (m_uAudio.m_pWavData)
    {
        free(m_uAudio.m_pWavData);

        m_uAudio.m_pWavData = NULL;
        m_uAudio.m_pData = NULL;
        m_uAudio.m_dwWavDataLen = 0;
        m_uAudio.m_dwChunkLen = 0;
        m_uAudio.m_dwWavSent = 0;
    }

    if (NULL == (m_uAudio.m_pWavData = ReadFile(pcFileName, &m_uAudio.m_dwWavDataLen)))
    {
        Log("Could not open audio file %s", pcFileName);
        return NULL;
    }

    //get Data chunk pointer and length
    if (NULL == (m_uAudio.m_pData = GetWavDataDataChunk(m_uAudio.m_pWavData, m_uAudio.m_dwWavDataLen, &m_uAudio.m_dwChunkLen)))
    {
        Log("Error: could not get the data section of the wav file %s ", pcFileName);
        return NULL;
    }


    return m_uAudio.m_pData;
}

WaveFileWriter * pWaveFileWriter=NULL;

void MainWindow::InitAudioSrc()
{
    m_audio_connected = false;
    m_audio_started = false;
    m_bPortOpen = false;
    m_fpAudioFile = NULL;
    memset(&m_uAudio, 0, sizeof(m_uAudio));

    // setup signals/slots
    connect(ui->btnStartAudio, SIGNAL(clicked()), this, SLOT(onStartAudio()));
    connect(ui->btnStopAudio, SIGNAL(clicked()), this, SLOT(onStopAudio()));
    connect(ui->btnConnectAudio, SIGNAL(clicked()), this, SLOT(onConnectAudioSrc()));
    connect(ui->btnDisconnectAudio, SIGNAL(clicked()), this, SLOT(onDisconnectAudioSrc()));
    connect(ui->btnFindAudioFile, SIGNAL(clicked()), this, SLOT(onFindAudioFile()));

    ui->edAudioFile->setText( m_settings.value("AudioFile","").toString());
    ui->rbAudioSrcFile->setChecked(m_settings.value("AudioSrcFile",false).toBool());
    ui->rbAudioSrcSine->setChecked(!m_settings.value("AudioSrcFile",false).toBool());
    setAudioSrcUI();

    ui->rbAudioModeMono->setChecked(false);
    ui->rbAudioModeSterio->setChecked(true);
    ui->cbSineFreq->setCurrentIndex(1); // 48.1 KHz

    pWaveFileWriter=new WaveFileWriter (this);
    pWaveFileWriter->start(QThread::TimeCriticalPriority);

}

void MainWindow::setAudioSrcUI()
{
    /*
    ui->btnConnectAudio->setEnabled(!m_audio_connected );
    ui->btnDisconnectAudio->setEnabled(m_audio_connected );
    ui->btnStartAudio->setEnabled(!m_audio_started & m_audio_connected );
    ui->btnStopAudio->setEnabled(m_audio_started & m_audio_connected );  
    ui->rbAudioSrcSine->setEnabled(!m_audio_connected);
    ui->rbAudioSrcFile->setEnabled(!m_audio_connected);
    ui->rbAudioModeMono->setEnabled(!m_audio_connected);
    ui->cbSineFreq->setEnabled(!m_audio_connected);
    ui->btnFindAudioFile->setEnabled(!m_audio_connected);
    ui->edAudioFile->setEnabled(!m_audio_connected);
    */
}

void MainWindow::closeEventAudioSrc(QCloseEvent *event)
{
    onDisconnectAudioSrc();
    m_settings.setValue("AudioSrcFile",ui->rbAudioSrcFile->isChecked());
}

void MainWindow::onDisconnectAudioSrc()
{
    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
        return;
    }

    CBtDevice * pDev =(CBtDevice *)GetConnectedAudioSrcDevice();
    if (NULL == pDev)
        return;

    BYTE    cmd[60];
    int     commandBytes = 0;
    USHORT  nHandle = pDev->m_audio_handle;
    int i = 0;
    for (i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending Audio Disconnect Command, Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_AUDIO_COMMAND_DISCONNECT, cmd, commandBytes);

    pDev->m_audio_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_AUDIO;
}

void MainWindow::onConnectAudioSrc()
{
    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
        return;
    }

    // connect audio source
    unsigned char    cmd[60];
    int     commandBytes = 0;    


    CBtDevice * pDev =(CBtDevice *)GetSelectedDevice();

    if (pDev == NULL)
    {
        Log("No device selected");
        return;
    }

    if(pDev->m_audio_handle != NULL_HANDLE)
    {
        Log("AV SRC already connected for selected device Handle: 0x%04x", pDev->m_audio_handle);
        return;
    }

    if (ui->rbAudioSrcFile->isChecked())
    {
        if (!InitializeAudioFile())
        {
            Log("InitializeAudioFile failed");
            return;
        }

    }

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];

    cmd[commandBytes++] = ui->rbAudioSrcFile->isChecked() ? 1 : 2;

    SendWicedCommand(HCI_CONTROL_AUDIO_COMMAND_CONNECT, cmd, commandBytes);

}

void MainWindow::onHandleWicedEventAudioSrc(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_DEVICE:
        HandleDeviceEventsAudioSrc(opcode, p_data, len);
        break;

    case HCI_CONTROL_GROUP_AUDIO:
        HandleA2DPEventsAudioSrc(opcode, p_data, len);
        break;
    }
}

void MainWindow::HandleDeviceEventsAudioSrc(DWORD opcode, LPBYTE p_data, DWORD len)
{
    switch (opcode)
    {
        case HCI_CONTROL_EVENT_DEVICE_STARTED:
            m_audio_connected = false;
            m_audio_started = false;
            break;
    }
    setAudioSrcUI();
}

#define MAX_PATH          260

bool MainWindow::InitializeAudioFile()
{
    char audioFile[MAX_PATH] = { 0 };
    strcpy(audioFile, ui->edAudioFile->text().toStdString().c_str());

    if (!ExecuteSetWavFile(audioFile)){
        return false;
    }

    return true;
}



void MainWindow::HandleA2DPAudioRequestEvent(BYTE * pu8Data, DWORD len)
{
    int bytes_per_packet = pu8Data[0] | (pu8Data[1] << 8);
    int num_packets = pu8Data[2];

    m_uAudio.m_BytesPerPacket = bytes_per_packet;

    m_audio_packets.lock();
    m_uAudio.m_PacketsToSend += num_packets;

    m_audio_packets.unlock();


    if (pWaveFileWriter == NULL)
    {
        Log("thread not running\n");
        return;
    }

    if (!m_uAudio.m_pWavData)        
    {
        Log("Setup the wave file to send using wavefile <wavfile.wav>\n");
        return;       
    }

    // signal the thread to send down data to embedded app
    audio_tx_wait.wakeAll();
}

void MainWindow::HandleA2DPEventsAudioSrc(DWORD opcode, BYTE *p_data, DWORD len)
{
    BYTE       bda[6];
    CBtDevice *device;
    UINT16     handle;

    switch (opcode)
    {
    case HCI_CONTROL_AUDIO_EVENT_CONNECTED:
    {
        int i = 0;
        for (i = 0; i < 6; i++)
            bda[5 - i] = p_data[i];        

        handle = p_data[i++] + (p_data[i++] << 8);

        // find device in the list with received address and save the connection handle
        if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
            device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

        device->m_audio_handle = handle;
        device->m_conn_type |= CONNECTION_TYPE_AUDIO;

        SelectDevice(ui->cbDeviceList, bda);
        m_audio_connected = true;

        Log("Audio Connected, Handle: 0x%04x", handle);

        // if connect then device must be paired
        if (!device->m_paired)
            SetDevicePaired(device->m_address);

        setAudioSrcUI();
    }
        break;

    case HCI_CONTROL_AUDIO_EVENT_DISCONNECTED:
    {
        m_audio_connected = false;
        handle = p_data[0] | (p_data[1] << 8);
        CBtDevice * pDev = FindInList(CONNECTION_TYPE_AUDIO, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_audio_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_AUDIO;
        }
        m_audio_started = false;
        Log("Audio disconnected, Handle: 0x%04x", handle);
        setAudioSrcUI();
    }
        break;

    case HCI_CONTROL_AUDIO_EVENT_STARTED:
        Log("Audio started");
        m_audio_started = true;
        if (ui->rbAudioSrcFile->isChecked() && (m_uAudio.m_pWavData == NULL))
            InitializeAudioFile();
        setAudioSrcUI();
        break;

    case HCI_CONTROL_AUDIO_EVENT_STOPPED:
        Log("Audio stopped");
        m_audio_started = false;

        setAudioSrcUI();
        break;

    case HCI_CONTROL_AUDIO_EVENT_REQUEST_DATA:        
        if (m_uAudio.m_pWavData != NULL)
            HandleA2DPAudioRequestEvent(p_data, len);
        break;

    case HCI_CONTROL_AUDIO_EVENT_COMMAND_COMPLETE:
        Log("Audio event command complete");
        break;

    case HCI_CONTROL_AUDIO_EVENT_COMMAND_STATUS:
        Log("Audio event command status");
        break;

    case HCI_CONTROL_AUDIO_EVENT_CONNECTION_FAILED:
        Log("Audio event connection attempt failed (0x%X)", opcode);
        break;

    default:
        Log("Rcvd cmd: %d (0x%X)", opcode, opcode);
        /* Unhandled */
        break;
    }

}

void MainWindow::onStartAudio()
{
    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedAudioSrcDevice();

    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_audio_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    cmd[commandBytes++] = ui->cbSineFreq->currentIndex();
    cmd[commandBytes++] = ui->rbAudioModeMono->isChecked() ? 0 : 1; // 0 - mono, 1 - stereo

    if (!m_audio_started)
    {
        if (ui->rbAudioSrcFile->isChecked())
        {
            if (!InitializeAudioFile())
            {
                return;
            }
        }
    }

    Log("Sending Audio Start Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_AUDIO_START, cmd, commandBytes);
}

void MainWindow::onStopAudio()
{
    if (!m_audio_started)
        return;

    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedAudioSrcDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_audio_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    Log("Sending Audio stop Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_AUDIO_STOP, cmd, commandBytes);
}

void MainWindow::onFindAudioFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Audio File"), "", tr("Audio Files (*.wav)"));
    ui->edAudioFile->setText(fileName);
    m_settings.setValue("AudioFile",fileName);
}

CBtDevice* MainWindow::GetConnectedAudioSrcDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_audio_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as AV SRC");
        return NULL;
    }

    return pDev;
}
