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


 static const CHAR *szScanState[] =
 {
     "Scan:None",
     "Scan:High",
     "Scan:Low",
     "Conn:High",
     "Conn:Low",
 };

 static const CHAR *szAdvState[] =
 {
     "Advertisement:Not discoverable",
     "Advertisement:low directed",
     "Advertisement:high directed",
     "Advertisement:low undirected",
     "Advertisement:high undirected",
 };

 static const char *szStatusCode[] =
 {
     "Success",
     "In Progress",
     "Connected",
     "Not Connected",
     "Bad Handle",
     "Wrong State",
     "Invalid Args",
 };

void MainWindow::InitGATT()
{
    // setup signals/slots

    connect(ui->btnBLEConnect, SIGNAL(clicked()), this, SLOT(OnBnClickedLeConnect()));
    connect(ui->btnBLECancelConnect, SIGNAL(clicked()), this, SLOT(OnBnClickedLeCancelConnect()));
    connect(ui->btnBLEDisconnect, SIGNAL(clicked()), this, SLOT(OnBnClickedLeDisconnect()));
    connect(ui->btnBLEDiscoverServices, SIGNAL(clicked()), this, SLOT(OnBnClickedDiscoverServices()));
    connect(ui->btnBLEDiscoverChars, SIGNAL(clicked()), this, SLOT(OnBnClickedDiscoverCharacteristics()));
    connect(ui->btnBLEDiscoverDescriptors, SIGNAL(clicked()), this, SLOT(OnBnClickedDiscoverDescriptors()));
    connect(ui->btnBLEStartStopAdvert, SIGNAL(clicked()), this, SLOT(OnBnClickedStartStopAdvertisements()));
    connect(ui->btnBLEValueNotify, SIGNAL(clicked()), this, SLOT(OnBnClickedSendNotification()));
    connect(ui->btnBLEValueIndicate, SIGNAL(clicked()), this, SLOT(OnBnClickedSendIndication()));
    connect(ui->btnBLERead, SIGNAL(clicked()), this, SLOT(OnBnClickedCharacteristicRead()));
    connect(ui->btnBLEWrite, SIGNAL(clicked()), this, SLOT(OnBnClickedCharacteristicWrite()));
    connect(ui->btnBLEWriteNoRsp, SIGNAL(clicked()), this, SLOT(OnBnClickedCharacteristicWriteWithoutResponse()));

    connect(this, SIGNAL(HandleLeAdvState(BYTE)), this, SLOT(processHandleLeAdvState(BYTE)));

}
	
void MainWindow::setGATTUI()
{
    ui->btnBLEStartDisc->setEnabled(!m_scan_active );
    ui->btnBLEStopDisc->setEnabled(m_scan_active );
}

void MainWindow::OnBnClickedLeConnect()
{
    BYTE command[20];
    int    commandBytes = 0;

    int item =  ui->cbBLEDeviceList->currentIndex();
    if (item < 0)
        return;

    CBtDevice *p_device = (CBtDevice *)ui->cbBLEDeviceList->itemData(item).value<CBtDevice *>();

    if (p_device == NULL)
        return;

    Log("LeConnect BtDevice : %02x:%02x:%02x:%02x:%02x:%02x",
        p_device->m_address[0], p_device->m_address[1], p_device->m_address[2], p_device->m_address[3],
            p_device->m_address[4], p_device->m_address[5]);
    // send command to connect
    // type and BDADDR
    command[commandBytes++] = p_device->address_type;

    for (int i = 0; i < 6; i++)
        command[commandBytes++] = p_device->m_address[5 - i];

    SendWicedCommand(HCI_CONTROL_LE_COMMAND_CONNECT, command, commandBytes);
}

void MainWindow::OnBnClickedLeCancelConnect()
{
    int item = ui->cbBLEDeviceList->currentIndex();
    if (item < 0)
        return;
    // send command to connect
    BYTE command[20] = {};
    int    commandBytes = 0;
    CBtDevice *p_device = (CBtDevice *)ui->cbBLEDeviceList->itemData(item).value<CBtDevice *>();

    if (p_device == NULL)
        return;

    // type and BDADDR
    command[commandBytes++] = p_device->address_type;

    for (int i = 0; i < 6; i++)
        command[commandBytes++] = p_device->m_address[5 - i];

   SendWicedCommand(HCI_CONTROL_LE_COMMAND_CANCEL_CONNECT, command, 7);
}

USHORT MainWindow::GetConHandle(QComboBox *pCombo)
{
    int sel;
    if ((sel = pCombo->currentIndex()) >= 0)
        return ((CBtDevice *)pCombo->itemData(sel).value<CBtDevice *>())->con_handle;
    return 0;
}

void MainWindow::OnBnClickedLeDisconnect()
{
    USHORT con_handle = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[3] = {};
    int    commandBytes = 0;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    SendWicedCommand(HCI_CONTROL_LE_COMMAND_DISCONNECT, command, commandBytes);
}

void MainWindow::OnBnClickedDiscoverServices()
{
    USHORT con_handle = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[8] = { 0 };
    int    commandBytes = 0;
    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = 1;         // start handle
    command[commandBytes++] = 0;
    command[commandBytes++] = 0xff;      // end handle
    command[commandBytes++] = 0xff;
    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_DISCOVER_SERVICES, command, commandBytes);
}

void MainWindow::OnBnClickedDiscoverCharacteristics()
{
    QString str1 = ui->edtBLEHandleStart->text();
    DWORD s_handle = GetHandle(str1);
    QString str2 = ui->edtBLEHandleEnd->text();
    DWORD e_handle = GetHandle(str2);

    USHORT con_handle   = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[20]  = { 0 };
    int    commandBytes = 0;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = s_handle & 0xff;       // start handle
    command[commandBytes++] = (s_handle >> 8) & 0xff;
    command[commandBytes++] = e_handle & 0xff;       // end handle
    command[commandBytes++] = (e_handle >> 8) & 0xff;

    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_DISCOVER_CHARACTERISTICS, command, commandBytes);
}

void MainWindow::OnBnClickedDiscoverDescriptors()
{
    QString str1 = ui->edtBLEHandleStart->text();
    DWORD s_handle = GetHandle(str1);
    QString str2 = ui->edtBLEHandleEnd->text();
    DWORD e_handle = GetHandle(str2);

    USHORT con_handle   = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[20]  = { 0 };
    int    commandBytes = 0;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = s_handle & 0xff;       // start handle
    command[commandBytes++] = (s_handle >> 8) & 0xff;
    command[commandBytes++] = e_handle & 0xff;       // end handle
    command[commandBytes++] = (e_handle >> 8) & 0xff;
    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_DISCOVER_DESCRIPTORS, command, commandBytes);
}

void MainWindow::OnBnClickedStartStopAdvertisements()
{
    BYTE command[2] = { 0 };
    int  commandBytes = 0;

    if (!m_advertisments_active)
    {
        command[commandBytes++] = 1;
        m_advertisments_active = TRUE;
        ui->btnBLEStartStopAdvert->setText("Stop Adverts");
    }
    else
    {
        command[commandBytes++] = 0;
        m_advertisments_active = FALSE;
        ui->btnBLEStartStopAdvert->setText("Start Adverts");
    }
    SendWicedCommand(HCI_CONTROL_LE_COMMAND_ADVERTISE, command, commandBytes);
}

void MainWindow::OnBnClickedSendNotification()
{
    USHORT con_handle   = GetConHandle(ui->cbBLEDeviceList);
    static BYTE  command[32]  = { 0 };
    int    commandBytes = 0;

    QString str = ui->edtBLEHandle->text();
    DWORD hdlc = GetHandle(str);
    static DWORD num_bytes;
    static unsigned char prev_send = 0;
    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = (BYTE)(hdlc & 0xff);
    command[commandBytes++] = (BYTE)((hdlc >> 8) & 0xff);
#ifdef REPEAT_NOTIFICATIONS_FOREVER
    if (sending_notifications)
    {
        command[commandBytes] = prev_send++;
    }
    else
#endif
    {
        QString str = ui->edtBLEHandleValue->text();
        num_bytes = GetHexValue(&command[commandBytes], sizeof(command) - commandBytes, str);
        prev_send = command[commandBytes] + 1;
    }
    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_NOTIFY, command, commandBytes + num_bytes);
#ifdef REPEAT_NOTIFICATIONS_FOREVER
    sending_notifications = TRUE;
#endif
}

void MainWindow::OnBnClickedSendIndication()
{
    USHORT con_handle   = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[32]  = { 0 };
    int    commandBytes = 0;

    QString str1 = ui->edtBLEHandle->text();
    DWORD hdlc = GetHandle(str1);
    DWORD num_bytes;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = (BYTE)(hdlc & 0xff);
    command[commandBytes++] = (BYTE)((hdlc >> 8) & 0xff);
    QString str2 = ui->edtBLEHandleValue->text();
    num_bytes = GetHexValue(&command[commandBytes], sizeof(command) - commandBytes, str2);

    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_INDICATE, command, commandBytes + num_bytes);
}

void MainWindow::OnBnClickedCharacteristicRead()
{
    USHORT con_handle   = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[32]  = { 0 };

    QString str = ui->edtBLEHandle->text();
    DWORD  hdlc         = GetHandle(str);
    int    commandBytes = 0;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = (BYTE)(hdlc & 0xff);
    command[commandBytes++] = (BYTE)((hdlc >> 8) & 0xff);

    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_READ_REQUEST, command, commandBytes);
}

void MainWindow::OnBnClickedCharacteristicWrite()
{
    USHORT con_handle = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[32] = { 0 };
    int    commandBytes = 0;

    QString str1 = ui->edtBLEHandle->text();
    DWORD hdlc = GetHandle(str1);
    DWORD num_bytes;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = (BYTE)(hdlc & 0xff);
    command[commandBytes++] = (BYTE)((hdlc >> 8) & 0xff);
    QString str2 = ui->edtBLEHandleValue->text();
    num_bytes = GetHexValue(&command[commandBytes], sizeof(command) - commandBytes, str2);

    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_WRITE_REQUEST, command, commandBytes + num_bytes);
}

void MainWindow::OnBnClickedCharacteristicWriteWithoutResponse()
{
    USHORT con_handle = GetConHandle(ui->cbBLEDeviceList);
    BYTE   command[32] = { 0 };
    int    commandBytes = 0;

    QString str1 = ui->edtBLEHandle->text();
    DWORD hdlc = GetHandle(str1);
    DWORD num_bytes;

    command[commandBytes++] = con_handle & 0xff;
    command[commandBytes++] = (con_handle >> 8) & 0xff;
    command[commandBytes++] = (BYTE)(hdlc & 0xff);
    command[commandBytes++] = (BYTE)((hdlc >> 8) & 0xff);
    QString str2 = ui->edtBLEHandleValue->text();
    num_bytes = GetHexValue(&command[commandBytes], sizeof(command) - commandBytes, str2);

    SendWicedCommand(HCI_CONTROL_GATT_COMMAND_WRITE_COMMAND, command, commandBytes++ + num_bytes);
}

void MainWindow::SetRole(CBtDevice *pDevice, uint8_t role)
{
    pDevice->role = role;
}

uint8_t MainWindow::GetRole(CBtDevice *pDevice)
{
    return pDevice->role;
}

void MainWindow::UpdateGattButtons(CBtDevice *pDevice)
{
    BOOL enable = 0;

    // Enable GATT Buttons for Master connections only
    if (GetRole(pDevice) == 0x00)
        enable = 1;

    ui->btnBLEDiscoverServices->setEnabled(enable);
    ui->btnBLEDiscoverChars->setEnabled(enable);
    ui->btnBLEDiscoverDescriptors->setEnabled(enable);
    ui->btnBLERead->setEnabled(enable);
    ui->btnBLEWrite->setEnabled(enable);
    ui->btnBLEWriteNoRsp->setEnabled(enable);
    ui->btnBLEValueNotify->setEnabled(enable);
    ui->btnBLEValueIndicate->setEnabled(enable);
}

void MainWindow::onHandleWicedEventGATT(unsigned int opcode, unsigned char *p_data, unsigned int len)
{
    switch (HCI_CONTROL_GROUP(opcode))
    {
    case HCI_CONTROL_GROUP_LE:
    case HCI_CONTROL_GROUP_TIME:
    case HCI_CONTROL_GROUP_ANCS:
    case HCI_CONTROL_GROUP_ALERT:
        HandleLEEvents(opcode, p_data, len);
        break;
    case HCI_CONTROL_GROUP_GATT:
        HandleGattEvents(opcode, p_data, len);
        break;
    }
}

void MainWindow::processHandleLeAdvState(BYTE val)
{
    ui->lblBLEAdvertisementState->setText(szAdvState[val]);

    if (val)
    {
        m_advertisments_active = TRUE;
        ui->btnBLEStartStopAdvert->setText("Stop Adverts");
    }
    else
    {
        m_advertisments_active = FALSE;
        ui->btnBLEStartStopAdvert->setText("Start Adverts");
    }

    Log("Advertisement:%d", val);
}

void MainWindow::HandleLEEvents(DWORD identifier, LPBYTE p_data, DWORD len)
{
    CHAR   trace[1024];
    BYTE    bda[6];
    CBtDevice *device;

    switch (identifier)
    {
    case HCI_CONTROL_LE_EVENT_SCAN_STATUS:

        ui->lblBLEScanState->setText(szScanState[p_data[0]]);        
        sprintf (trace, "Scan:%d", p_data[0]);
        Log(trace);
        break;

    case HCI_CONTROL_LE_EVENT_ADVERTISEMENT_REPORT:
    {
        char bd_name[50] = {0};



        for (int i = 0; i < 6; i++)
            bda[5 - i] = p_data[2 + i];
        // check if this device is not present yet.
        if (FindInList(bda, ui->cbBLEDeviceList) != NULL)
            break;

        sprintf (trace, "Advertisement report:%d type:%d address %02x:%02x:%02x:%02x:%02x:%02x rssi:%d",
            p_data[0], p_data[1], p_data[7], p_data[6], p_data[5], p_data[4], p_data[3], p_data[2],
            p_data[8] - 256);
        Log(trace);

        // dump advertisement data to the trace
        if (len > 10)
        {
#if 0
            trace[0] = 0;
            for (int i = 0; i < (int)len - 8; i++)
                sprintf(&trace[strlen(trace)], "%02x", p_data[9 + i]);
            Log(trace);
#endif

            DecodeEIR(&p_data[9], len - 9, bd_name, sizeof(bd_name));

        }

        device = AddDeviceToList(bda, ui->cbBLEDeviceList, bd_name); // /* p_data[0], */ bda, p_data[7] + (p_data[8] << 8), CONNECTION_TYPE_LE);
        device->m_conn_type = CONNECTION_TYPE_LE;
        device->address_type = p_data[1];
        device->m_bIsLEDevice = true;
    }
    break;
    case HCI_CONTROL_LE_EVENT_ADVERTISEMENT_STATE:

        emit HandleLeAdvState(p_data[0]);

        break;
    case HCI_CONTROL_LE_EVENT_CONNECTED:
        sprintf (trace, "Connection up:type:%d address %02x:%02x:%02x:%02x:%02x:%02x connection handle:%04x role:%d",
            p_data[0], p_data[6], p_data[5], p_data[4], p_data[3], p_data[2], p_data[1], p_data[7] + (p_data[8] << 8), p_data[9]);
        Log(trace);

        // Check if the device is connected in Peripheral (Slave) role.
        if (p_data[9])
        {
            // If the device is connected in a peripheral role adv. would be stopped automatically.
            // Hence the button's state needs to be changed as well.
            m_advertisments_active = FALSE;
            ui->btnBLEStartStopAdvert->setText("Start Adverts");
        }

        for (int i = 0; i < 6; i++)
            bda[5 - i] = p_data[1 + i];
        // find device in the list with received address and save the connection handle
        if ((device = FindInList(bda, ui->cbBLEDeviceList)) == NULL)
            device = AddDeviceToList(bda, ui->cbBLEDeviceList, NULL); // /* p_data[0], */ bda, p_data[7] + (p_data[8] << 8), CONNECTION_TYPE_LE);

        device->con_handle = p_data[7] + (p_data[8] << 8);
        device->m_conn_type = CONNECTION_TYPE_LE;
        device->m_bIsLEDevice = true;

        SetRole(device, p_data[9]);     // Save LE role
        SelectDevice(ui->cbBLEDeviceList, bda);
        UpdateGattButtons(device);
        break;



    case HCI_CONTROL_LE_EVENT_DISCONNECTED:
        sprintf (trace, "Connection down:connection handle:%04x reason:0x%x",
            p_data[0] + (p_data[1] << 8), p_data[2]);
        Log(trace);
        ui->lblCTMessage->setText("");
        ui->lblCTTitle->setText("");
        ui->btnCTANCSPositive->setText("");
        ui->btnCTANCSNegative->setText("");

        break;

    case HCI_CONTROL_ANCS_EVENT_NOTIFICATION:
        m_notification_uid = p_data[0] + (p_data[1] << 8) + (p_data[2] << 16), (p_data[3] << 24);
        sprintf (trace, "(ANCS) %04d Command:%d Category:%d Flags:%d", m_notification_uid, p_data[4], p_data[5], p_data[6]);
        Log(trace);

        // notification Added or Modified
        if ((p_data[4] == 0) || (p_data[4] == 1))
        {
            int len = 7;
            ui->lblCTMessage->setText((char*)&p_data[len]);

            len += (int)strlen((char *)&p_data[len]) + 1;
            ui->lblCTTitle->setText((char*)&p_data[len]);

            len += (int)strlen((char *)&p_data[len]) + 1;
            ui->btnCTANCSPositive->setText((char*)&p_data[len]);

            len += (int)strlen((char *)&p_data[len]) + 1;
            ui->btnCTANCSNegative->setText((char*)&p_data[len]);
        }
        else // removed
        {
            ui->lblCTMessage->setText("Message");
            ui->lblCTTitle->setText("Tile");
            ui->btnCTANCSPositive->setText("ANCS Positive");
            ui->btnCTANCSNegative->setText("ANCS Negative");
        }

        break;

    case HCI_CONTROL_EVENT_COMMAND_STATUS:
        sprintf (trace, "Status: %s", (p_data[0] <= HCI_CONTROL_STATUS_INVALID_ARGS) ? szStatusCode[p_data[0]] : "????");
        Log(trace);
        break;
    }
}

void MainWindow::HandleGattEvents(DWORD opcode, LPBYTE p_data, DWORD len)
{
    CHAR   trace[1024];

    switch (opcode)
    {
    case HCI_CONTROL_GATT_EVENT_COMMAND_STATUS:
        sprintf (trace, "GATT command status:%x", p_data[0]);
        Log(trace);
        break;
    case HCI_CONTROL_GATT_EVENT_DISCOVERY_COMPLETE:
        sprintf (trace, "Discovery Complete connection handle:%04x", p_data[0] + (p_data[1] << 8));
        Log(trace);
        break;
    case HCI_CONTROL_GATT_EVENT_SERVICE_DISCOVERED:
        if (len == 8)
        {
            sprintf (trace, "Service:UUID %04x start handle:%04x end handle %04x",
                p_data[2] + (p_data[3] << 8), p_data[4] + (p_data[5] << 8), p_data[6] + (p_data[7] << 8));
        }
        else
        {
            sprintf (trace, "Service:UUID %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x start handle:%04x end handle %04x",
                p_data[2],  p_data[3],  p_data[4],  p_data[5],  p_data[6],  p_data[7],  p_data[8],  p_data[9],
                p_data[10], p_data[11], p_data[12], p_data[13], p_data[14], p_data[15], p_data[16], p_data[17],
                p_data[18] + (p_data[19] << 8), p_data[20] + (p_data[21] << 8));
        }
        Log(trace);
        break;
    case HCI_CONTROL_GATT_EVENT_CHARACTERISTIC_DISCOVERED:
        if (len == 9)
        {
            sprintf (trace, "Characteristic:char handle:%04x UUID %04x Properties:0x%02x value handle:%04x",
                p_data[2] + (p_data[3] << 8), p_data[4] + (p_data[5] << 8), p_data[6], p_data[7] + (p_data[8] << 8));
        }
        else
        {
            sprintf (trace, "Characteristic:char handle:%04x UUID %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Properties:0x%02x value handle:%04x",
                p_data[2] + (p_data[3] << 8),
                p_data[4],  p_data[5],  p_data[6],  p_data[7],  p_data[8],  p_data[9],  p_data[10], p_data[11],
                p_data[12], p_data[13], p_data[14], p_data[15], p_data[16], p_data[17], p_data[18], p_data[19],
                p_data[20], p_data[21] + (p_data[22] << 8));
        }
        Log(trace);
        break;
    case HCI_CONTROL_GATT_EVENT_DESCRIPTOR_DISCOVERED:
        if (len == 6)
        {
            sprintf (trace, "Descriptor:UUID %04x handle:%04x",
                p_data[2] + (p_data[3] << 8), p_data[4] + (p_data[5] << 8));
        }
        else
        {
            sprintf (trace, "Descriptor:UUID %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x handle:%04x",
                p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7], p_data[8], p_data[9],
                p_data[10], p_data[11], p_data[12], p_data[13], p_data[14], p_data[15], p_data[16], p_data[17],
                p_data[18] + (p_data[19] << 8));
        }
        Log(trace);
        break;
    case HCI_CONTROL_GATT_EVENT_READ_RESPONSE:
        sprintf (trace, "Read Response [%02x] ", p_data[0] + (p_data[1] << 8));
        for (int i = 0; i < (int)len - 2; i++)
            sprintf(&trace[strlen(trace)], "%02x", p_data[2 + i]);
        Log(trace);
        ui->edtBLEHandleValue->setText(trace);

        break;
    case HCI_CONTROL_GATT_EVENT_READ_REQUEST:
        sprintf (trace, "Read Req:Conn:%04x handle:%04x", p_data[0] + (p_data[1] << 8), p_data[2] + (p_data[3] << 8));
        Log(trace);
        // send 1 byte response with value 100;
        p_data[4] = 100;
        SendWicedCommand(HCI_CONTROL_GATT_COMMAND_READ_RESPONSE, p_data, 5);
        break;
    case HCI_CONTROL_GATT_EVENT_WRITE_REQUEST:
        sprintf (trace, "Write Req:Conn:%04x handle:%04x", p_data[0] + (p_data[1] << 8), p_data[2] + (p_data[3] << 8));
        // send 1 byte result code 0
        p_data[4] = 0;
        SendWicedCommand(HCI_CONTROL_GATT_COMMAND_WRITE_RESPONSE, p_data, 5);
        break;
    case HCI_CONTROL_GATT_EVENT_WRITE_RESPONSE:
        if (len == 2)
            sprintf (trace, "Write Rsp [%x] ", p_data[0] + (p_data[1] << 8));
        else
            sprintf (trace, "Write Rsp [%x] result:0x%x", p_data[0] + (p_data[1] << 8), p_data[2]);
        Log(trace);
#ifdef REPEAT_NOTIFICATIONS_FOREVER
        if (sending_notifications)
        {
            OnBnClickedSendNotification();
        }
#endif
        break;
    case HCI_CONTROL_GATT_EVENT_NOTIFICATION:
        sprintf (trace, "Notification [%02x] handle:%04x ", p_data[0] + (p_data[1] << 8), p_data[2] + (p_data[3] << 8));
        for (int i = 0; i < (int)len - 4; i++)
            sprintf(&trace[strlen(trace)], "%02x", p_data[4 + i]);
        Log(trace);
        break;
    case HCI_CONTROL_GATT_EVENT_INDICATION:
        sprintf (trace, "Indication [%02x] handle:%04x ", p_data[0] + (p_data[1] << 8), p_data[2] + (p_data[3] << 8));
        for (int i = 0; i < (int)len - 4; i++)
            sprintf(&trace[strlen(trace)], "%02x", p_data[4 + i]);
        Log(trace);
        break;

    }
}

