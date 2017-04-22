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
#include "btle_homekit2_lightbulb.h"
#include <QMessageBox>
#include <QTimer>

#define HDLC_LOCK_MANAGEMENT_SERVICE_CURRENT_DOOR_STATE_VALUE               0x7f
#define HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_CURRENT_STATE_VALUE      0x94
#define HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_TARGET_STATE_VALUE       0x98

const char* door_state[] = {
    "Open",
    "Closed",
    "Opening",
    "Closing",
    "Stopped"
};

const char* lock_state[] = {
    "Unsecured",
    "Secured",
    "Jammed",
    "Unknown"
};

const char* lock_target_state[] = {
    "Unsecured",
    "Secured"
};

#define HAP_CHARACTERISTIC_FORMAT_BOOL                      0x01
#define HAP_CHARACTERISTIC_FORMAT_UINT8                     0x04
#define HAP_CHARACTERISTIC_FORMAT_UINT16                    0x06
#define HAP_CHARACTERISTIC_FORMAT_UINT32                    0x08
#define HAP_CHARACTERISTIC_FORMAT_UINT64                    0x0a
#define HAP_CHARACTERISTIC_FORMAT_INT32                     0x10
#define HAP_CHARACTERISTIC_FORMAT_FLOAT                     0x14
#define HAP_CHARACTERISTIC_FORMAT_STRING                    0x19
#define HAP_CHARACTERISTIC_FORMAT_TLV8                      0x1b

const char* TypeString(BYTE type)
{
    QString str;
    switch (type)
    {
    case HAP_CHARACTERISTIC_FORMAT_BOOL:
        return "boolean";
    case HAP_CHARACTERISTIC_FORMAT_UINT8:
        return "uint8";
    case HAP_CHARACTERISTIC_FORMAT_UINT16:
        return "uint16";
    case HAP_CHARACTERISTIC_FORMAT_UINT32:
        return "uint32";
    case HAP_CHARACTERISTIC_FORMAT_UINT64:
        return "uint64";
    case HAP_CHARACTERISTIC_FORMAT_INT32:
        return "int32";
    case HAP_CHARACTERISTIC_FORMAT_FLOAT:
        return "float";
    case HAP_CHARACTERISTIC_FORMAT_STRING:
        return "string";
    case HAP_CHARACTERISTIC_FORMAT_TLV8:
        return "tlv8";
    default:
        return "unknown";
    }
}

void MainWindow::InitHK()
{
    m_bLightOn = false;
    m_nLightBrightness = 0;
    m_nDoorState = 1;
    m_nLockState = 1;
    m_nLockTargetState = 0;
    m_nIdentifyTimerCounter = 0;

    SetLightOnOff(m_bLightOn);

    char strBrightness[20];
    sprintf(strBrightness, "%d", m_nLightBrightness);

    ui->lineEditHKBrightness->setText(strBrightness);

    for (int i = 0; i < 5; i++)
    {
        ui->cbDoorState->addItem(door_state[i]);
    }
    ui->cbDoorState->setCurrentIndex(m_nDoorState);

    for (int i = 0; i < 4; i++)
    {
        ui->cbLockState->addItem(lock_state[i]);
    }
    ui->cbLockState->setCurrentIndex(m_nLockState);

    for (int i = 0; i < 2; i++)
    {
        ui->cbLockTargetState->addItem(lock_target_state[i]);
    }
    ui->cbLockTargetState->setCurrentIndex(m_nLockTargetState);

    p_timer = new QTimer(this);

    connect(p_timer, SIGNAL(timeout()), this, SLOT(on_timer()));
}

void MainWindow::SendHciCommand(UINT16 command, USHORT handle, LPBYTE p, DWORD dwLen)
{
    BYTE buffer[32];
    char trace[1024];

    buffer[0] = handle & 0xff;
    buffer[1] = (handle >> 8) & 0xff;
    if (p)
        memcpy(&buffer[2], p, dwLen);

    SendWicedCommand(command, buffer, dwLen + 2);

    switch (command)
    {
    case HCI_CONTROL_HK_COMMAND_READ:
        sprintf(trace, "Read characteristic [%02x]", handle);
        Log(trace);
        break;
    case HCI_CONTROL_HK_COMMAND_WRITE:
        sprintf(trace, "Write characteristic [%02x] : ", handle);
        for (int i = 0; i < (int)dwLen; i++)
            sprintf(&trace[strlen(trace)], "%02x", p[i]);
        Log(trace);
        break;
    }
}

void MainWindow::on_btnHKRead_clicked()
{
    QString str = ui->lineEditHKHandle->text();
    USHORT handle = GetHandle(str);

    SendHciCommand(HCI_CONTROL_HK_COMMAND_READ, handle, NULL, 0);
}

void MainWindow::on_btnHKWrite_clicked()
{
    USHORT handle;
    BYTE  buffer[32];
    DWORD num_bytes;

    QString strHandle = ui->lineEditHKHandle->text();
    handle = GetHandle(strHandle);

    QString strHex = ui->lineEditHKHexVal->text();
    num_bytes = GetHexValue(buffer, sizeof(buffer), strHex);

    SendHciCommand(HCI_CONTROL_HK_COMMAND_WRITE, handle, buffer, num_bytes);
}

void MainWindow::on_btnHKList_clicked()
{
    SendWicedCommand(HCI_CONTROL_HK_COMMAND_LIST, NULL, 0);
    Log("List characteristics");
}

void MainWindow::on_btnHKSwitch_clicked()
{
    m_bLightOn = !m_bLightOn;
    SetLightOnOff(m_bLightOn);
    SendHciCommand(HCI_CONTROL_HK_COMMAND_WRITE, HDLC_LIGHTBULB_ON_VALUE, (LPBYTE)&m_bLightOn, 1);
}

void MainWindow::ShowMessage()
{
    QMessageBox msgBox;
    msgBox.setText("Please enter a value between 0 and 100");
    msgBox.exec();
}

void MainWindow::on_btnHKSet_clicked()
{
    QString str = ui->lineEditHKBrightness->text();
    if(str.length() == 0 || str.length() > 3)
    {
        ShowMessage();
        return;
    }

    for(int i = 0; i < str.length(); i++)
    {
        QChar c = str.at(i);
        if(!c.isDigit())
        {
            ShowMessage();
            return;
        }
    }

    uint brightness = ui->lineEditHKBrightness->text().toUInt();

    if (brightness > 100)
    {
        ShowMessage();
        return;
    }

    m_nLightBrightness = brightness;
    SendHciCommand(HCI_CONTROL_HK_COMMAND_WRITE, HDLC_LIGHTBULB_BRIGHTNESS_VALUE,(LPBYTE)&m_nLightBrightness, 4);

}

void MainWindow::on_cbDoorState_currentIndexChanged(int index)
{    
    m_nDoorState = index;
    SendHciCommand(HCI_CONTROL_HK_COMMAND_WRITE, HDLC_LOCK_MANAGEMENT_SERVICE_CURRENT_DOOR_STATE_VALUE, (LPBYTE)&m_nDoorState, 1);

}

void MainWindow::on_cbLockState_currentIndexChanged(int index)
{    
    m_nLockState = index;
    SendHciCommand(HCI_CONTROL_HK_COMMAND_WRITE, HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_CURRENT_STATE_VALUE, (LPBYTE)&m_nLockState, 1);
}

void MainWindow::on_cbLockTargetState_currentIndexChanged(int index)
{    
    m_nLockTargetState = index;
    SendHciCommand(HCI_CONTROL_HK_COMMAND_WRITE, HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_TARGET_STATE_VALUE, (LPBYTE)&m_nLockTargetState, 1);
}

void MainWindow::onHandleWicedEventHK(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
        {
            case HCI_CONTROL_GROUP_HK:
            HandleHkEvent(opcode, p_data, len);
            break;
        }
}

void MainWindow::on_btnHKFactoryReset_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Warning");
    msgBox.setText("Are you sure you want to factory reset?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes)
    {
        SendHciCommand(HCI_CONTROL_HK_COMMAND_FACTORY_RESET, 0, NULL, 0);
    }
}


void MainWindow::SetLightOnOff(BOOL on)
{

    if (on)
        ui->labelHKLight->setText("ON");
    else
        ui->labelHKLight->setText("ON");
}


void MainWindow::UpdateUI(USHORT handle, LPBYTE p, DWORD dwLen)
{
    char str[10];
    switch (handle)
    {
    case HDLC_LIGHTBULB_ON_VALUE:
        m_bLightOn = p[0];
        Log("UpdateUI HDLC_LIGHTBULB_ON_VALUE : %d", m_bLightOn);
        SetLightOnOff(m_bLightOn);
        break;
    case HDLC_LIGHTBULB_BRIGHTNESS_VALUE:
        m_nLightBrightness = p[0];
        sprintf(str, "%d", m_nLightBrightness);
        Log("UpdateUI HDLC_LIGHTBULB_BRIGHTNESS_VALUE : %d", m_nLightBrightness);
        ui->lineEditHKBrightness->setText(str);
        break;
    case HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_TARGET_STATE_VALUE:
        m_nLockTargetState = p[0];
        Log("UpdateUI HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_TARGET_STATE_VALUE : %d", m_nLockTargetState);
        ui->cbLockTargetState->setCurrentIndex(m_nLockTargetState);
        break;
    }
}

void MainWindow::on_timer()
{
    if (m_nIdentifyTimerCounter > 0)
    {
        if (--m_nIdentifyTimerCounter > 0)
        {
            m_bLightOn = !m_bLightOn;
            SetLightOnOff(m_bLightOn);
        }
        else
        {
            p_timer->stop();
        }
    }
}


void MainWindow::HandleHkEvent(DWORD opcode, LPBYTE p_data, DWORD len)
{
    char trace[1024];
    uint handle;
    char strhandle[20];


    Log("CLightbulbControlDlg::HandleHkEvent  Rcvd Op Code: 0x%04x, len: %d", opcode, len);

    switch (opcode)
    {
    case HCI_CONTROL_HK_EVENT_READ_RESPONSE:
        sprintf(trace, "Read Response : ");
        for (int i = 0; i < (int)len - 2; i++)
            sprintf(&trace[strlen(trace)], "%02x", p_data[2 + i]);
        ui->lineEditHKHexVal->setText(&trace[strlen("Read Response : ")]);
        Log(trace);
        break;
    case HCI_CONTROL_HK_EVENT_UPDATE:
        handle = p_data[0] + (p_data[1] << 8);
        sprintf(strhandle, "%04x", handle);
        ui->lineEditHKHandle->setText(strhandle);

        if (handle == HDLC_ACCESSORY_INFO_IDENTIFY_VALUE && p_data[2])
        {
            Log("Received Identify");
            m_nIdentifyTimerCounter = 5;

            p_timer->start(1000); // start a 1 sec timer to flicker the light
            break;
        }
        else if (handle == HDLC_LOCK_MECHANISM_SERVICE_LOCK_MECHANISM_TARGET_STATE_VALUE)
        {
            sprintf(trace, "Characteristic update [%s] : %02x", strhandle, p_data[2]);
            ui->lineEditHKHexVal->setText(&trace[strlen("Characteristic update [00] : ")]);
            Log(trace);
            if (len > 3)
            {
                Log("Additional authorization data : ");
                for (int i = 0; i < (int)len - 3; i++)
                    sprintf(&trace[strlen(trace)], "%02x", p_data[3 + i]);
               Log(trace);
            }
        }
        else
        {
            sprintf(trace, "Characteristic update [%s] : ", strhandle);
            for (int i = 0; i < (int)len - 2; i++)
                sprintf(&trace[strlen(trace)],"%02x", p_data[2 + i]);
            ui->lineEditHKHexVal->setText(&trace[strlen("Characteristic update [00] : ")]);
            Log(trace);
        }

        UpdateUI(handle, &p_data[2], len - 2);
        break;
    case HCI_CONTROL_HK_EVENT_LIST_ITEM:
        handle = p_data[0] + (p_data[1] << 8);
        sprintf(trace, "0x%04x    %s     %s", handle, TypeString(p_data[2]), &p_data[3]);
        Log(trace);
        break;
    default:
        Log("HandleHkEvent  Rcvd Unsupported Op Code: 0x%04x", opcode);
        break;
    }
}

