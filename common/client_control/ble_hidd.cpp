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

void MainWindow::InitBLEHIDD()
{
    m_pairing_mode_active = FALSE;
    ui->cbBLEHIDInterupt->clear();
    ui->cbBLEHIDReport->clear();

    ui->cbBLEHIDInterupt->addItem("Control Channel", HCI_CONTROL_HID_REPORT_CHANNEL_CONTROL);
    ui->cbBLEHIDInterupt->addItem("Interrupt Channel", HCI_CONTROL_HID_REPORT_CHANNEL_INTERRUPT);

    ui->cbBLEHIDReport->addItem("Other", HCI_CONTROL_HID_REPORT_TYPE_OTHER);
    ui->cbBLEHIDReport->addItem("Input", HCI_CONTROL_HID_REPORT_TYPE_INPUT);
    ui->cbBLEHIDReport->addItem("Output", HCI_CONTROL_HID_REPORT_TYPE_OUTPUT);
    ui->cbBLEHIDReport->addItem("Feature", HCI_CONTROL_HID_REPORT_TYPE_FEATURE);

    ui->cbBLEHIDInterupt->setCurrentIndex(1);
    ui->cbBLEHIDReport->setCurrentIndex(1);


}

void MainWindow::on_btnBLEHIDSendReport_clicked()
{
   char szLog[80] = { 0 };
   BYTE  cmd[60];

   QVariant v1 = ui->cbBLEHIDInterupt->currentData();
   cmd[0] = (BYTE)v1.toUInt();
   //WriteProfileInt(szAppName, "ReportChannel", (DWORD)cmd[1]);
   QVariant v2 = ui->cbBLEHIDReport->currentData();
   cmd[1] = (BYTE)v2.toUInt();
   //WriteProfileInt(szAppName, "ReportType", (DWORD)cmd[2]);

   char buf[100] = { 0 };
   //GetDlgItemTextA(m_hWnd, IDC_SEND_TEXT, buf, 100);
   QString str = ui->lineEditBLEHIDSendText->text();
   //WriteProfileString(szAppName, "LastReport", buf);

   int num_digits = GetHexValue(&cmd[2], 50, str);

   for (int i = 0; i < num_digits; i++)
       sprintf(&szLog[strlen(szLog)], "%02x ", cmd[i + 2]);

   Log("Sending HID Report: channel %d, report %d, %s",  cmd[0], cmd[1], szLog);

   SendWicedCommand(HCI_CONTROL_HID_COMMAND_SEND_REPORT, cmd, 2 + num_digits);
}

void MainWindow::on_btnBLEHIDPairingMode_clicked()
{
    BYTE    cmd[1];
    if (!m_pairing_mode_active)
    {
        ui->btnBLEHIDPairingMode->setText("Exit Pairing Mode");
        m_pairing_mode_active = TRUE;
        cmd[0] = 1;
    }
    else
    {
        ui->btnBLEHIDPairingMode->setText("Enter Pairing Mode");
        m_pairing_mode_active = FALSE;
        cmd[0] = 0;
    }
    SendWicedCommand(HCI_CONTROL_HID_COMMAND_ACCEPT_PAIRING, cmd, 1);
}

void MainWindow::on_btnBLEHIDConnect_clicked()
{
    Log("Sending HID Connect Command");

    SendWicedCommand(HCI_CONTROL_HID_COMMAND_CONNECT, NULL, 0);
}

void MainWindow::on_btnBLEHIDSendKey_clicked()
{
    char buf[100] = { 0 };
    QString str = ui->cbBLEHIDKey->currentText();
    strcpy(buf, str.toStdString().c_str());
    //GetDlgItemTextA(m_hWnd, IDC_KEY, buf, 100);
    BYTE  cmd[60] = { 0 };
    cmd[0] = HCI_CONTROL_HID_REPORT_CHANNEL_INTERRUPT;
    cmd[1] = HCI_CONTROL_HID_REPORT_TYPE_INPUT;
    cmd[2] = 1; // report id
    cmd[3] = ui->cbBLEHIDCapLock->isChecked() ? 0x20 : 0x00 |
        ui->cbBLEHIDCtrl->isChecked() ? 0x80 : 0x00 |
        ui->cbBLEHIDAlt->isChecked() ? 0x40 : 0x00;

    for (int i = 0; (i < 6) && (buf[i] != 0); i++)
    {
        if ((buf[i] >= '1') && (buf[i] <= '9'))
            cmd[5 + i] = 0x1e + buf[i] - '1';
        if (buf[i] == '0')
            cmd[5 + i] = 0x27;
        if ((buf[i] >= 'a') && (buf[i] <= 'z'))
            cmd[5 + i] = 0x04 + buf[i] - 'a';
    }
    char trace[256];
    sprintf(trace, "Send HID Report Type:%d ID:%d %02x %02x %02x %02x %02x %02x %02x %02x", cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7], cmd[8], cmd[9], cmd[10]);
    Log(trace);
    SendWicedCommand(HCI_CONTROL_HID_COMMAND_SEND_REPORT, cmd, 11);

    if (ui->cbBLEHIDBtnUp->isChecked())
    {
        for (int i = 0; i < 7; i++)
            cmd[4 + i] = 0;
        SendWicedCommand(HCI_CONTROL_HID_COMMAND_SEND_REPORT, cmd, 11);
    }
}

void MainWindow::on_cbBLEHIDCapLock_clicked()
{
    BYTE  cmd[60] = { 0 };
    cmd[0] = HCI_CONTROL_HID_REPORT_CHANNEL_INTERRUPT;
    cmd[1] = HCI_CONTROL_HID_REPORT_TYPE_INPUT;
    cmd[2] = 1; // report id
    cmd[3] = ui->cbBLEHIDCapLock->isChecked() ? 0x20 : 0x00 |
        ui->cbBLEHIDCtrl->isChecked() ? 0x80 : 0x00 |
        ui->cbBLEHIDAlt->isChecked() ? 0x40 : 0x00;
    for (int i = 0; i < 7; i++)
        cmd[4 + i] = 0;
    SendWicedCommand(HCI_CONTROL_HID_COMMAND_SEND_REPORT, cmd, 11);
}

void MainWindow::on_cbBLEHIDCtrl_clicked()
{
    on_cbBLEHIDCapLock_clicked();
}

void MainWindow::on_cbBLEHIDAlt_clicked()
{
    on_cbBLEHIDCapLock_clicked();
}




void MainWindow::onHandleWicedEventBLEHIDD(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    char   trace[1024];

    switch (opcode)
    {
        case HCI_CONTROL_EVENT_DEVICE_STARTED:
            m_pairing_mode_active = FALSE;
            break;

        case HCI_CONTROL_HID_EVENT_OPENED:
            Log("HID connection opened");
            ui->btnBLEHIDPairingMode->setText("Enter Pairing Mode");
            m_pairing_mode_active = FALSE;
            break;
        case HCI_CONTROL_LE_EVENT_ADVERTISEMENT_STATE:
            sprintf(trace, "Advertisement state:%d", p_data[0]);
            Log(trace);
            if (p_data[0] == 0)
            {
                ui->btnBLEHIDPairingMode->setText("Enter Pairing Mode");
                m_pairing_mode_active = FALSE;
            }
            break;
        case HCI_CONTROL_HID_EVENT_VIRTUAL_CABLE_UNPLUGGED:
            Log("HID Virtual Cable Unplugged");
            break;
        case HCI_CONTROL_HID_EVENT_DATA:
            sprintf(trace, "Recv HID Report type:%d ", p_data[0]);
            for (uint i = 0; i < len - 1; i++)
                sprintf(&trace[strlen(trace)], "%02x ", p_data[i + 1]);
            Log(trace);
            break;
        case HCI_CONTROL_HID_EVENT_CLOSED:
            sprintf(trace, "HID Connection Down reason::%d ", p_data[0]);
            Log(trace);
            break;
    }
}
