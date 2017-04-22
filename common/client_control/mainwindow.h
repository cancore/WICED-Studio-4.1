#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdio.h>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QComboBox>
#include <QSettings>
#include <QWaitCondition>
#include <QMessageBox>
#include <QThread>
#include <QMutex>
#ifndef uint16_t
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef unsigned int   uint32_t;
#endif

#ifndef DWORD
typedef unsigned int    DWORD ;
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned short  USHORT;
typedef bool            BOOL;
typedef wchar_t         WCHAR;
typedef unsigned char * LPBYTE;
typedef char            CHAR;
typedef unsigned long   ULONG;

#endif
#define FALSE   false
#define TRUE    true

#define CONNECTION_TYPE_NONE    0x0
#define CONNECTION_TYPE_AG      0x1
#define CONNECTION_TYPE_SPP     0x2
#define CONNECTION_TYPE_AUDIO   0x3
#define CONNECTION_TYPE_HF      0x4
#define CONNECTION_TYPE_HIDH    0x5
#define CONNECTION_TYPE_IAP2    0x6
#define CONNECTION_TYPE_LE      0x7
#define CONNECTION_TYPE_AVRC    0x8
#define CONNECTION_TYPE_AVK     0x9

#define NULL_HANDLE             0xFF
#define LE_DWORD(p) (((DWORD)(p)[0]) + (((DWORD)(p)[1])<<8) + (((DWORD)(p)[2])<<16) + (((DWORD)(p)[3])<<24))
typedef struct
{
    BYTE     *m_pWavData;
    BYTE     *m_pData;
    DWORD     m_dwWavDataLen;
    DWORD     m_dwChunkLen;
    DWORD     m_dwWavSent;
    DWORD     m_PacketsToSend; // incremented on receiving the message to send new buffers
    DWORD     m_PacketsSent;   // incremented in the write thread
    DWORD     m_BytesPerPacket;// received       
}hci_audio_sample_t;

class Worker;
// remote device information
class CBtDevice
{
public:
    CBtDevice (bool paired=false);
    ~CBtDevice ();

    UINT8 m_address[6];
    UINT8  address_type;
    UINT16 m_conn_type;
    UINT16 m_audio_handle;
    UINT16 m_hf_handle;
    UINT16 m_ag_handle;
    UINT16 m_spp_handle;
    UINT16 m_hidh_handle;
    UINT16 m_iap2_handle;
    UINT16 m_avrc_handle;
    UINT16 con_handle;
    UINT16 m_bsg_handle;
    UINT16 m_avk_handle;

    UINT8  role;

    bool m_paired;
    char m_name[100];
    int m_nvram_id;
    QByteArray m_nvram;

    bool m_bIsLEDevice;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT    

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString m_SettingsFile;
    QSettings m_settings;
    void closeEvent (QCloseEvent *event);
    void HandleDeviceEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    void Log(const char * tr,...);
    BYTE ProcNibble (char n);
    USHORT GetHandle(QString &str);
    DWORD GetHexValue(LPBYTE buf, DWORD buf_size, QString &str);

    // Device manager
    QMessageBox dl_msgbox;
    void WriteNVRAMToDevice(bool bBLEDevice);
    int FindBaudRateIndex(int baud);    
    void setPairingMode();
    void setVis();
    QSerialPort *m_CommPort;
    bool m_bPortOpen ;
    bool SetupCommPort();
    unsigned int SendWicedCommand(unsigned short command, unsigned char * payload, unsigned int len);
    void InitDm();
    void closeEventDm (QCloseEvent *event);
    void HandleDeviceEventsDm(DWORD opcode, LPBYTE p_data, DWORD len);
//    void HandleLEEventsDm(DWORD identifier, LPBYTE p_data, DWORD len);
    void DecodeEIR(LPBYTE p_data, DWORD len, char * szName, int name_len);
    void EnableUI(bool bEnable);
    bool m_bUIEnabled;
    void CloseCommPort();
    void ClearPort();
    QTimer *p_dm_timer;
    QWaitCondition serial_read_wait;
    bool m_scan_active;
    bool m_inquiry_active;
    void GetVersion();
    void HandleDeviceEventsMisc(DWORD opcode, LPBYTE p_data, DWORD len);
    UINT8 m_major;
    UINT8 m_minor;
    UINT8 m_rev;
    UINT8 m_build;
    uint32_t m_chip;
    UINT8 m_power;
    UINT16 m_features;

    void HandleA2DPEvents(DWORD opcode, DWORD len, BYTE *p_data);
    CBtDevice *AddDeviceToList(BYTE *addr, QComboBox * pCb, char * bd_name=NULL,bool bPaired=false);
    CBtDevice * FindInList(BYTE * addr, QComboBox * pCb);
    CBtDevice * FindInList(UINT16 conn_type, UINT16 handle, QComboBox * pCb);
    void SelectDevice(QComboBox* cb, BYTE * bda);
    CBtDevice * GetSelectedDevice();
    CBtDevice * GetSelectedLEDevice();
    void ResetDeviceList(QComboBox *cb);
    void onHandleWicedEventDm(unsigned int opcode, unsigned char*p_data, unsigned int len);
    void SetDevicePaired(BYTE * bda);
    QIcon m_paired_icon;
    FILE * m_fp_logfile;
    BOOL SendLaunchRam();
    BOOL SendDownloadMinidriver();
    void downlWoad(FILE * fHCD);
    //BOOL SendHcdRecord(ULONG nAddr, ULONG nHCDRecSize, BYTE * arHCDDataBuffer);
    void SendRecvCmd(BYTE *arHciCommandTx, int tx_sz, BYTE *arBytesExpectedRx, int rx_sz);
    DWORD ReadCommPort(BYTE *lpBytes, DWORD dwLen, QSerialPort * m_CommPort);
    void ReadDevicesFromSettings(const char *group, QComboBox *cbDevices, QPushButton *btnUnbond);

    // Serial port read
    QThread* m_port_read_thread;
    Worker* m_port_read_worker;
    void CreateReadPortThread();
    QMutex m_write;

    // audio source
    bool m_audio_connected;
    bool m_audio_started;

    bool m_volMute;
    FILE * m_fpAudioFile;
    hci_audio_sample_t m_uAudio;
    void InitAudioSrc();    
    void closeEventAudioSrc(QCloseEvent *event);
    void onHandleWicedEventAudioSrc(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleDeviceEventsAudioSrc(DWORD opcode, LPBYTE p_data, DWORD len);
    void HandleA2DPEventsAudioSrc(DWORD opcode, LPBYTE p_data, DWORD len);    
    void setAudioSrcUI();
    BYTE * ExecuteSetWavFile();
    void HandleA2DPAudioRequestEvent(BYTE * pu8Data, DWORD len);
    CBtDevice* GetConnectedAudioSrcDevice();
    BYTE* GetWavDataDataChunk(BYTE *pWavData, DWORD dwWavDataLen, DWORD *pdwDataLen);
    BYTE * ExecuteSetWavFile(char *pcFileName);
    BYTE* ReadFile(const char* FilePathName, DWORD *pdwWavDataLen);
    bool InitializeAudioFile();
    QMutex m_audio_packets;    
    QWaitCondition audio_tx_wait;

    // Hands-free
    void InitHF();
    void onHandleWicedEventHF(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleHFEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    void SendAtCmd(int nAtCmd, int num, char *atStr);
    CBtDevice* GetConnectedHFDevice();
    bool m_audio_connection_active;    
    int m_mic_cur_pos;
    int m_speaker_cur_pos;

    // SPP
    void InitSPP();
    void onHandleWicedEventSPP(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleSPPEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    DWORD SendFileThreadSPP();
    CBtDevice* GetConnectedSPPDevice();
    DWORD   m_spp_bytes_sent;
    DWORD   m_spp_total_to_send;
    BYTE    m_spp_tx_complete_result;    
    FILE   *m_spp_receive_file;
    DWORD m_hSppTxCompleteEvent;
    QWaitCondition spp_tx_wait;
    // SPP connection can happen from iAP2 tab or
    // SPP tab in the UI.
    BYTE  m_iContext;
    QThread* m_thread_spp;
    Worker* m_worker_spp;

    // AG
    void InitAG();
    void onHandleWicedEventAG(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleAgEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    bool m_ag_connection_active;
    CBtDevice* GetConnectedAGDevice();

    // BLE/BR HID Device
    void InitBLEHIDD();
    void onHandleWicedEventBLEHIDD(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleBLEHIDDEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    bool m_pairing_mode_active;


    // HID Host
    void HidhVirtualUnplug(uint16_t handle);
    void InitHIDH();
    void onHandleWicedEventHIDH(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleHIDHEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    void DumpMemory(BYTE * p_buf, int length);
    CBtDevice* GetConnectedHIDHDevice();

    // AVRC CT
    void InitAVRCCT();
    void onHandleWicedEventAVRCCT(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleDeviceEventsAVRCCT(DWORD opcode, LPBYTE p_data, DWORD len);
    void HandleAVRCControllerEvents(DWORD opcode, BYTE *p_data, DWORD len);
    void setAVRCCTUI();
    CBtDevice* GetConnectedAVRCDevice();


    // AVRC TG
    void InitAVRCTG();
    void onHandleWicedEventAVRCTG(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleDeviceEventsAVRCTG(DWORD opcode, LPBYTE p_data, DWORD len);
    void HandleAVRCTargetEvents(DWORD opcode, BYTE *p_data, DWORD len);
    void setAVRCTGUI();

    void SetTrack();
    void TrackInfo();
    void PlayerStatus();

    int m_current_volume_pct;

    //GATT
    void InitGATT();
    void onHandleWicedEventGATT(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleLEEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    void HandleGattEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    void setGATTUI();
    USHORT GetConHandle(QComboBox *pCombo);

    void  SetRole(CBtDevice *pDevice, uint8_t role);
    UINT8 GetRole(CBtDevice *pDevice);
    void  UpdateGattButtons(CBtDevice *pDevice);
    BOOL  m_advertisments_active;
    ULONG m_notification_uid;

    // BSG
    void InitBSG();
    void onHandleWicedEventBSG(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleBSGEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    DWORD SendFileThreadBSG();
    CBtDevice* GetConnectedBSGDevice();
    DWORD   m_bsg_bytes_sent;
    DWORD   m_bsg_total_to_send;
    BYTE    m_bsg_tx_complete_result;
    FILE   *m_bsg_receive_file;
    DWORD m_hBsgTxCompleteEvent;
    QWaitCondition bsg_tx_wait;

    USHORT  m_bsg_sent;
    USHORT  m_bsg_acked;
    DWORD   m_uart_tx_size;

    // HomeKit
    void InitHK();
    void onHandleWicedEventHK(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleHkEvent(DWORD opcode, LPBYTE p_data, DWORD len);
    void SendHciCommand(UINT16 command, USHORT handle, LPBYTE p, DWORD dwLen);

    void SetLightOnOff(BOOL on);
    void UpdateUI(USHORT handle, LPBYTE p, DWORD dwLen);
    void ShowMessage();
    bool m_bLightOn;
    uint m_nLightBrightness;
    uint m_nDoorState;
    uint m_nLockState;
    uint m_nLockTargetState;
    uint m_nIdentifyTimerCounter;
    QTimer *p_timer;

    // iAP2
    void InitiAP2();
    void onHandleWicedEventiAP2(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleiAP2PEvents(DWORD opcode, LPBYTE p_data, DWORD len);
    void HandleSPPEvents2(DWORD opcode, LPBYTE p_data, DWORD len);
    DWORD SendFileThreadiAP2();
    CBtDevice* GetConnectediAP2Device();
    CBtDevice * GetConnectediAP2orSPPDevice(UINT16 &handle, UINT16 &conn_type);
    DWORD   m_iap2_bytes_sent;
    DWORD   m_iap2_total_to_send;
    BYTE    m_iap2_tx_complete_result;
    FILE   *m_iap2_receive_file;
    DWORD m_hiap2TxCompleteEvent;
    QWaitCondition iap2_tx_wait;

    // audio sink
    void InitAudioSnk();
    void onHandleWicedEventAudioSnk(unsigned int opcode, unsigned char *p_data, unsigned int len);
    void HandleA2DPEventsAudioSnk(DWORD opcode, BYTE *p_data, DWORD len);
    CBtDevice* GetConnectedAudioSnkDevice();


signals:
   void HandleWicedEvent(unsigned int opcode, unsigned int len, unsigned char *p_data);
   void HandleTrace(QString *pTrace);
   void HandleLeAdvState(BYTE val);

private slots:

   // utility methods
   void processTrace(QString * trace);
   void onMsgBoxButton(QAbstractButton*btn);
   void onClear();
   void btnFindLogfileClicked();
   void btnLogToFileClicked(bool);
   void btnAddTraceClicked();
   void EnableAppTraces();
   void handleReadyRead();

    // Device manager
    void processAddDeviceItem(QVariant pCb, QString sz, QVariant qv);
    void processHandleLeAdvState(BYTE val);
    void btnClearClicked();
    void onHandleWicedEvent(unsigned int opcode, unsigned int len, unsigned char *p_data);
    void onStartDisc();
    void onStopDisc();
    void onReset();
    void OnBnClickedBREDRUnbond();
    void OnBnClickedLeUnbond();
    void onUnbond(QComboBox* cb);
    void onCommPortChange(QString newPort);
    void onDevChange(QString);
    void onLEDevChange(QString);
    void onDiscoverable(bool);
    void onConnectable(bool);
    void onPairable(bool);
    void on_btnOpenPort_clicked();
    void onDlProgress(QString *msg, int pktcnt, int bytecnt);
    void onDlDone(const QString &s);    
    void onDownload();
    void onFindPatchFile();
    void resetState();
    void on_dm_timer();

    // AV source
    void onDisconnectAudioSrc();
    void onConnectAudioSrc();
    void onFindAudioFile();
    void onStartAudio();
    void onStopAudio();

    // Hands-free
    void on_btnConnectHF_clicked();
    void on_btnDisconnectHF_clicked();
    void on_btnHFConnectAudio_clicked();
    void on_btnHFHangup_clicked();
    void on_btnHFAnswer_clicked();
    void on_btnHFRedial_clicked();
    void on_btnHFDial_clicked();
    void on_btnHFDTMF_clicked();
    void on_btnHFVoiceReco_clicked();
    void on_btnHFCallHeld_clicked();
    void on_horizontalSliderHFMic_sliderMoved(int position);
    void on_horizontalSliderHFSpeaker_sliderMoved(int position);

    // SPP
    void on_btnSPConnect_clicked();
    void on_btnSPPDisconnect_clicked();
    void on_btnSPPSend_clicked();
    void on_btnSPPBrowseSend_clicked();
    void on_btnSPPBrowseReceive_clicked();
    void on_cbSPPSendFile_clicked(bool checked);
    void on_cbSPPReceiveFile_clicked(bool checked);
    void on_cbSPPThreadComplete();

    // AG
    void on_btnAGConnect_clicked();
    void on_btnAGDisconnect_clicked();
    void on_btnAGAudioConnect_clicked();

    // BLE/BR HID Device
    void on_btnBLEHIDSendReport_clicked();
    void on_btnBLEHIDPairingMode_clicked();
    void on_btnBLEHIDConnect_clicked();
    void on_btnBLEHIDSendKey_clicked();
    void on_cbBLEHIDCapLock_clicked();
    void on_cbBLEHIDCtrl_clicked();
    void on_cbBLEHIDAlt_clicked();

    // HID Host
    void on_btnHIDHConnect_clicked();
    void on_btnHIDHDisconnect_clicked();
    void on_btnHIDHGetDesc_clicked();
    void on_cbHIDHProtocol_currentIndexChanged(int index);

    //AVRCP CT
    void onCTPlay();
    void onCTPause();
    void onCTStop();
    void onCTNext();
    void onCTPrevious();
    void onCTVolumeUp();
    void onCTVolumeDown();
    void onCTMute();
    void onCTConnect();
    void onCTDisconnect();
    void onCTSkipForward();
    void onCTSkipBackward();
    void onCTRepeatMode(int index);
    void onCTShuffleMode(int index);
    void cbCTVolumeChanged(int index);
    void on_cbTGVolume_currentIndexChanged(int index);

    // AVRCP TG
    void onTGPlay();
    void onTGPause();
    void onTGStop();
    void onTGNext();
    void onTGPrevious();
    void onTGConnect();
    void onTGDisconnect();
    void oncbTGShuffleCurrentIndexChanged(int index);
    void oncbTGRepeatCurrentIndexChanged(int index);

    //GATT
    void OnBnClickedAncsPositive();
    void OnBnClickedAncsNegative();

    void OnBnClickedDiscoverDevicesStart();
    void OnBnClickedDiscoverDevicesStop();
    void OnBnClickedLeConnect();
    void OnBnClickedLeCancelConnect();
    void OnBnClickedLeDisconnect();
    void OnBnClickedDiscoverServices();
    void OnBnClickedDiscoverCharacteristics();
    void OnBnClickedDiscoverDescriptors();
    void OnBnClickedStartStopAdvertisements();
    void OnBnClickedSendNotification();
    void OnBnClickedSendIndication();
    void OnBnClickedCharacteristicRead();
    void OnBnClickedCharacteristicWrite();
    void OnBnClickedCharacteristicWriteWithoutResponse();

    //BSG
    void on_btnBSGSend_clicked();
    void on_btnBSGBrowseSend_clicked();
    void on_btnBSGBrowseReceive_clicked();
    void on_cbBSGSendFile_clicked(bool checked);
    void on_cbBSGReceiveFile_clicked(bool checked);
    void on_cbBSGThreadComplete();

    // HomeKit
    void on_btnHKRead_clicked();
    void on_btnHKWrite_clicked();
    void on_btnHKList_clicked();
    void on_btnHKSwitch_clicked();
    void on_btnHKSet_clicked();
    void on_cbDoorState_currentIndexChanged(int index);
    void on_cbLockState_currentIndexChanged(int index);
    void on_cbLockTargetState_currentIndexChanged(int index);
    void on_timer();
    void on_btnHKFactoryReset_clicked();

    // iAP2
    void on_btniAPConnect_clicked();
    void on_btniAPSDisconnect_clicked();
    void on_btniAPSend_clicked();
    void on_cbiAPPSendFile_clicked();
    void on_btnSPPBrowseSend_2_clicked();
    void on_cbiAPReceiveFile_clicked();
    void on_btniAPBrowseReceive_clicked();
    void on_btniAPRead_clicked();
    void on_cbiAP2ThreadComplete();    
    void on_btniAP2ReadCert_clicked();
    void on_btniAP2GenSign_clicked();
    void on_btniAP2ConnectSPP_clicked();


    // audio sink
    void on_btnAVSinkConnect_clicked();
    void on_btnAvSinkDisconnect_clicked();



public:
    Ui::MainWindow *ui;
};

// Thread for SPP, iAP2 and serial port read
class Worker : public QObject
 {
     Q_OBJECT

public:
    explicit Worker() {}
    ~Worker(){}

    // Read serial port
    bool m_bClosing;
    DWORD Read(BYTE * lpBytes, DWORD dwLen);
    DWORD ReadNewHciPacket(BYTE * pu8Buffer, int bufLen, int * pOffset);
    MainWindow * m_pParent;    

 public slots:
     void process_spp();
     void process_bsg();
     void process_iap2();
     void read_serial_port_thread();

 signals:
     void finished();
     void HandleWicedEvent(DWORD opcode, DWORD len, BYTE *p_data);

 };

class WaveFileWriter : public QThread
{
    Q_OBJECT

public:
    explicit WaveFileWriter(MainWindow * pParent);
    ~WaveFileWriter() {}
    MainWindow * m_pParent;
    void SendNextWav(hci_audio_sample_t * puHci, int bytesPerPacket);
    BYTE* GetWavDataDataChunk(BYTE *pWavData, DWORD dwWavDataLen, DWORD *pdwDataLen);
    BYTE * ExecuteSetWavFile(char *pcFileName);


protected:
    void run() Q_DECL_OVERRIDE;
};

#endif // MAINWINDOW_H


