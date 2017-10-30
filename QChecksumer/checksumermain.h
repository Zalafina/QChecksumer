#ifndef CHECKSUMERMAIN_H
#define CHECKSUMERMAIN_H

#include <QDialog>
#include <QMessageBox>
#include <QStatusBar>
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

    Q_INVOKABLE void ActivationChangedProc(void);
protected:
    void changeEvent(QEvent *event);

private slots:
    void on_openfileButton_clicked();

    void on_checksumButton_clicked();

public slots:
    void processbar_SetRange(int minimum, int maximum);
    void processbar_SetValue(int progressValue);
    void setChecksumResult(quint64 checksum, qint64 elapsedtime);
    void processbar_ValueChanged(int value);
    void elapsedTimeUpdate(void);

private:
    void setElapsedTimetoLCDNumber(qint64 elapsedtime);

    Ui::ChecksumerMain  *ui;
    Checksumer          *m_Checksumer;
    QTimer              m_updatetimer;
};

#endif // CHECKSUMERMAIN_H
