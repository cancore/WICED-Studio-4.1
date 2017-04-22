#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QtSerialPort/QSerialPort>
#include "mainwindow.h"

#define WAIT_TIMEOUT    3000

class DownloadThread : public QThread
{
    Q_OBJECT

public:
    DownloadThread(QObject *parent = 0);
    ~DownloadThread();
    void run();
    void download(MainWindow *pParent, QString &fn);
    void stop()
    {
        bStop = true;
    }
    bool getSuccess()
    {
        return success;
    }

signals:
    void dlProgress(QString* msg,int pkt_cnt, int byte_cnt);
    void dlDone(const QString &s);

private:
    bool send_hci_reset(QSerialPort & serial);
    bool ReadNextHCDRecord(FILE * fHCD, ULONG * nAddr, ULONG * nHCDRecSize, uint8_t * arHCDDataBuffer, BOOL * bIsLaunch);
    bool SendRecvCmd(BYTE * txbuf,int txbuf_sz, BYTE * rxbuf, int rxbuf_sz,QSerialPort & serial, int ms_to=WAIT_TIMEOUT);
    bool send_miniport(QSerialPort & serial);
    bool execute_change_baudrate(int newBaudRate, QSerialPort & serial);
    void Log(const char *);
    void SetupSerialPort(QSerialPort &serial);
    bool WriteNVRAMToDevice(bool bBLEDevice, QSerialPort & serial);

    MainWindow * parent;
    QString file;
    bool bStop;
    bool success;
    int nPktCnt, nByteCnt;
};


#endif // DOWNLOAD_H

