#ifndef CHECKSUMER_H
#define CHECKSUMER_H

#include <QObject>
#ifdef DEBUG_LOGOUT_ON
#include <QDebug>
#endif
#include <QThread>
#include <QFileDialog>
#include <QFile>
#include <QMutex>
#include <QProgressDialog>
#include <QTimer>

class Checksumer;

typedef struct {
    int index;
    quint64 offset;
    quint64 length;
    quint64 checksum;
    Checksumer *checksumer_ptr;
} Split_st;

class Checksumer : public QObject
{
    Q_OBJECT
public:
    explicit Checksumer(QObject *parent = 0);

    enum Checksumer_Status
    {
        CHECKSUMER_IDLE,
        CHECKSUMER_FILEOPENED,
        CHECKSUMER_CHECKSUMMING,
        CHECKSUMER_COMPLETE
    };
    Q_ENUM(Checksumer_Status)

signals:
    void OpenFileButtonClicked(QString filepath);
    void ChecksumButtonClicked(void);

    void Checksumer_RangeChangedSignal(int minimum, int maximum);
    void Checksumer_ValueChangedSignal(int progressValue);
    void Checksumer_ChecksumResultSignal(quint64 checksum);

public slots:
    void threadStarted(void);
    void OpenFileProcesser(QString filepath);
    void ChecksumProcesser(void);

public:
    static Split_st splitChecksum(const Split_st &split);
    static void reduceResult(quint64 &checksum, const Split_st &split);

public:
    static QString  m_filepath;
    static QList<Split_st> m_splitlist;
    static quint8 m_status;
};

#endif // CHECKSUMER_H
