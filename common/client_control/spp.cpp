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
#include <QMutex>

#define WICED_BT_RFCOMM_SUCCESS 0

extern MainWindow *g_pMainWindow;

DWORD MainWindow::SendFileThreadSPP()
{
    FILE *fp = NULL;
    char buf[1030] = { 0 };
    QString strfile = ui->lineEditSPPSendFile->text();

    fp = fopen(strfile.toStdString().c_str(), "rb");
    if (!fp)
    {
        Log("Failed to open file %s", strfile.toStdString().c_str());

        return 0;
    }

    CBtDevice * pDev = GetConnectedSPPDevice();
    if (pDev == NULL)
    {

        return 0;
    }

    if(m_iContext != CONNECTION_TYPE_SPP)
        return 0;

    int nHandle = pDev->m_spp_handle;
    buf[0] = nHandle & 0xff;
    buf[1] = (nHandle >> 8) & 0xff;

    fseek(fp, 0, SEEK_END);
    m_spp_total_to_send = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    m_spp_bytes_sent = 0;

    int read_bytes;
    QMutex mutex;

    while ((read_bytes = fread(&buf[2], 1, HCI_CONTROL_SPP_MAX_TX_BUFFER, fp)) != 0)
    {
        mutex.lock();        

        SendWicedCommand(HCI_CONTROL_SPP_COMMAND_DATA, (LPBYTE)buf, 2 + read_bytes);
        m_spp_bytes_sent += read_bytes;

        if(spp_tx_wait.wait(&mutex, 5000) == false)
        {
            Log("Wait failed");
            //break;
        }


        if ((m_spp_tx_complete_result != WICED_BT_RFCOMM_SUCCESS) ||
            (!ui->cbSPPSendFile->isChecked()))
        {
            m_spp_total_to_send = 0;
            mutex.unlock();
            break;
        }
        mutex.unlock();
    }
    fclose(fp);

    return 0;
}

void MainWindow::on_cbSPPThreadComplete()
{
    Log("on_cbSPPThreadComplete");
    ui->btnSPPSend->setDisabled(false);
    if(m_thread_spp) delete m_thread_spp;
    m_thread_spp = NULL;
    m_worker_spp = NULL;
}

void MainWindow::InitSPP()
{    
    m_spp_bytes_sent = 0;
    m_spp_total_to_send = 0;
    m_spp_tx_complete_result = 0;
    m_spp_receive_file = NULL;
    g_pMainWindow = this;
    m_thread_spp = NULL;
    m_worker_spp = NULL;
}



void MainWindow::on_btnSPConnect_clicked()
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
    if (NULL == pDev)
        return;

    if(pDev->m_spp_handle != NULL_HANDLE)
    {
        Log("SPP already connected for selected device");
        return;
    }

    // SPP connection can happen from iAP2 tab or
    // SPP tab in the UI. Set the context, which UI
    // tab started the connection, update that UI tab
    m_iContext = CONNECTION_TYPE_SPP;

    for (int i = 0; i < 6; i++)
        cmd[commandBytes++] = pDev->m_address[5 - i];

    Log("Sending SPP Connect Command, BDA: %02x:%02x:%02x:%02x:%02x:%02x",
        cmd[5], cmd[4], cmd[3], cmd[2], cmd[1], cmd[0]);

    SendWicedCommand(HCI_CONTROL_SPP_COMMAND_CONNECT, cmd, 6);
}

void MainWindow::on_btnSPPDisconnect_clicked()
{
    BYTE   cmd[60];
    int    commandBytes = 0;
    CBtDevice * pDev = GetConnectedSPPDevice();
    if (pDev == NULL)
        return;

    if(m_iContext != CONNECTION_TYPE_SPP)
        return;

    USHORT nHandle = pDev->m_spp_handle;

    cmd[commandBytes++] = nHandle & 0xff;
    cmd[commandBytes++] = (nHandle >> 8) & 0xff;

    pDev->m_spp_handle = NULL_HANDLE;
    pDev->m_conn_type &= ~CONNECTION_TYPE_SPP;

    Log("Sending SPP Disconnect Command, Handle: 0x%04x", nHandle);
    SendWicedCommand(HCI_CONTROL_SPP_COMMAND_DISCONNECT, cmd, commandBytes);
}

void MainWindow::on_btnSPPSend_clicked()
{
    char buf[1030] = { 0 };
    CBtDevice * pDev = GetConnectedSPPDevice();
    if (pDev == NULL)
        return;

    if(m_iContext != CONNECTION_TYPE_SPP)
        return;

    USHORT nHandle = pDev->m_spp_handle;

    buf[0] = nHandle & 0xff;
    buf[1] = (nHandle >> 8) & 0xff;


    if (!ui->cbSPPSendFile->isChecked())
    {
        QString str = ui->lineEditSPPSend->text();
        strncpy(&buf[2], str.toStdString().c_str(), sizeof(buf) - 2);
        m_spp_total_to_send = 0;
        m_spp_bytes_sent = 0;
        SendWicedCommand(HCI_CONTROL_SPP_COMMAND_DATA, (LPBYTE)buf, 2 + strlen(&buf[2]));
    }
    else
    {
        ui->btnSPPSend->setDisabled(true);

        m_thread_spp = new QThread;
        m_worker_spp = new Worker();
        m_worker_spp->moveToThread(m_thread_spp);

        connect(m_thread_spp, SIGNAL(started()), m_worker_spp, SLOT(process_spp()));
        connect(m_worker_spp, SIGNAL(finished()), m_thread_spp, SLOT(quit()));
        connect(m_worker_spp, SIGNAL(finished()), m_worker_spp, SLOT(deleteLater()));
        connect(m_thread_spp, SIGNAL(finished()), m_thread_spp, SLOT(deleteLater()));
        connect(m_worker_spp, SIGNAL(finished()), this, SLOT(on_cbSPPThreadComplete()));

        m_thread_spp->start();
    }
}

void MainWindow::on_btnSPPBrowseSend_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"),"","");

    ui->lineEditSPPSendFile->setText(file);
}

void MainWindow::on_btnSPPBrowseReceive_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save File"),"","");

    ui->lineEditSPPReceiveFile->setText(file);

    on_cbSPPReceiveFile_clicked(true);
}

void MainWindow::on_cbSPPSendFile_clicked(bool checked)
{

}

void MainWindow::on_cbSPPReceiveFile_clicked(bool checked)
{
    if (m_spp_receive_file){
        fclose(m_spp_receive_file);
        m_spp_receive_file = NULL;
    }

    if (ui->cbSPPReceiveFile->isChecked())
    {        
        QString strfile = ui->lineEditSPPReceiveFile->text();


        m_spp_receive_file = fopen(strfile.toStdString().c_str(), "wb");

        if (!m_spp_receive_file){
            Log("Error: could not open spp receive file %s \n", strfile.toStdString().c_str());
        }
        else{
            Log("Opened spp receive file %s \n", strfile.toStdString().c_str());
        }
    }
    else
    {
        if (m_spp_receive_file){
            fclose(m_spp_receive_file);
            m_spp_receive_file = NULL;
        }
    }
}


void MainWindow::onHandleWicedEventSPP(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_SPP:
        HandleSPPEvents(opcode, p_data, len);
        break;
    }
}

void MainWindow::HandleSPPEvents(DWORD opcode, LPBYTE p_data, DWORD len)
{
    char   trace[1024];
    CBtDevice *device;
    BYTE bda[6];
    UINT16  handle;
    static int ea_total = 0;
    static BYTE last_byte_received = 0xff;

    if(m_iContext != CONNECTION_TYPE_SPP)
        return;

    switch (opcode)
    {
    case HCI_CONTROL_SPP_EVENT_CONNECTED:
        for (int i = 0; i < 6; i++)
            bda[5 - i] = p_data[i];

        handle = p_data[6] + (p_data[7] << 8);

        sprintf(trace, "SPP connected %02x:%02x:%02x:%02x:%02x:%02x handle %04x",
            bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], handle);
        Log(trace);

        // find device in the list with received address and save the connection handle
        if ((device = FindInList(bda,ui->cbDeviceList)) == NULL)
            device = AddDeviceToList(bda, ui->cbDeviceList, NULL);

        device->m_spp_handle = handle;
        device->m_conn_type |= CONNECTION_TYPE_SPP;

        SelectDevice(ui->cbDeviceList, bda);
        break;
    case HCI_CONTROL_SPP_EVENT_SERVICE_NOT_FOUND:
        Log("SPP Service not found");
        break;
    case HCI_CONTROL_SPP_EVENT_CONNECTION_FAILED:
        Log("SPP Connection Failed");
        break;
    case HCI_CONTROL_SPP_EVENT_DISCONNECTED:
    {
        Log("SPP disconnected");
        handle = p_data[0] | (p_data[1] << 8);
        CBtDevice * pDev = FindInList(CONNECTION_TYPE_SPP, handle, ui->cbDeviceList);
        if (pDev)
        {
            pDev->m_spp_handle = NULL_HANDLE;
            pDev->m_conn_type &= ~CONNECTION_TYPE_SPP;
            m_iContext = 0;
        }
    }
        break;
    case HCI_CONTROL_SPP_EVENT_TX_COMPLETE:
        handle = p_data[0] | (p_data[1] << 8);
        m_spp_tx_complete_result = p_data[2];

        if (!ui->cbSPPSendFile->isChecked())
        {
            sprintf(trace, "SPP tx complete handle:%d result:%d", handle, m_spp_tx_complete_result);
        }
        else
        {
            sprintf(trace, "SPP tx complete handle:%d result:%d %d of %d",
                handle, m_spp_tx_complete_result, m_spp_bytes_sent, m_spp_total_to_send);
        }        
        spp_tx_wait.wakeAll();
        Log(trace);
        break;
    case HCI_CONTROL_SPP_EVENT_RX_DATA:

        handle = p_data[0] + (p_data[1] << 8);
        ea_total += (len - 2);
        if (len > 32)
        {
            if (p_data[2] != (BYTE)(last_byte_received + 1))
                sprintf(trace, "----SPP rx complete session id:%d len:%d total:%d %02x - %02x",
                    handle, len - 2, ea_total, p_data[2], p_data[len - 1]);
            else
                sprintf(trace, "SPP rx complete session id:%d len:%d total:%d %02x - %02x",
                    handle, len - 2, ea_total, p_data[2], p_data[len - 1]);
            last_byte_received = p_data[len - 1];
        }
        else
        {
            sprintf(trace, "SPP rx complete session id:%d len:%d total:%d ",
                handle, len - 2, ea_total);
            for (DWORD i = 0; i < len - 2; i++)
                snprintf(&trace[strlen(trace)], (sizeof(trace)) / sizeof(char) - strlen(trace), "%02x ", p_data[2 + i]);
        }
        Log(trace);
        if (m_spp_receive_file)
            fwrite(&p_data[2], 1, len - 2, m_spp_receive_file);
        else
        {
            trace[0] = 0;
            for (DWORD i = 0; (i < len - 2) && (i < 100); i++)
                snprintf(&trace[strlen(trace)], sizeof(trace) / sizeof(char) - strlen(trace), "%02x ", p_data[2 + i]);

            ui->lineEditSPPreceive->setText(trace);
        }
        break;

    }
}

CBtDevice* MainWindow::GetConnectedSPPDevice()
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    if(pDev->m_spp_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as SPP");
        return NULL;
    }

    return pDev;
}

/* spp thread */
void Worker::process_spp()
{
    g_pMainWindow->SendFileThreadSPP();
    emit finished();
}
