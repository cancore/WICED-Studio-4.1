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
#include <QMessageBox>
#include "hci_control_api.h"
#include <QFileDialog>
#include <QDialogButtonBox>
#include "download.h"
#include <QTimer>
#include <time.h>
#include "btle_homekit2_lightbulb.h"

extern void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length);
static void DumpData(char *description, void* p, unsigned int length, unsigned int max_lines);
Q_DECLARE_METATYPE( CBtDevice* )
DownloadThread dl;

CBtDevice::CBtDevice (bool paired) :
    m_nvram(NULL), m_nvram_id(-1), m_paired(paired)
{
    memset(m_address,0,sizeof(m_address));
    memset(m_name,0,sizeof(m_name));

    m_audio_handle = NULL_HANDLE;
    m_hf_handle = NULL_HANDLE;
    m_ag_handle = NULL_HANDLE;
    m_spp_handle = NULL_HANDLE;
    m_hidh_handle = NULL_HANDLE;
    m_iap2_handle = NULL_HANDLE;
    m_avrc_handle = NULL_HANDLE;
    m_avk_handle = NULL_HANDLE;
    m_bsg_handle = NULL_HANDLE;
    m_conn_type = 0;
    m_bIsLEDevice = false;
}

CBtDevice::~CBtDevice ()
{
}

// valid baud rates
static int as32BaudRate[] =
{
    115200,
    3000000,
    4000000
};

void MainWindow::InitDm()
{
    m_scan_active = m_inquiry_active = false;
    m_CommPort = NULL;

    m_port_read_worker = NULL;
    m_port_read_thread = NULL;

    EnableUI(false);

    // read settings for baudrate, serial port and flow-ctrl
    //QSettings settings(m_SettingsFile, QSettings::IniFormat);
    int baud_rate = m_settings.value("Baud",4000000).toInt();
    bool flow_control = m_settings.value("FlowControl",true).toBool();
    QString comm_port = m_settings.value("port").toString();
    ui->btnPairable->setChecked(m_settings.value("pairing_mode",true).toBool());
    ui->btnDiscoverable->setChecked(m_settings.value("discoverable",true).toBool());
    ui->btnConnectable->setChecked(m_settings.value("Connectable",true).toBool());

    // setup icon in device list for paried devices
    ui->cbDeviceList->setIconSize(QSize(20,20));
    ui->cbBLEDeviceList->setIconSize(QSize(20,20));

    // get paired devices
    ReadDevicesFromSettings("devices", ui->cbDeviceList, ui->btnUnbond);
    ReadDevicesFromSettings("devicesLE", ui->cbBLEDeviceList, ui->btnBLEUnbond);

    // get list of all available serial ports
    int port_inx = -1;
    QList<QSerialPortInfo> port_list = QSerialPortInfo::availablePorts();
    for (int i =0; i < port_list.size(); i++)
    {        
        ui->cbCommport->addItem(port_list.at(i).systemLocation(),port_list[i].serialNumber());
    }

    if ( -1 != (port_inx = ui->cbCommport->findText(comm_port)))
        ui->cbCommport->setCurrentIndex(port_inx);

    // populate dropdown list of baud rates
    QString strBaud;
    int baud_inx = 2; // select baud rate 4000000 by default
    for (int i = 0; i < sizeof(as32BaudRate) / sizeof(as32BaudRate[0]); i++)
    {
        strBaud.sprintf( "%d", as32BaudRate[i]);
        ui->cbBaudRate->addItem(strBaud,i);
        if (as32BaudRate[i] == baud_rate)
            baud_inx = i;
    }
    ui->cbBaudRate->setCurrentIndex(baud_inx);
    ui->btnFlowCntrl->setChecked(flow_control );


    // setup signals/slots
    connect(ui->btnStartDisc, SIGNAL(clicked()), this, SLOT(onStartDisc()));
    connect(ui->btnStopDisc, SIGNAL(clicked()), this, SLOT(onStopDisc()));
    connect(ui->cbCommport, SIGNAL(currentTextChanged(QString)), this, SLOT(onCommPortChange(QString)));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(onReset()));
    connect(ui->btnUnbond, SIGNAL(clicked()), this, SLOT(OnBnClickedBREDRUnbond()));
    connect(ui->btnBLEUnbond, SIGNAL(clicked()), this, SLOT(OnBnClickedLeUnbond()));
    connect(ui->cbDeviceList, SIGNAL(currentTextChanged(QString)), this, SLOT(onDevChange(QString)));
    connect(ui->cbBLEDeviceList, SIGNAL(currentTextChanged(QString)), this, SLOT(onLEDevChange(QString)));
    connect(ui->btnDiscoverable, SIGNAL(clicked(bool)), this, SLOT(onDiscoverable(bool)));
    connect(ui->btnConnectable, SIGNAL(clicked(bool)), this, SLOT(onConnectable(bool)));
    connect(ui->btnPairable, SIGNAL(clicked(bool)), this, SLOT(onPairable(bool)));
    connect(ui->btnDownload, SIGNAL(clicked()), this, SLOT(onDownload()));
    connect(ui->btnFindPatchFile, SIGNAL(clicked()), this, SLOT(onFindPatchFile()));
    connect(&dl, SIGNAL(dlProgress(QString*,int,int)), this, SLOT(onDlProgress(QString*,int,int)));
    connect(&dl, SIGNAL(dlDone(QString)), this, SLOT(onDlDone(QString)));    
    connect(&dl_msgbox,SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onMsgBoxButton(QAbstractButton*)));
    connect(ui->btnBLEStartDisc, SIGNAL(clicked()), this, SLOT(OnBnClickedDiscoverDevicesStart()));
    connect(ui->btnBLEStopDisc, SIGNAL(clicked()), this, SLOT(OnBnClickedDiscoverDevicesStop()));

    ui->edPatchFile->setText(m_settings.value("HCDFile","").toString());
    m_bPortOpen = false;

    m_major = 0;
    m_minor = 0;
    m_rev = 0;
    m_build = 0;
    m_chip = 0;
    m_power = 0;
    m_features = 0xFFFF;

    char strBda[100];
    srand (time(NULL));
    sprintf(strBda, "%02X:%02X:%02X:%02X:%02X:%02X", 0x2, 0x0, 0x7, rand() % 255, rand() % 255, rand() % 255);
    ui->edLocalBDA->setText(m_settings.value("LocalBDA",strBda).toString());

    p_dm_timer = new QTimer(this);
    connect(p_dm_timer, SIGNAL(timeout()), this, SLOT(on_dm_timer()));       
}

void MainWindow::ReadDevicesFromSettings(const char *group, QComboBox *cbDevices, QPushButton *btnUnbond)
{
    // get paired devices
    m_settings.beginGroup(group);
    QStringList  device_keys = m_settings.allKeys();

    int nvram_id=-1;
    QString dev_name_id, dev_name="";
    int bda0,bda1,bda2,bda3,bda4,bda5;
    for (int i = 0; i < device_keys.size(); i++)
    {
        dev_name_id = m_settings.value(device_keys[i],"").toString();
        if (dev_name_id.size() >= 3)
        {
            sscanf(dev_name_id.toStdString().c_str(),"%02X:", &nvram_id);
            if (dev_name_id.size() > 4)
                dev_name = dev_name_id.right(dev_name_id.size()-3);
        }

        sscanf(device_keys[i].toStdString().c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
               &bda0, &bda1, &bda2, &bda3, &bda4, &bda5);
        BYTE bda[6];
        bda[0] = bda0;
        bda[1] = bda1;
        bda[2] = bda2;
        bda[3] = bda3;
        bda[4] = bda4;
        bda[5] = bda5;

        CBtDevice * pDev = AddDeviceToList(bda,cbDevices, (char *)dev_name.toStdString().c_str(),true);
        if (pDev && nvram_id != -1)
        {
            pDev->m_nvram_id = nvram_id;
        }
        if(pDev && (strcmp(group, "devicesLE") == 0))
        {
            pDev->m_bIsLEDevice = true;
        }
    }
    m_settings.endGroup();

    // get link key for paired devices
    m_settings.beginGroup("NVRAM");
    for (int i = 0; i < cbDevices->count(); i++)
    {
        QVariant var;
        var = cbDevices->itemData(i);
        CBtDevice * pDev = var.value<CBtDevice *>();
        if (NULL == pDev)
            continue;
        if (pDev->m_nvram_id == -1)
            continue;
        QString nvram_id_key;
        nvram_id_key.sprintf("%02X",pDev->m_nvram_id);
        pDev->m_nvram = m_settings.value(nvram_id_key).toByteArray();
    }
    m_settings.endGroup();

    btnUnbond->setEnabled(false);
    if (cbDevices->count())
    {
        cbDevices->setCurrentIndex(0);
        btnUnbond->setEnabled(true);
    }
}

void MainWindow::onMsgBoxButton(QAbstractButton*btn)
{
    // user pressed cancel button for download
    dl.stop();
}

void MainWindow::resetState()
{

}

// called by download thread to report progress
void MainWindow::onDlProgress(QString * msg, int pktcnt, int bytecnt)
{
    if (msg)
    {
        Log(msg->toStdString().c_str());
        delete msg;
    }

    if (pktcnt && bytecnt)
    {
        QString trace;

        trace.sprintf("Download status, record %d, total bytes: %d",pktcnt,bytecnt);
        Log(trace.toStdString().c_str());
        dl_msgbox.setText(trace);
        dl_msgbox.update();
    }

    // update screen
    update();
    qApp->processEvents();
}

// called by download thread when finished
void MainWindow::onDlDone(const QString &s)
{
    qDebug(s.toStdString().c_str());
    Log(s.toStdString().c_str());
    dl_msgbox.close();  // close download message box
}

void MainWindow::onDownload()
{
    if (ui->edPatchFile->text().length() == 0 || ui->edLocalBDA->text().length() == 0)
    {
        Log("Specify valid configuration file and Address");
        return;
    }

    m_settings.setValue("HCDFile",ui->edPatchFile->text());
    ClearPort();

    // download patch file in separate thread
    QString str = ui->edPatchFile->text();
    dl.download(this,str);

    dl_msgbox.setText("Downloading new firmware.");
    dl_msgbox.setWindowTitle("Firmware Download");
    dl_msgbox.setStandardButtons(QMessageBox::Cancel);
    dl_msgbox.show();
}

void MainWindow::onFindPatchFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Patch Download File"), "", tr("HCD Files (*.hcd)"));
    ui->edPatchFile->setText(fileName);
}

void MainWindow::onDiscoverable(bool)
{
    m_settings.setValue("discoverable",ui->btnDiscoverable->isChecked());
    setVis();
}

void MainWindow::onConnectable(bool)
{
    m_settings.setValue("Connectable",ui->btnConnectable->isChecked());
    setVis();
}

void MainWindow::onPairable(bool)
{
    m_settings.setValue("pairing_mode",ui->btnPairable->isChecked());
    setPairingMode();
}

void MainWindow::onUnbond(QComboBox* cb)
{
    if (m_CommPort == NULL)
    {
        Log("Serial port is not open");
        return;
    }

    if (!m_bPortOpen)
    {
        Log("Serial port is not open");
        return;
    }

    int i = cb->currentIndex();
    if ( i == -1)
        return;

    QVariant var;
    var = cb->itemData(i);

    CBtDevice * pDev = var.value<CBtDevice *>();
    if (pDev == NULL)
        return;

    m_settings.beginGroup((cb != ui->cbBLEDeviceList) ? "devices" : "devicesLE");
    QString strBda;
    strBda.sprintf("%02X:%02X:%02X:%02X:%02X:%02X",
        pDev->m_address[0], pDev->m_address[1], pDev->m_address[2], pDev->m_address[3], pDev->m_address[4], pDev->m_address[5]);
    m_settings.remove(strBda);
    m_settings.endGroup();

    m_settings.beginGroup("NVRAM");
    QString nvram_id_key;
    nvram_id_key.sprintf("%02X",pDev->m_nvram_id);
    m_settings.remove(nvram_id_key);
    m_settings.endGroup();

    m_settings.sync();

    BYTE   cmd[10];
    cmd[0] = (BYTE)pDev->m_nvram_id;
    cmd[1] = (BYTE)(pDev->m_nvram_id >> 8);
    SendWicedCommand(HCI_CONTROL_COMMAND_DELETE_NVRAM_DATA, cmd, 2);

    pDev->m_paired = false;
    pDev->m_nvram_id = -1;
    pDev->m_nvram.clear();

    if(cb == ui->cbBLEDeviceList)
        ui->btnBLEUnbond->setEnabled(false);
    else
        ui->btnUnbond->setEnabled(false);

    cb->setItemIcon(cb->currentIndex(), *new QIcon);
}

void MainWindow::OnBnClickedBREDRUnbond()
{
    onUnbond(ui->cbDeviceList);
}

void MainWindow::OnBnClickedLeUnbond()
{
    onUnbond(ui->cbBLEDeviceList);
}

void MainWindow::onReset()
{
    if (m_CommPort == NULL)
        return;

    if (!m_bPortOpen)
    {
         return;
    }

    if (QMessageBox::Yes != QMessageBox(QMessageBox::Information, "Reset", "Are you sure you want to reset? After resetting the device, you may need to download the embedded application again.", QMessageBox::Yes|QMessageBox::No).exec())
    {
        return;
    }

    // send command to reset
    SendWicedCommand(HCI_CONTROL_COMMAND_RESET, 0, 0);

    // close comm port reader thread and disable UI
    QThread::msleep(10);

    ClearPort();

}

int MainWindow::FindBaudRateIndex(int baud)
{
    int index = 0;

    for (; index < sizeof(as32BaudRate) / sizeof(as32BaudRate[0]); index++)
        if (as32BaudRate[index] == baud)
            return index;

    return 0;
}

void MainWindow::onDevChange(QString)
{
    CBtDevice * pDev = GetSelectedDevice();
    if (pDev)
    {        
         ui->btnUnbond->setEnabled(pDev->m_paired);
    }
}

void MainWindow::onLEDevChange(QString)
{
    CBtDevice * pDev = GetSelectedLEDevice();
    if (pDev)
    {
        ui->btnBLEUnbond->setEnabled(pDev->m_paired);
    }
}


void MainWindow::onCommPortChange(QString newPort)
{

}

void MainWindow::CloseCommPort()
{
    if (m_port_read_worker)
    {
        m_port_read_worker->m_bClosing = true;
        serial_read_wait.wakeAll();

        m_port_read_thread->exit();
        if (!m_port_read_thread->wait(3500))
        {
            m_port_read_thread->terminate();
            m_port_read_thread->wait(1000);
        }

        delete m_port_read_thread;

        m_port_read_worker = NULL;
        m_port_read_thread = NULL;
    }

    if (m_CommPort && m_CommPort->isOpen())
        m_CommPort->close();

    m_bPortOpen = false;
}

void MainWindow::closeEventDm (QCloseEvent *)
{
    CloseCommPort();

    // save settings for baudrate, serial port and flow-ctrl
    QSettings settings(m_SettingsFile, QSettings::IniFormat);
    int baud_rate = ui->cbBaudRate->currentText().toInt();
    m_settings.setValue("Baud",baud_rate);

    bool flow_control = ui->btnFlowCntrl->isChecked();
    m_settings.setValue("FlowControl",flow_control);

    QString comm_port = ui->cbCommport->currentText();
    m_settings.setValue("port",comm_port);

    m_settings.setValue("LocalBDA",ui->edLocalBDA->text());
}


unsigned int MainWindow::SendWicedCommand(unsigned short command, unsigned char * payload, unsigned int len)
{
    char    data[5040];
    memset(data, 0, 5040);

    int     header = 0;

    if (NULL == m_CommPort)
        return -1;
    if (!m_CommPort->isOpen())
        return -1;

    // Since the write to serial port can happen from main thread as well as
    // audio wav thread, serialize the write of WICED HCI packets
    m_write.lock();

    if (command)
    {
        data[header++] = HCI_WICED_PKT;
        data[header++] = command & 0xff;
        data[header++] = (command >> 8) & 0xff;
        data[header++] = len & 0xff;
        data[header++] = (len >> 8) & 0xff;
    }
    memcpy(&data[header], payload, len);

    DWORD Length = header+len;

    if(Length > 5040)
    {
        Log("Something wrong, lenght %d", Length);
    }

    char * p = &data[0];

    DWORD dwWritten = 0;
    DWORD dwTotalWritten = 0;

    while (Length && m_bPortOpen)
    {
        dwWritten = 0;

        dwWritten = m_CommPort->write(p, Length);
        m_CommPort->flush();

        if(dwWritten == -1)
        {
            Log("error - port write error");
            break;
        }

        if(Length != dwWritten)
        {
            if(!m_bPortOpen)
                break;

            Log("port write mismatch, to write %d, written %d, wait and try", Length, dwWritten);

            if (!m_CommPort->waitForBytesWritten(100))
            {
                Log("error - waitForBytesWritten");
                break;
            }

            if(!m_bPortOpen)
                break;
        }
        if (dwWritten > Length)
            break;

        Length -= dwWritten;

        if(Length)
            p += dwWritten;

        dwTotalWritten += dwWritten;
    }

    m_write.unlock();


    return dwTotalWritten;
}

// BR/EDR discovery start
void MainWindow::onStartDisc()
{
    if (!m_inquiry_active)
    {
        m_inquiry_active = TRUE;

        // Clear all items from combo box except paired devices
        ResetDeviceList(ui->cbDeviceList);

        BYTE command[] = { 1 };
        int r = SendWicedCommand(HCI_CONTROL_COMMAND_INQUIRY, command, 1);
        if (r <= 0)
        {
            Log("Error starting inquiry");
            m_inquiry_active = FALSE;
        }
    }

    ui->btnStartDisc->setEnabled(!m_inquiry_active);
    ui->btnStopDisc->setEnabled(m_inquiry_active);
}

// BR/EDR discovery start
void MainWindow::onStopDisc()
{
    m_inquiry_active = FALSE;

    BYTE command[] = { 0 };
    if (SendWicedCommand(HCI_CONTROL_COMMAND_INQUIRY, command, 1) <= 0)
    {
        Log("Error stopping inquiry");
    }

    ui->btnStartDisc->setEnabled(!m_inquiry_active);
    ui->btnStopDisc->setEnabled(m_inquiry_active);
}

// LE discovery start
void MainWindow::OnBnClickedDiscoverDevicesStart()
{
    if (!m_scan_active)
    {
        m_scan_active = true;

        // Clear all items from combo box except paired devices
        ResetDeviceList(ui->cbBLEDeviceList);

        // scan command, len 1, enable = 1
        BYTE command[] = { 1 };
        SendWicedCommand(HCI_CONTROL_LE_COMMAND_SCAN, command, 1);
    }

    setGATTUI();
}

// Clear all items from combo box except paired devices
void MainWindow::ResetDeviceList(QComboBox *cb)
{
    CBtDevice *device = NULL;
    QComboBox temp;
    int i = 0;
    // Save the paired devices to a temp combo box
    for(i = 0; i < cb->count(); i++)
    {
        device = (CBtDevice *)cb->itemData(i).value<CBtDevice *>();
        if(device && device->m_paired)
        {
            QVariant qv;
            qv.setValue<CBtDevice *>(device);

            temp.addItem(cb->itemText(i), qv);
        }
    }

    // clear the device combo box
    cb->clear();

    // add the paired devices back to device combo box
    for(i = 0; i < temp.count(); i++)
    {
        device = (CBtDevice *)temp.itemData(i).value<CBtDevice *>();
        if(device)
        {
            QVariant qv;
            qv.setValue<CBtDevice *>(device);

            cb->addItem(m_paired_icon, temp.itemText(i), qv);
        }
    }
}

// LE discovery stop
void MainWindow::OnBnClickedDiscoverDevicesStop()
{
    if (m_scan_active)
    {
        m_scan_active = false;
        // scan command, len 1, enable = 0
        BYTE command[] = { 0 };
        SendWicedCommand(HCI_CONTROL_LE_COMMAND_SCAN, command, 1);
    }

    setGATTUI();
}

CBtDevice * MainWindow::GetSelectedDevice()
{
    int i = ui->cbDeviceList->currentIndex();
    if ( i == -1)
        return NULL;

    QVariant var;    
    var = ui->cbDeviceList->itemData(i);
    return var.value<CBtDevice *>();    
}

CBtDevice * MainWindow::GetSelectedLEDevice()
{
    int i = ui->cbBLEDeviceList->currentIndex();
    if ( i == -1)
        return NULL;

    QVariant var;
    var = ui->cbBLEDeviceList->itemData(i);
    return var.value<CBtDevice *>();
}


bool MainWindow::SetupCommPort()
{
    if (m_CommPort)
    {
        ClearPort();
    }

    m_CommPort = new QSerialPort(this);

    QString serialPortName = ui->cbCommport->currentText();

    m_CommPort->setPortName(serialPortName);

    int serialPortBaudRate = as32BaudRate[ui->cbBaudRate->currentIndex()];
    m_CommPort->setBaudRate(serialPortBaudRate);
    m_CommPort->setFlowControl(ui->btnFlowCntrl->isChecked() ? QSerialPort::HardwareControl : QSerialPort::NoFlowControl);
    m_CommPort->setStopBits(QSerialPort::OneStop);
    m_CommPort->setDataBits(QSerialPort::Data8);
    m_CommPort->setParity(QSerialPort::NoParity);





    if (m_bPortOpen = m_CommPort->open(QIODevice::ReadWrite))
    {

        m_CommPort->clear();
        connect(m_CommPort, SIGNAL(readyRead()), SLOT(handleReadyRead()));

        Log("Opened %s at speed: %u flow %s", serialPortName.toStdString().c_str(), serialPortBaudRate,
                ui->btnFlowCntrl->isChecked() ? "on":"off");        

        CreateReadPortThread();
        m_port_read_thread->start();

        p_dm_timer->start(1000); // start a 1 sec timer to set pairing mode and visibility
    }
    else
    {
        Log("Error opening serial port %s: Error number %d", serialPortName.toStdString().c_str(), (int)m_CommPort->error());
    }

    return m_bPortOpen;
}

void MainWindow::handleReadyRead()
{
   serial_read_wait.wakeAll();
}

void MainWindow::on_dm_timer()
{
    p_dm_timer->stop();
    setVis();
    setPairingMode();
    GetVersion();
}

void MainWindow::setPairingMode()
{
    if(!m_CommPort)
        return;

    if (!m_CommPort->isOpen())
        return;

    BYTE pairing_mode=ui->btnPairable->isChecked() ? 1 : 0;
    SendWicedCommand(HCI_CONTROL_COMMAND_SET_PAIRING_MODE, &pairing_mode, 1);
}

void MainWindow::setVis()
{
    if(!m_CommPort)
        return;

    if (!m_CommPort->isOpen())
        return;

    BYTE   cmd[60];
    int    commandBytes = 0;

    cmd[commandBytes++] = ui->btnDiscoverable->isChecked() ? 1:0; //discoverable
    cmd[commandBytes++] = ui->btnConnectable->isChecked() ? 1:0; ; //CONNECTABLE

    SendWicedCommand(HCI_CONTROL_COMMAND_SET_VISIBILITY, cmd, commandBytes);
}


DWORD min(DWORD len, DWORD bufLen)
{
    return (len < bufLen) ? len : bufLen;
}

void DumpData(char *description, void* p, unsigned int length, unsigned int max_lines)
{
    char    buff[100];
    unsigned int    i, j;
    char    full_buff[3000];

    if (p != NULL)
    {
        for (j = 0; j < max_lines && (32 * j) < length; j++)
        {
            for (i = 0; (i < 32) && ((i + (32 * j)) < length); i++)
            {
                sprintf(&buff[3 * i], "%02x \n", ((UINT8*)p)[i + (j * 32)]);
            }
            if (j == 0)
            {
                strcpy(full_buff, description);
                strcat(full_buff, buff);
                //qDebug(full_buff);
            }
            else
            {
                //qDebug(buff);
            }
        }
    }
}


CBtDevice *MainWindow::FindInList(BYTE * addr, QComboBox * pCb)
{
    int j;
    CBtDevice *device = NULL;
    for (int i = 0; i < pCb->count(); i++)
    {
        device = (CBtDevice *)pCb->itemData(i).value<CBtDevice *>();
        if (device == NULL)
            continue;
        for (j = 0; j < 6; j++)
        {
            if (device->m_address[j] != addr[j])
                break;
        }
        if (j == 6)
        {
            return device;
        }
    }
    return NULL;
}

CBtDevice *MainWindow::FindInList(UINT16 conn_type, UINT16 handle, QComboBox * pCb)
{
    CBtDevice *device = NULL;
    for (int i = 0; i < pCb->count(); i++)
    {
        device = (CBtDevice *)pCb->itemData(i).value<CBtDevice *>();
        if (device == NULL)
            continue;

        if((device->m_conn_type && conn_type) && (device->m_hf_handle == handle))
            return device;
    }
    return NULL;
}

void MainWindow::SelectDevice(QComboBox* cb, BYTE * bda)
{
    int j;
    CBtDevice *device = NULL;
    for (int i = 0; i < cb->count(); i++)
    {
        device = (CBtDevice *)cb->itemData(i).value<CBtDevice *>();
        if (device == NULL)
            continue;
        for (j = 0; j < 6; j++)
        {
            if (device->m_address[j] != bda[j])
                break;
        }
        if (j == 6)
        {
            cb->setCurrentIndex(i);
            break;
        }
    }
}

// Set device status to paired
void MainWindow::SetDevicePaired(BYTE * bda)
{
    QString strBda;
    strBda.sprintf("%02X:%02X:%02X:%02X:%02X:%02X",
                   bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

    bool bLeDevice = false;
    int i = ui->cbDeviceList->findText(strBda,Qt::MatchStartsWith);
    if (i == -1)
    {
        i = ui->cbBLEDeviceList->findText(strBda,Qt::MatchStartsWith);
        if(i >= 0)
            bLeDevice = true;
    }
    if (i == -1)
        return;

    CBtDevice * device = NULL;
    if(!bLeDevice)
        device = (CBtDevice *)ui->cbDeviceList->itemData(i).value<CBtDevice *>();
    else
        device = (CBtDevice *)ui->cbBLEDeviceList->itemData(i).value<CBtDevice *>();

    if (device == NULL)
        return;

    if (device->m_paired)
        return;

    device->m_paired = true;

    if(!bLeDevice)
        ui->cbDeviceList->setItemIcon(i,m_paired_icon);
    else
        ui->cbBLEDeviceList->setItemIcon(i,m_paired_icon);

    m_settings.beginGroup(bLeDevice? "devicesLE" : "devices");
    m_settings.setValue(strBda,device->m_name);
    m_settings.endGroup();
    m_settings.sync();
    if(!bLeDevice)
        ui->btnUnbond->setEnabled( (i == ui->cbDeviceList->currentIndex()));
    else
        ui->btnUnbond->setEnabled( (i == ui->cbBLEDeviceList->currentIndex()));
}

// Add new device to combo box
CBtDevice *MainWindow::AddDeviceToList(BYTE *addr, QComboBox * pCb, char * bd_name, bool paired)
{
    CBtDevice *device = NULL;
    QString abuffer;

    if (bd_name)
        abuffer.sprintf("%02x:%02x:%02x:%02x:%02x:%02x (%s)",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],bd_name);
    else
        abuffer.sprintf("%02x:%02x:%02x:%02x:%02x:%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Check if device is already present
    int i = pCb->findText( abuffer,Qt::MatchStartsWith);
    if (i == -1)
    {
        QVariant qv;

        device = new CBtDevice(paired);
        if (bd_name && strlen(bd_name))
            strncpy(device->m_name, bd_name,sizeof(device->m_name)-1);

        qv.setValue<CBtDevice *>(device);        

        if (paired)
        {
            pCb->addItem( m_paired_icon, abuffer, qv);
        }
        else
        {
           pCb->addItem( abuffer, qv );
        }

        memcpy(device->m_address, addr, 6);
    }
    else
    {
        // device already in list

        // If paired, set icon
        if(paired)
            pCb->setItemIcon(i,m_paired_icon);


        device = (CBtDevice *)pCb->itemData(i).value<CBtDevice*>();
    }

    return device;
}

void MainWindow::processAddDeviceItem(QVariant qvcb, QString sz, QVariant qv)
{
    QComboBox* pCb = (QComboBox*) qvcb.value<QComboBox *>();
    pCb->addItem( sz, qv );
}

void MainWindow::WriteNVRAMToDevice(bool bBLEDevice)
{
    QVariant var;
    CBtDevice * pDev = NULL;
    BYTE cmd[300];
    int len=0;

    QComboBox *cb = ui->cbDeviceList;
    if(bBLEDevice)
        cb = ui->cbBLEDeviceList;

    for (int i = 0; i < cb->count(); i++)
    {
        var = cb->itemData(i);
        if (NULL == (pDev = var.value<CBtDevice *>()))
            continue;
        if (pDev->m_nvram_id == -1 || pDev->m_nvram.size() == 0)
            continue;

        cmd[0] = (BYTE)pDev->m_nvram_id;
        cmd[1] = (BYTE)(pDev->m_nvram_id >> 8);
        if ( (len = pDev->m_nvram.size()) > sizeof(cmd)-2)
            len = sizeof(cmd)-2;

        memcpy(&cmd[2], pDev->m_nvram.constData(), len);
        SendWicedCommand(HCI_CONTROL_COMMAND_PUSH_NVRAM_DATA, cmd, 2 + len);
    }
}

void MainWindow::HandleDeviceEventsDm(DWORD opcode, LPBYTE p_data, DWORD len)
{
    BYTE    bda[6];
    char trace[1024];    

    switch (opcode)
    {
    case HCI_CONTROL_EVENT_WICED_TRACE:
        if (len >= 2)
        {
            if ((len > 2) && (p_data[len - 2] == '\n'))
            {
                p_data[len - 2] = 0;
                len--;
            }
            TraceHciPkt(0, p_data, (USHORT)len);
        }

        break;
    case HCI_CONTROL_EVENT_HCI_TRACE:

        TraceHciPkt(p_data[0] + 1, &p_data[1], (USHORT)(len - 1));
        break;

    case HCI_CONTROL_EVENT_NVRAM_DATA:
    {
        QString nvram_id_key;
        nvram_id_key.sprintf("%02X", p_data[0] + (p_data[1] << 8));
        m_settings.beginGroup("NVRAM");
        QByteArray val =  (const char *)&p_data[2];
        m_settings.setValue(nvram_id_key,val);
        m_settings.endGroup();
        m_settings.sync();

        Log("HCI_CONTROL_EVENT_NVRAM_DATA %s", nvram_id_key.toLocal8Bit().data());

        // if it is a link key add this device to the appropriate combobox
        if (len - 2)
        {
            // application should probably send type of the device, if host does not remember.
            // for now we will use the fact that BR/EDR keys are just 16 bytes.
            BOOL bLeDevice = (p_data[25] | p_data[26] | p_data[27]) != 0;

            CBtDevice * pDev = AddDeviceToList(&p_data[2], bLeDevice ? ui->cbBLEDeviceList : ui->cbDeviceList, NULL, true);
            pDev->m_nvram_id =  p_data[0] + (p_data[1] << 8);
            pDev->m_nvram = QByteArray ((const char *)&p_data[2], len - 2);
            if(bLeDevice)
                pDev->m_bIsLEDevice = true;
            pDev->m_paired = true;

            if(!bLeDevice)
                m_settings.beginGroup("devices");
            else
                m_settings.beginGroup("devicesLE");

            QString dev_key;
            dev_key.sprintf("%02X:%02X:%02X:%02X:%02X:%02X",
                pDev->m_address[0], pDev->m_address[1], pDev->m_address[2],
                pDev->m_address[3], pDev->m_address[4], pDev->m_address[5] );
            QString name_id;
            name_id.sprintf("%02X:%s", pDev->m_nvram_id, pDev->m_name);
            m_settings.setValue(dev_key,name_id);
            m_settings.endGroup();
            m_settings.sync();

            Log("device (LE? %d) %s, ID %s", bLeDevice, dev_key.toLocal8Bit().data(), name_id.toLocal8Bit().data());

        }
    }
    break;

    case HCI_CONTROL_EVENT_USER_CONFIRMATION:
    {
        Log("Numeric Comparison %02x:%02x:%02x:%02x:%02x:%02x Value:%d", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6] + (p_data[7] << 8) + (p_data[8] << 16) + (p_data[9] << 24));
        QString str;
        str.sprintf("Confirm pairing code: %d", p_data[6] + (p_data[7] << 8) + (p_data[8] << 16) + (p_data[9] << 24));
        if (QMessageBox::Yes != QMessageBox(QMessageBox::Information, "User Confirmation", str, QMessageBox::Yes|QMessageBox::No).exec())
        {
            break;
        }

        // send command to confirm user confirmation value
        {
            BYTE command[20];

            for (int i = 0; i < 6; i++)
                command[5 - i] = p_data[i];

            command[6] = 1; // 1 - accept, 0 - do not accept

            SendWicedCommand(HCI_CONTROL_COMMAND_USER_CONFIRMATION, command, 7);            
        }
    }
        break;

    case HCI_CONTROL_EVENT_DEVICE_STARTED:
        Log("Device Started");
        m_scan_active = false;
        m_inquiry_active = false;
        ui->btnStartDisc->setEnabled(!m_inquiry_active);
        ui->btnStopDisc->setEnabled(m_inquiry_active);
        setGATTUI();
        WriteNVRAMToDevice(false);
        WriteNVRAMToDevice(true);
        break;

    case HCI_CONTROL_EVENT_PAIRING_COMPLETE:
    {        
        Log("Pairing status: %s, code: %d", (p_data[0] == 0) ? "Success" : "Fail", (int)p_data[0]);
        if(len >= 7)
        {
            BYTE command[20];
            for (int i = 0; i < 6; i++)
                command[5 - i] = p_data[i+1];
            Log("BDA %02x:%02x:%02x:%02x:%02x:%02x",
                command[0], command[1], command[2], command[3], command[4], command[5]);

            SetDevicePaired(&p_data[1]);
        }
        break;
    }

    case HCI_CONTROL_EVENT_INQUIRY_RESULT:
    {        
        Log("Device found: %02x:%02x:%02x:%02x:%02x:%02x device_class %02x:%02x:%02x rssi:%d",
            (int)p_data[5], (int)p_data[4], (int)p_data[3], (int)p_data[2], (int)p_data[1], (int)p_data[0],
                (int)p_data[8], (int)p_data[7], (int)p_data[6],(int) p_data[9] - 256);

        char szName[50] = {0};

        if (len > 10)
        {
            DecodeEIR(&p_data[10], len - 10, szName,sizeof(szName));
        }

        // dump advertisement data to the trace
        /*
        if (len > 10)
        {
            trace[0] = 0;
            for (int i = 0; i < (int)len - 10; i++)
                sprintf(&trace[strlen(trace)], "%02x", p_data[10 + i]);
            Log(trace);

        }
        */
        for (int i = 0; i < 6; i++)
            bda[5 - i] = p_data[i];
        // check if this device is not present yet.
        if (FindInList(bda,ui->cbDeviceList) != NULL)
            break;
        AddDeviceToList(bda,ui->cbDeviceList, szName);
    }
        break;

    case HCI_CONTROL_EVENT_INQUIRY_COMPLETE:
        if (m_inquiry_active)
        {
            m_inquiry_active = FALSE;
            Log("Inquiry Complete");
        }
        ui->btnStartDisc->setEnabled(!m_inquiry_active);
        ui->btnStopDisc->setEnabled(m_inquiry_active);

        break;
    }
}


void GetTagDesc(char *desc, BYTE tag)
{
    switch (tag)
    {
    case 0x01: sprintf(&desc[strlen(desc)], "Flags"); break;
    case 0x02: sprintf(&desc[strlen(desc)], "MORE UUID16"); break;
    case 0x03: sprintf(&desc[strlen(desc)], "UUID16"); break;
    case 0x04: sprintf(&desc[strlen(desc)], "MORE UUID32"); break;
    case 0x05: sprintf(&desc[strlen(desc)], "UUID32"); break;
    case 0x06: sprintf(&desc[strlen(desc)], "MORE UUID128"); break;
    case 0x07: sprintf(&desc[strlen(desc)], "UUID128"); break;
    case 0x08: sprintf(&desc[strlen(desc)], "Name(Short)"); break;
    case 0x09: sprintf(&desc[strlen(desc)], "Name"); break;
    case 0x0A: sprintf(&desc[strlen(desc)], "TxPower"); break;
    case 0x0C: sprintf(&desc[strlen(desc)], "BdAddr"); break;
    case 0x0D: sprintf(&desc[strlen(desc)], "COD"); break;
    case 0xFF: sprintf(&desc[strlen(desc)], "Manufacturer"); break;
    default: sprintf(&desc[strlen(desc)], "(0x%02X)", tag); break;
    }
}

// returns device name if present in data
void MainWindow::DecodeEIR(LPBYTE p_data, DWORD len, char * szName, int name_len)
{
    char trace[1024] = {0};
    //static char bd_name[100]={0};
    BYTE tag;
    int i, tag_len;

    //szName[0]=0;
    memset(szName, 0, name_len);
    while (len >= 2)
    {
        //trace[0] = '\0';
        memset(trace, 0, 1024);

        tag_len = (int)(unsigned int)*p_data++;
        tag = *p_data++;
        sprintf(trace, " -Tag:");
        GetTagDesc(trace, tag);
        sprintf(&trace[strlen(trace)], ":");
        switch (tag)
        {
        case 0x02: // UUID16 More
        case 0x03: // UUID16
            unsigned short uuid16;           

            for (i = 0; i < tag_len - 1; i += 2)
            {
                uuid16 = *p_data++;
                uuid16 |= (*p_data++ << 8);
                sprintf(&trace[strlen(trace)], "%04X ", uuid16);
            }
            break;

        case 0x04: // UUID32 More
        case 0x05: // UUID32

            unsigned long uuid32;
            for (i = 0; i < tag_len - 1; i += 2)
            {
                uuid32 = *p_data++;
                uuid32 |= (*p_data++ << 8);
                uuid32 |= (*p_data++ << 16);
                uuid32 |= (*p_data++ << 24);
                sprintf(&trace[strlen(trace)], "%08X ", uuid32);
            }
            break;

        case 0x08: // Shortened name
        case 0x09: // Name


            for (i = 0; i < tag_len - 1; i++)
            {
                if (i < (name_len-1))
                    szName[i] = *p_data;
                sprintf(&trace[strlen(trace)], "%c", *p_data++);
            }
            break;

        default:


            for (i = 0; i < tag_len - 1; i++)
                sprintf(&trace[strlen(trace)], "%02X ", *p_data++);
            break;
        }        
        len -= tag_len + 1;

        Log(trace);
    }    
}

void MainWindow::onHandleWicedEventDm(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    if(!m_bUIEnabled)
    {
        EnableUI(true);
        EnableAppTraces();
        Log("Client Control app estabilshed communication with Bluetooth device.");
    }

    switch (HCI_CONTROL_GROUP(opcode))
    {
        case HCI_CONTROL_GROUP_DEVICE:
            HandleDeviceEventsDm(opcode, p_data, len);
            break;
        case HCI_CONTROL_GROUP_MISC:
            HandleDeviceEventsMisc(opcode, p_data, len);
            break;

    }
}

void MainWindow::on_btnOpenPort_clicked()
{
    if(!m_bPortOpen)
    {
        EnableUI(false);
        ui->btnOpenPort->setEnabled(false);
        bool bopen = SetupCommPort();
        ui->btnOpenPort->setEnabled(true);

        if(!bopen)
        {
            QMessageBox(QMessageBox::Information, "Serial Port", "Error opening serial port", QMessageBox::Ok).exec();
        }
        else
        {
            ui->cbCommport->setEnabled(false);
            ui->btnOpenPort->setText("Close Port");
            ui->cbBaudRate->setEnabled(false);
            ui->btnFlowCntrl->setEnabled(false);
        }
    }
    else
    {
        ClearPort();
    }
}


void MainWindow::EnableUI(bool bEnable)
{
    if((m_features != 0xFFFF) && bEnable)
    {
        if(m_features & HCI_CONTROL_GROUP_GATT)
            ui->tabGATT->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_HF)
            ui->tabHF->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_SPP)
            ui->tabSPP->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_AUDIO)
            ui->tabAVSRC->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_HIDD)
            ui->tabHIDD->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_AVRC_TARGET)
            ui->tabAVRCTG->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_IAP2)
            ui->tabIAP2->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_AG)
            ui->tabAG->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_BSG)
            ui->tabBSG->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_AVRC_CONTROLLER)
            ui->tabAVRCCT->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_AUDIO)
            ui->tabAVSink->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_HIDH)
            ui->tabHIDH->setEnabled(bEnable);
        if(m_features & HCI_CONTROL_GROUP_HK)
            ui->tabHK->setEnabled(bEnable);
    }
    else
    {
        ui->tabAG->setEnabled(bEnable);
        ui->tabHF->setEnabled(bEnable);
        ui->tabAVSRC->setEnabled(bEnable);
        ui->tabAVRCCT->setEnabled(bEnable);
        ui->tabAVRCTG->setEnabled(bEnable);
        ui->tabHIDD->setEnabled(bEnable);
        ui->tabHIDH->setEnabled(bEnable);
        ui->tabSPP->setEnabled(bEnable);
        ui->tabIAP2->setEnabled(bEnable);
        ui->tabHK->setEnabled(bEnable);
        ui->tabGATT->setEnabled(bEnable);
        ui->tabAVSink->setEnabled(bEnable);
        ui->tabBSG->setEnabled(bEnable);
    }

    ui->btnStartDisc->setEnabled(bEnable);
    ui->btnBLEStartDisc->setEnabled(bEnable);
    ui->btnBLEStopDisc->setEnabled(bEnable );
    ui->btnReset->setEnabled(bEnable);    
    ui->btnStopDisc->setEnabled(bEnable);
    ui->btnPairable->setEnabled(bEnable);
    ui->btnDiscoverable->setEnabled(bEnable);
    ui->btnConnectable->setEnabled(bEnable);

    m_bUIEnabled = bEnable;

}

void MainWindow::ClearPort()
{
    CloseCommPort();
    QThread::sleep(1);
    if(m_CommPort)
        delete m_CommPort;
    m_CommPort = NULL;

    Log("Serial port closed.");
    if(m_bUIEnabled)
        Log("Client Control app disconnected from Bluetooth device.");

    EnableUI(false);
    ui->cbCommport->setEnabled(true);
    ui->cbBaudRate->setEnabled(true);
    ui->btnFlowCntrl->setEnabled(true);
    ui->btnOpenPort->setText("Open Port");


}

void MainWindow::EnableAppTraces()
{
    BYTE cmd[2];
    cmd[0] = TRUE; // Enable Bluetooth HCI trace
    cmd[1] = 1;    //  WICED_ROUTE_DEBUG_TO_WICED_UART;

    // send command to configure traces
    SendWicedCommand(HCI_CONTROL_COMMAND_TRACE_ENABLE, cmd, 2);
}

void MainWindow::CreateReadPortThread()
{
    m_port_read_thread = new QThread;
    m_port_read_worker = new Worker();
    m_port_read_worker->moveToThread(m_port_read_thread);
    m_port_read_worker->m_bClosing = false;
    m_port_read_worker->m_pParent = this;


    connect(m_port_read_thread, SIGNAL(started()), m_port_read_worker, SLOT(read_serial_port_thread()));
    connect(m_port_read_worker, SIGNAL(finished()), m_port_read_thread, SLOT(quit()));
    connect(m_port_read_worker, SIGNAL(finished()), m_port_read_worker, SLOT(deleteLater()));
    connect(m_port_read_thread, SIGNAL(finished()), m_port_read_thread, SLOT(deleteLater()));
}


// Serial port read thread
void Worker::read_serial_port_thread()
{
    unsigned char au8Hdr[1024 + 6];
    memset(au8Hdr, 0, 1030);
    int           offset = 0, pktLen;
    int           packetType;

    while (!m_bClosing)
    {
        memset(au8Hdr, 0, 1030);
        offset = 0;
        // Read HCI packet
        pktLen = ReadNewHciPacket(au8Hdr, sizeof(au8Hdr), &offset);
        if (m_bClosing)
            break;

        if(pktLen < 0) // skip this
            continue;

        if (pktLen + offset == 0)
            continue;

        packetType = au8Hdr[0];
        if (HCI_WICED_PKT == packetType)
        {
            DWORD channel_id = au8Hdr[1] | (au8Hdr[2] << 8);
            DWORD len = au8Hdr[3] | (au8Hdr[4] << 8);

            if(len < 0)
                continue;

            if (len > 1024)
            {
                m_pParent->Log("Skip bad packet %d", len);
                continue;
            }
            BYTE * pBuf = NULL;

            if(len)
            {
                // malloc and create a copy of the data.
                //  MainWindow::onHandleWicedEvent deletes the data
                pBuf = (BYTE*)malloc(len);
                memcpy(pBuf, &au8Hdr[5], len);
            }

            // send it to main thread
            emit m_pParent->HandleWicedEvent(channel_id, len, pBuf);
        }
    }

    emit finished();
}

// Read HCI packet
DWORD Worker::ReadNewHciPacket(BYTE * pu8Buffer, int bufLen, int * pOffset)
{
    DWORD dwLen, len = 0, offset = 0;

    dwLen = Read(&pu8Buffer[offset], 1);

    if (dwLen == 0 || m_bClosing)
        return (0);

    offset++;

    switch (pu8Buffer[0])
    {
    case HCI_EVENT_PKT:
    {
        dwLen = Read(&pu8Buffer[offset], 2);
        if(dwLen == 2)
        {
            len = pu8Buffer[2];
            offset += 2;
            m_pParent->Log("HCI_EVENT_PKT len %d", len);
        }
        else
            m_pParent->Log("error HCI_EVENT_PKT, needed 2 got %d", dwLen);        
    }
        break;

    case HCI_ACL_DATA_PKT:
    {
        dwLen = Read(&pu8Buffer[offset], 4);
        if(dwLen == 4)
        {
            len = pu8Buffer[3] | (pu8Buffer[4] << 8);
            offset += 4;
            m_pParent->Log("HCI_ACL_DATA_PKT, len %d", len);
        }
        else
            m_pParent->Log("error HCI_ACL_DATA_PKT needed 4 got %d", dwLen);        
    }
        break;

    case HCI_WICED_PKT:
    {
        dwLen = Read(&pu8Buffer[offset], 4);
        if(dwLen == 4)
        {
            len = pu8Buffer[3] | (pu8Buffer[4] << 8);
            offset += 4;
        }
        else
            m_pParent->Log("error HCI_WICED_PKT,  needed 4 got %d", dwLen);
    }
        break;
    default:
    {
        if(m_pParent->m_bUIEnabled)
            m_pParent->Log("error unknown packet, type %d", pu8Buffer[0]);
    }
    }

    if(len > 1024)
    {
        m_pParent->Log("bad packet %d", len);
        return -1; // bad packet
    }

    if (len)
    {
        DWORD lenRd = min(len, (DWORD)(bufLen-offset));
        dwLen = Read(&pu8Buffer[offset], lenRd);
        if(dwLen != lenRd)
            m_pParent->Log("read error to read %d, read %d", lenRd, dwLen);
    }

    *pOffset = offset;

    return len;
}

// Read from serial port
DWORD Worker::Read(BYTE *lpBytes, DWORD dwLen)
{
    BYTE * p = lpBytes;
    DWORD Length = dwLen;
    DWORD dwRead = 0;
    DWORD dwTotalRead = 0;

    if(!m_pParent->m_CommPort)
        return 0;

    QMutex mutex;
    int retry_cnt = 0;
    // Loop here until request is fulfilled or port is closed
    while (Length && !m_bClosing)
    {
        if(m_bClosing)
            return 0;

        dwRead = 0;
        char buff_temp[1030];
        memset(buff_temp, 0, 1030);

        dwRead = m_pParent->m_CommPort->read(buff_temp,Length);
        if(dwRead <= 0)
        {
            if(dwRead < 0)
            {
                m_pParent->Log("Error in port read");
                return 0;
            }

            // retry 3 time with longer timeout for each subsequent
            unsigned long ulTimeout = 20;
            if(retry_cnt < 3)
            {
                retry_cnt++;
                ulTimeout *= retry_cnt;
            }
            else
                ulTimeout = ULONG_MAX;

            //qDebug("serial_read wait Length %d, timeout %d, retry %d", Length, ulTimeout, retry_cnt);
            // If dwRead == 0, then we need to wait for device to send the MCU app data
            mutex.lock();
            // serial_read_wait is set when there is more data or when the serial port is closed
            m_pParent->serial_read_wait.wait(&mutex, ulTimeout);
            mutex.unlock();
            //qDebug("serial_read continue");
        }
        else
        {
            memcpy(p, buff_temp, dwRead);
            retry_cnt = 0;
        }

        if (dwRead > Length)
            break;
        p += dwRead;
        Length -= dwRead;
        dwTotalRead += dwRead;
    }

    return dwTotalRead;
}

void MainWindow::GetVersion()
{
    // send command to get version and feature info
    SendWicedCommand(HCI_CONTROL_MISC_COMMAND_GET_VERSION, NULL, 0);
}

void MainWindow::HandleDeviceEventsMisc(DWORD opcode, LPBYTE tx_buf, DWORD len)
{
    int index = 0;

    switch (opcode)
    {
    case HCI_CONTROL_MISC_EVENT_VERSION:
        m_major = tx_buf[index++];
        m_minor = tx_buf[index++];
        m_rev =  tx_buf[index++];
        m_build = tx_buf[index++] | (tx_buf[index++] << 8);
        m_chip = tx_buf[index++] | (tx_buf[index++] << 8) | (tx_buf[index++] << 24);
        m_power = tx_buf[index++];
        if(len < 10) // old API len is 9
            break;
        m_features = tx_buf[index++] | (tx_buf[index++] << 8);

        break;
    }
}
