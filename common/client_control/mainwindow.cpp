#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include "hci_control_api.h"
#include <QDateTime>
#include <QFileDialog>

MainWindow *g_pMainWindow = NULL ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_paired_icon(":/paired.png"),
    m_settings("clientcontrol.ini",QSettings::IniFormat),
    m_fp_logfile(NULL),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(this->size());
    qApp->setStyleSheet("QGroupBox {  border: 1px solid gray;}");

    g_pMainWindow = this;

    connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(btnClearClicked()));
    connect(ui->btnFindLogFile, SIGNAL(clicked()), this, SLOT(btnFindLogfileClicked()));
    connect(ui->btnLogToFile, SIGNAL(clicked(bool)), this, SLOT(btnLogToFileClicked(bool)));
    connect(ui->btnAddTrace, SIGNAL(clicked()), this, SLOT(btnAddTraceClicked()));

    connect(this, SIGNAL(HandleWicedEvent(unsigned int,unsigned int,unsigned char*)), this, SLOT(onHandleWicedEvent(unsigned int,unsigned int ,unsigned char*)), Qt::QueuedConnection);
    connect(this, SIGNAL(HandleTrace(QString*)), this, SLOT(processTrace(QString*)), Qt::QueuedConnection);


    m_iContext = 0;

    qApp->setStyleSheet("QGroupBox {border: 1px solid gray; border-radius: 9px; margin-top: 0.5em;}");
    qApp->setStyleSheet("QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px;}");

    InitDm();
    InitAudioSrc();
    InitAVRCTG();
    InitAudioSnk();
    InitAVRCCT();
    InitAG();
    InitHF();
    InitSPP();
    InitHIDH();
    InitBLEHIDD();    
    InitGATT();
    InitHK();
    InitiAP2();    
    InitBSG();

    onClear();
    Log("Instructions:");
    Log("1.  Plug the WICED Studio Evaluation Board into the computer using a USB cable.");
    Log("2.  In the Eclipse IDE, double-click on the desired target in 'Make Target' window to build");
    Log("    and download an embedded application to the WICED Studio evaluation board.");
    Log("3.  Select the serial (COM) port for the WICED Studio Evaluation Board and open the port.");
    Log("    The UI will be enabled when the Client Control app is able to communicate with the embedded BT app.");
    Log(" ");
    Log("Note: To re-download the embedded application, first close the serial port.");
    Log(" ");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    closeEventDm(event);
    qDebug("MainWindow::closeEvent exiting");
    event->accept();
}

void MainWindow::onHandleWicedEvent(unsigned int opcode, unsigned int len, unsigned char *p_data)
{

    // send event to modules, Dm should be first
    onHandleWicedEventDm(opcode,p_data,len);
    onHandleWicedEventAudioSrc(opcode, p_data, len);
    onHandleWicedEventHF(opcode, p_data, len);
    onHandleWicedEventSPP(opcode, p_data, len);
    onHandleWicedEventAG(opcode, p_data, len);
    onHandleWicedEventBLEHIDD(opcode, p_data, len);
    onHandleWicedEventHIDH(opcode, p_data, len);
    onHandleWicedEventAVRCCT(opcode, p_data, len);
    onHandleWicedEventAVRCTG(opcode, p_data, len);
    onHandleWicedEventHK(opcode, p_data, len);
    onHandleWicedEventiAP2(opcode, p_data, len);
    onHandleWicedEventGATT(opcode, p_data, len);
    onHandleWicedEventAudioSnk(opcode, p_data, len);
    onHandleWicedEventBSG(opcode, p_data, len);

    // free event data, allocated in Dm module when event arrives
    if (p_data)
        free(p_data);
}

void MainWindow::onClear()
{
    ui->lstTrace->clear();
}

void MainWindow::processTrace(QString * trace)
{
    ui->lstTrace->addItem(*trace);
    ui->lstTrace->scrollToBottom();
    ui->lstTrace->scrollToItem(ui->lstTrace->item( ui->lstTrace->count()));

    if (ui->btnLogToFile->isChecked() && m_fp_logfile)
    {
        fprintf(m_fp_logfile, "%s\n", trace->toStdString().c_str());
        fflush(m_fp_logfile);
    }
    delete trace;
}

void MainWindow::Log(const char * fmt, ...)
{
    va_list cur_arg;
    va_start(cur_arg, fmt);
    char trace[1000];
    memset(trace, 0, sizeof(trace));
    vsprintf(trace, fmt, cur_arg);

    QString s = QDateTime::currentDateTime().toString("MM-dd-yyyy hh:mm:ss.zzz: ") + trace;
    va_end(cur_arg);

    // add trace to file and UI screen in UI thread
    emit HandleTrace(new QString(s));
}

void MainWindow::btnClearClicked()
{
    ui->lstTrace->clear();
}

void MainWindow::btnFindLogfileClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Log File"), "", tr("All Files (*.*)"));
    ui->edLogFile->setText(fileName);
}

void MainWindow::btnLogToFileClicked(bool checked)
{
    if (checked)
    {
        if (NULL == (m_fp_logfile = fopen(ui->edLogFile->text().toStdString().c_str(), "w")))
        {
            Log("Error opening logfile: %d", 0/*errno*/);
            ui->btnLogToFile->setChecked(false);
        }
    }
    else
    {
        // stop logging to file
        if (m_fp_logfile)
        {
            fclose(m_fp_logfile);
            m_fp_logfile = NULL;
        }
    }
}

void MainWindow::btnAddTraceClicked()
{
    Log(ui->edTrace->text().toStdString().c_str());
}


#ifndef WIN32
void TraceHciPkt(BYTE type, BYTE *buffer, USHORT length)
{

}
#endif

USHORT MainWindow::GetHandle(QString &str)
{
    BYTE buf[2];
    int num_digits = GetHexValue(buf, 2, str);
    if (num_digits == 2)
        return (buf[0] << 8) + buf[1];
    else
        return buf[0];
}

DWORD MainWindow::GetHexValue(LPBYTE buf, DWORD buf_size, QString &str)
{
    char szbuf[100];
    char *psz = szbuf;
    BYTE *pbuf = buf;
    DWORD res = 0;

    memset(buf, 0, buf_size);

    strncpy(szbuf, str.toStdString().c_str(), 100);

    if (strlen(szbuf) == 1)
    {
        szbuf[2] = 0;
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    else if (strlen(szbuf) == 3)
    {
        szbuf[4] = 0;
        szbuf[3] = szbuf[2];
        szbuf[2] = szbuf[1];
        szbuf[1] = szbuf[0];
        szbuf[0] = '0';
    }
    for (DWORD i = 0; i < strlen(szbuf); i++)
    {
        if (isxdigit(psz[i]) && isxdigit(psz[i + 1]))
        {
            *pbuf++ = (ProcNibble(psz[i]) << 4) + ProcNibble(psz[i + 1]);
            res++;
            i++;
        }
    }
    return res;
}

BYTE MainWindow::ProcNibble (char n)
{
    if ((n >= '0') && (n <= '9'))
    {
        n -= '0';
    }
    else if ((n >= 'A') && (n <= 'F'))
    {
        n = ((n - 'A') + 10);
    }
    else if ((n >= 'a') && (n <= 'f'))
    {
        n = ((n - 'a') + 10);
    }
    else
    {
        n = (char)0xff;
    }
    return (n);
}


