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

Q_DECLARE_METATYPE( CBtDevice* )


#define WICED_BT_RFCOMM_SUCCESS 0
#define WICED_BT_RFCOMM_NO_MEM  5

extern MainWindow *g_pMainWindow;

// Bluetooth Serial over GATT


void MainWindow::onHandleWicedEventBSG(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {

    case HCI_CONTROL_GROUP_BSG:
        HandleBSGEvents(opcode, p_data, len);
        break;
    }
}


DWORD MainWindow::SendFileThreadBSG()
{
    FILE *fp = NULL;
    char buf[1030] = { 0 };
    QString strfile = ui->lineEditBSGSendFile->text();

    fp = fopen(strfile.toStdString().c_str(), "rb");
    if (!fp)
    {
        Log("Failed to open file %s", strfile.toStdString().c_str());

        return 0;
    }

    CBtDevice * pDev = GetConnectedBSGDevice();
    if (pDev == NULL)
    {

        return 0;
    }

    int nHandle = pDev->m_bsg_handle;
    buf[0] = nHandle & 0xff;
    buf[1] = (nHandle >> 8) & 0xff;

    fseek(fp, 0, SEEK_END);
    m_bsg_total_to_send = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    m_bsg_bytes_sent = 0;

    int read_bytes;
    QMutex mutex;

     m_uart_tx_size = m_settings.value("UartTxSize",1024).toInt();

     Log("UartTxSize set to %d", m_uart_tx_size);


    while ((read_bytes = fread(&buf[2], 1, m_uart_tx_size, fp)) != 0)
    {
        mutex.lock();

        SendWicedCommand(HCI_CONTROL_BSG_COMMAND_DATA, (LPBYTE)buf, 2 + read_bytes);
        m_bsg_bytes_sent += read_bytes;

        if(bsg_tx_wait.wait(&mutex, 5000) == false)
        {
            Log("Wait failed");
            //break;
        }


        if ((m_bsg_tx_complete_result != WICED_BT_RFCOMM_SUCCESS) ||
            (!ui->cbBSGSendFile->isChecked()))
        {
            m_bsg_total_to_send = 0;
            mutex.unlock();
            break;
        }
        mutex.unlock();
    }
    fclose(fp);

    return 0;
}

void MainWindow::on_cbBSGThreadComplete()
{
    Log("on_cbBSGThreadComplete");
    ui->btnBSGSend->setDisabled(false);
}

void MainWindow::InitBSG()
{
    m_bsg_bytes_sent = 0;
    m_bsg_total_to_send = 0;
    m_bsg_tx_complete_result = 0;
    m_bsg_receive_file = NULL;
    g_pMainWindow = this;
}

void MainWindow::HandleBSGEvents(DWORD opcode, LPBYTE p_data, DWORD len)
{
    CHAR    trace[1024];
    uint16_t handle;

    switch (opcode)
    {
    case HCI_CONTROL_BSG_EVENT_CONNECTED:
        Log("BSG Connected address %02x:%02x:%02x:%02x:%02x:%02x handle:%d",
            p_data[5], p_data[4], p_data[3], p_data[2], p_data[1], p_data[0], p_data[6] + (p_data[7] << 8));
        break;

    case HCI_CONTROL_BSG_EVENT_DISCONNECTED:
        Log("BSG Disconnected handle:%d", p_data[0] + (p_data[1] << 8));
        break;

    case HCI_CONTROL_BSG_EVENT_TX_COMPLETE:
        handle = p_data[0] | (p_data[1] << 8);
        m_bsg_tx_complete_result = p_data[2];

        if (!ui->cbBSGSendFile->isChecked())
        {
            sprintf(trace, "BSG tx complete handle:%d result:%d", handle, m_bsg_tx_complete_result);
        }
        else
        {
            sprintf(trace, "BSG tx complete handle:%d result:%d %d of %d",
                handle, m_bsg_tx_complete_result, m_bsg_bytes_sent, m_bsg_total_to_send);
        }
        bsg_tx_wait.wakeAll();
        Log(trace);
        break;

    case HCI_CONTROL_BSG_EVENT_RX_DATA:
    {
        handle = p_data[0] | (p_data[1] << 8);
        sprintf(trace, "BSG data rx handle:%d len:%d", handle, len - 2);
        Log(trace);

        if (m_bsg_receive_file)
        {
            fwrite((char*)&p_data[2], len - 2, 1, m_bsg_receive_file);
        }
        else
        {
            char ascii_data[3 * 1000 + 1];
            char *p_dst = ascii_data;
            char *p_src = (char *)&p_data[2];
            char hex_lut[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

            for (int i = 0; i < (int)len - 2; i++, p_src++)
            {
                // If this is a printable ascii character, write it
                if (isascii(p_src))
                {
                    *p_dst++ = *p_src;
                }
                else
                {
                    // If this is not a printable ascii character, print it as Hexa
                    *p_dst++ = '\'';
                    *p_dst++ = hex_lut[(*p_src >> 4)];
                    *p_dst++ = hex_lut[(*p_src & 0x0F)];
                }
            }
            *p_dst++ = '\0';
            ui->lineEditBSGreceive->setText(ascii_data);
        }
    }
    break;

    default:
        Log("Unknown BSG event:%d", opcode);
        break;
    }
}

void MainWindow::on_btnBSGSend_clicked()
{
    char buf[1030] = { 0 };
    CBtDevice * pDev = GetConnectedBSGDevice();
    if (pDev == NULL)
        return;
    USHORT nHandle = pDev->m_bsg_handle;

    buf[0] = nHandle & 0xff;
    buf[1] = (nHandle >> 8) & 0xff;


    if (!ui->cbBSGSendFile->isChecked())
    {
        QString str = ui->lineEditBSGSend->text();
        strncpy(&buf[2], str.toStdString().c_str(), sizeof(buf) - 2);
        m_bsg_total_to_send = 0;
        m_bsg_bytes_sent = 0;
        SendWicedCommand(HCI_CONTROL_BSG_COMMAND_DATA, (LPBYTE)buf, 2 + strlen(&buf[2]));
    }
    else
    {
        ui->btnBSGSend->setDisabled(true);

        QThread* thread = new QThread;
        Worker* worker = new Worker();
        worker->moveToThread(thread);

        connect(thread, SIGNAL(started()), worker, SLOT(process_bsg()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished()), this, SLOT(on_cbBSGThreadComplete()));

        thread->start();
    }
}

void MainWindow::on_btnBSGBrowseSend_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"),"","");

    ui->lineEditBSGSendFile->setText(file);
}

void MainWindow::on_btnBSGBrowseReceive_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save File"),"","");

    ui->lineEditBSGReceiveFile->setText(file);

    on_cbBSGReceiveFile_clicked(true);
}

void MainWindow::on_cbBSGSendFile_clicked(bool checked)
{

}

void MainWindow::on_cbBSGReceiveFile_clicked(bool checked)
{
    if (m_bsg_receive_file){
        fclose(m_bsg_receive_file);
        m_bsg_receive_file = NULL;
    }

    if (ui->cbBSGReceiveFile->isChecked())
    {
        QString strfile = ui->lineEditBSGReceiveFile->text();


        m_bsg_receive_file = fopen(strfile.toStdString().c_str(), "wb");

        if (!m_bsg_receive_file){
            Log("Error: could not open bsg receive file %s \n", strfile.toStdString().c_str());
        }
        else{
            Log("Opened bsg receive file %s \n", strfile.toStdString().c_str());
        }
    }
    else
    {
        if (m_bsg_receive_file){
            fclose(m_bsg_receive_file);
            m_bsg_receive_file = NULL;
        }
    }
}


CBtDevice* MainWindow::GetConnectedBSGDevice()
{
    CBtDevice * pDev = GetSelectedLEDevice();
    if (pDev == NULL)
    {
        Log("No device selected");
        return NULL;
    }

    /* currently there is no BSG handle
    if(pDev->m_bsg_handle == NULL_HANDLE)
    {
        Log("Selected device is not connected as BSG");
        return NULL;
    }
    */

    return pDev;
}

/* bsg thread */
void Worker::process_bsg()
{
    g_pMainWindow->SendFileThreadBSG();
    emit finished();
}

