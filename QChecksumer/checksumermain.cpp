#ifdef BRINGTO_FOREGROUND_ATFINISHED
#include "windef.h"
#include "winuser.h"
#include "process.h"
#endif
#include "checksumermain.h"
#include "ui_checksumermain.h"

static const quint64 TIME_UPDATE_TIMEOUT = 100;

#ifdef BRINGTO_FOREGROUND_ATFINISHED
static BOOL CALLBACK focusChildProcWindow(HWND hwnd, LPARAM lParam)
{
    DWORD processId = static_cast<WORD>(lParam);
    DWORD windowProcessId = NULL;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    if(windowProcessId == processId)
    {
        SwitchToThisWindow(hwnd, TRUE);
        SetForegroundWindow(hwnd);
        return FALSE;
    }

    return TRUE;
}
#endif

ChecksumerMain::ChecksumerMain(Checksumer *checksumer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChecksumerMain),
    m_updatetimer(this)
{
    ui->setupUi(this);
    m_Checksumer = checksumer;
    m_updatetimer.setTimerType(Qt::PreciseTimer);

    QObject::connect(m_Checksumer, SIGNAL(Checksumer_RangeChangedSignal(int,int)), this, SLOT(processbar_SetRange(int,int)), Qt::AutoConnection);
    QObject::connect(m_Checksumer, SIGNAL(Checksumer_ValueChangedSignal(int)), this, SLOT(processbar_SetValue(int)), Qt::AutoConnection);
    QObject::connect(m_Checksumer, SIGNAL(Checksumer_ChecksumResultSignal(quint64, qint64)), this, SLOT(setChecksumResult(quint64, qint64)), Qt::AutoConnection);
    //QObject::connect(m_Checksumer, SIGNAL(Checksumer_ElapsedTimeSignal(qint64)), this, SLOT(elapsedTimeUpdate(qint64)), Qt::AutoConnection);
    QObject::connect(&m_updatetimer, SIGNAL(timeout()), this, SLOT(elapsedTimeUpdate()), Qt::AutoConnection);
    QObject::connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(processbar_ValueChanged(int)), Qt::AutoConnection);
}

ChecksumerMain::~ChecksumerMain()
{
    delete ui;
}

void ChecksumerMain::on_openfileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open BIN File",
                                                    NULL,
                                                    "BIN Files (*.bin *.img)");

    if (fileName.length() != 0){
        emit m_Checksumer->OpenFileButtonClicked(fileName);

        QFileInfo fileInfo(fileName);
        QString dispFilename = fileInfo.fileName();
        ui->filenameDisplay->setText(dispFilename);
        ui->filenameDisplay->setToolTip(fileName);

        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->checksumDisplay->clear();
        this->setWindowTitle(QString("QChecksumer"));
    }
    else{
    }
}

void ChecksumerMain::on_checksumButton_clicked()
{
    if (false == Checksumer::m_filepath.isEmpty()){
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->checksumDisplay->clear();
        this->setWindowTitle(QString("QChecksumer"));
        m_updatetimer.start(TIME_UPDATE_TIMEOUT);
        emit m_Checksumer->ChecksumButtonClicked();
    }
    else{
        QMessageBox::warning(this, "Checksumer", "Select BIN file first!");
    }
}

void ChecksumerMain::processbar_SetRange(int minimum, int maximum)
{
#ifdef DEBUG_LOGOUT_ON
    qDebug("processbar_SetRange:minimum(%d), maximum(%d)", minimum, maximum);
#endif
    ui->progressBar->setRange(minimum, maximum);
    ui->progressBar->setValue(0);
    ui->checksumDisplay->clear();
    this->setWindowTitle(QString("QChecksumer"));
}

void ChecksumerMain::processbar_SetValue(int progressValue)
{
    ui->progressBar->setValue(progressValue);
}

void ChecksumerMain::setChecksumResult(quint64 checksum, qint64 elapsedtime)
{
    QString checksum_str;
    checksum_str.setNum(checksum,16);
    checksum_str = checksum_str.toUpper();
    ui->checksumDisplay->setText(checksum_str);
    m_updatetimer.stop();
    setElapsedTimetoLCDNumber(elapsedtime);

#ifdef BRINGTO_FOREGROUND_ATFINISHED
//    Qt::WindowFlags flags = this->windowFlags();
//    flags |= Qt::WindowStaysOnTopHint;
//    this->setWindowFlags(flags);
//    showNormal();
//    activateWindow();
//    raise();

//    flags &= ~Qt::WindowStaysOnTopHint;
//    this->setWindowFlags(flags);
//    showNormal();
//    activateWindow();
//    raise();

    int pid = getpid();
    EnumWindows(focusChildProcWindow, static_cast<LPARAM>(pid));
#else
    showNormal();
    activateWindow();
    raise();
#endif

    ui->checksumDisplay->selectAll();
    qreal displayvalue = ui->elapsedtimeLCDNumber->value();
    QString realString = QString::number(displayvalue, 'f', 1);
    realString.prepend(QChar('('));
    realString.append(" Sec)");
    this->setWindowTitle(QString("Complete") + realString + QString(" - QChecksumer"));
}

void ChecksumerMain::processbar_ValueChanged(int value)
{
    Q_UNUSED(value);
#ifdef DEBUG_LOGOUT_ON
    qDebug("processbar_ValueChanged:text(%s)", ui->progressBar->text().toLatin1().constData());
#endif

    if (ui->progressBar->text() != QString("100%")){
        QString titlestring = ui->progressBar->text() + " " + ui->filenameDisplay->text() + " - QChecksumer";
        this->setWindowTitle(titlestring);
    }
    else{
        //this->setWindowTitle(QString("QChecksumer"));
    }
}

void ChecksumerMain::elapsedTimeUpdate(void)
{
    qint64 ElapsedTime = m_Checksumer->getElapsedTime();

    if (ElapsedTime != -1){
        setElapsedTimetoLCDNumber(ElapsedTime);
    }
}

void ChecksumerMain::setElapsedTimetoLCDNumber(qint64 elapsedtime)
{
    qreal realelapsedtime = (qreal)(elapsedtime)/1000;
    QString realString = QString::number(realelapsedtime, 'f', 1);
    ui->elapsedtimeLCDNumber->display(realString);
#ifdef DEBUG_LOGOUT_ON
    qDebug("setElapsedTimetoLCDNumber(%lld)", elapsedtime);
    qDebug("realelapsedtime (%s) sec", realString.toLatin1().constData());
#endif
}
