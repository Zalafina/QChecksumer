#ifndef CHECKSUMERMAIN_H
#define CHECKSUMERMAIN_H

#include <QDialog>
#include <QMessageBox>
#include "checksumer.h"

namespace Ui {
class ChecksumerMain;
}

class ChecksumerMain : public QDialog
{
    Q_OBJECT

public:
    explicit ChecksumerMain(Checksumer * checksumer, QWidget *parent = 0);
    ~ChecksumerMain();

private slots:
    void on_openfileButton_clicked();

    void on_checksumButton_clicked();

public slots:
    void processbar_SetRange(int minimum, int maximum);
    void processbar_SetValue(int progressValue);
    void setChecksumResult(quint64 checksum);

private:
    Ui::ChecksumerMain *ui;
    Checksumer * m_Checksumer;
};

#endif // CHECKSUMERMAIN_H
