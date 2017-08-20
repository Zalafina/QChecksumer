#include "checksumermain.h"
#include "ui_checksumermain.h"

ChecksumerMain::ChecksumerMain(Checksumer *checksumer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChecksumerMain)
{
    ui->setupUi(this);
    m_Checksumer = checksumer;

    QObject::connect(m_Checksumer, SIGNAL(Checksumer_RangeChangedSignal(int,int)), this, SLOT(processbar_SetRange(int,int)), Qt::AutoConnection);
    QObject::connect(m_Checksumer, SIGNAL(Checksumer_ValueChangedSignal(int)), this, SLOT(processbar_SetValue(int)), Qt::AutoConnection);
    QObject::connect(m_Checksumer, SIGNAL(Checksumer_ChecksumResultSignal(quint64)), this, SLOT(setChecksumResult(quint64)), Qt::AutoConnection);
}

ChecksumerMain::~ChecksumerMain()
{
    delete ui;
}

void ChecksumerMain::on_openfileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open BIN File"),
                                                    NULL,
                                                    tr("BIN Files (*.bin *.img)"));

    if (fileName.length() != 0){
        emit m_Checksumer->OpenFileButtonClicked(fileName);

        QFileInfo fileInfo(fileName);
        QString dispFilename = fileInfo.fileName();
        ui->filenameDisplay->setText(dispFilename);

        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->checksumDisplay->clear();
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
        emit m_Checksumer->ChecksumButtonClicked();
    }
    else{
        QMessageBox::warning(this, tr("Checksumer"), tr("Select BIN file first!"));
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
}

void ChecksumerMain::processbar_SetValue(int progressValue)
{
    ui->progressBar->setValue(progressValue);
}

void ChecksumerMain::setChecksumResult(quint64 checksum)
{
    QString checksum_str;
    checksum_str.setNum(checksum,16);
    checksum_str = checksum_str.toUpper();
    ui->checksumDisplay->setText(checksum_str);
}
