#include <QtWidgets>
#include <QtConcurrent>
#include "checksumer.h"

QString Checksumer::m_filepath = QString();
QList<Split_st> Checksumer::m_splitlist = QList<Split_st>();
quint8 Checksumer::m_status = Checksumer::CHECKSUMER_IDLE;

static const quint64 EVERY_SPLIT_BYTESIZE = (1024 * 1024 * 30);
//static const quint64 EVERY_SPLIT_SIZE_LARGE = (1024 * 1024 * 10);

#define split_roundup(x,n) ((x+n-1)/n)

Checksumer::Checksumer(QObject *parent) :
    QObject(parent),
    m_elapsedtime()
{
    QObject::connect(this, &Checksumer::OpenFileButtonClicked, this, &Checksumer::OpenFileProcesser);
    QObject::connect(this, &Checksumer::ChecksumButtonClicked, this, &Checksumer::ChecksumProcesser);
}

void Checksumer::threadStarted()
{
    //qDebug("Checksumer::threadStarted() Called");
    //qDebug("Name:%s, ID:0x%08X", QThread::currentThread()->objectName().toLatin1().constData(), QThread::currentThreadId());
}

void Checksumer::OpenFileProcesser(QString filepath)
{
#ifdef DEBUG_LOGOUT_ON
    qDebug("file path : %s", filepath.toLatin1().constData());
#endif

    QFileInfo fileInfo(filepath);

    m_filepath = QString();

    if(fileInfo.isReadable()) {
        if (fileInfo.size() > 0){
#ifdef DEBUG_LOGOUT_ON
            qDebug("OpenFileProcesser::file size : %lld", fileInfo.size());
#endif
            m_filepath = filepath;
            m_status = CHECKSUMER_FILEOPENED;
            m_splitlist.clear();
        }
        else{
            qDebug("OpenFileProcesser::file size error : %lld", fileInfo.size());
        }
    }
    else{
        qDebug("OpenFileProcesser::File is unreadable!!!");
    }
}

void Checksumer::ChecksumProcesser()
{
    if (false == m_filepath.isEmpty()){
        QFileInfo fileInfo(m_filepath);

        if(fileInfo.isReadable()) {
            if (fileInfo.size() > 0){
            }
            else{
                qWarning("ChecksumProcesser::file size error : %lld", fileInfo.size());
                return;
            }
        }
        else{
            qWarning("ChecksumProcesser::File is unreadable!!!");
            return;
        }

        // Start Checksumming
        qint64 ChecksummingTime = 0;
        //QTime time;
        m_elapsedtime.restart();

        m_status = CHECKSUMER_CHECKSUMMING;

        // Prepare the QList
        m_splitlist.clear();

        qint64 filesize = fileInfo.size();

        int splitcount = split_roundup(filesize, EVERY_SPLIT_BYTESIZE);

        if (1 == splitcount){
#ifdef DEBUG_LOGOUT_ON
            // Get System ideal ThreadProcess Number
            int threadNumber = QThread::idealThreadCount();
            qDebug("ChecksumProcesser::file size : %lld, thread number : %d", filesize, threadNumber);
            qDebug("splitcount:%d", splitcount);
#endif
            emit Checksumer_RangeChangedSignal(0, 1);

            Split_st tempSplit;
            tempSplit.index = 0;
            tempSplit.length = filesize;
            tempSplit.checksum = 0;
            tempSplit.offset = 0;
            tempSplit.checksumer_ptr = this;

            Split_st result = splitChecksum(tempSplit);

            emit Checksumer_ValueChangedSignal(1);
            m_status = CHECKSUMER_COMPLETE;
            ChecksummingTime = m_elapsedtime.elapsed();
            emit Checksumer_ChecksumResultSignal(result.checksum, ChecksummingTime);
#ifdef DEBUG_LOGOUT_ON
            qreal elapsedtime = (qreal)(ChecksummingTime)/1000;
            qDebug("Checksum Result(0x%llX), TotalTime (%.2f) sec", result.checksum, elapsedtime);
#endif
        }
        else{
#ifdef DEBUG_LOGOUT_ON
            // Get System ideal ThreadProcess Number
            int threadNumber = QThread::idealThreadCount();
            qDebug("ChecksumProcesser::file size : %lld, thread number : %d", filesize, threadNumber);
#endif

            Split_st tempSplit;
            tempSplit.index = 0;
            tempSplit.length = 0;
            tempSplit.checksum = 0;
            tempSplit.offset = 0;
            tempSplit.checksumer_ptr = this;

#ifdef DEBUG_LOGOUT_ON
            qDebug("splitcount:%d", splitcount);
#endif

            for (int loop = 0; loop < splitcount - 1; loop++){
                tempSplit.index = loop;
                tempSplit.offset = (qint64)(EVERY_SPLIT_BYTESIZE * loop);
                tempSplit.length = EVERY_SPLIT_BYTESIZE;
                m_splitlist.append(tempSplit);
            }

            /* last split */
            tempSplit.index = splitcount - 1;
            tempSplit.offset = (qint64)(EVERY_SPLIT_BYTESIZE * (splitcount - 1));

            qint64 lastsplitsize = filesize%EVERY_SPLIT_BYTESIZE;
            if(0 == lastsplitsize){
                tempSplit.length = EVERY_SPLIT_BYTESIZE;
            }
            else{
                tempSplit.length = lastsplitsize;
            }
            m_splitlist.append(tempSplit);

            emit Checksumer_RangeChangedSignal(0, (splitcount - 1));

    //        for (int loop = 0; loop < m_splitlist.size(); loop++){
    //            qDebug("splitlist(%d):offset(%lld), length(%lld)", loop, m_splitlist[loop].offset, m_splitlist[loop].length);
    //        }

            quint64 final_checksum = \
                    QtConcurrent::mappedReduced(m_splitlist,
                                                Checksumer::splitChecksum,
                                                Checksumer::reduceResult,
                                                QtConcurrent::ReduceOptions(QtConcurrent::OrderedReduce
                                                                            | QtConcurrent::SequentialReduce));

            m_status = CHECKSUMER_COMPLETE;
            ChecksummingTime = m_elapsedtime.elapsed();
            emit Checksumer_ChecksumResultSignal(final_checksum, ChecksummingTime);
#ifdef DEBUG_LOGOUT_ON
            qreal elapsedtime = (qreal)(ChecksummingTime)/1000;
            qDebug("Checksum Result(0x%llX), TotalTime (%.2f) sec", final_checksum, elapsedtime);
#endif
        }

        m_elapsedtime.invalidate();
    }
    else{
        qWarning("ChecksumProcesser::File path is empty!!!");
        return;
    }
}

Split_st Checksumer::splitChecksum(const Split_st &split)
{
    //qDebug("split.index(%d), split.offset(%lld), split.length(%lld), in thread(%08X)", split.index, split.offset, split.length, (quint32)(QThread::currentThreadId()) );
    Split_st subchecksum;
    subchecksum = split;
    subchecksum.checksum = 0;

    QFile file(m_filepath);

    if(file.open(QIODevice::ReadOnly)) {
        qint64 offset = split.offset;
        if (file.seek(offset)){
            QByteArray filebuffer = file.read(split.length);

            foreach (const char &byte, filebuffer)
            {
                subchecksum.checksum += (quint8)byte;
            }
        }
        else{
            qWarning("seek failed: %lld", offset);
        }

        file.close();
    }

    return subchecksum;
}

void Checksumer::reduceResult(quint64 &checksum, const Split_st &split)
{
    checksum += split.checksum;
    emit split.checksumer_ptr->Checksumer_ValueChangedSignal(split.index);
#ifdef DEBUG_LOGOUT_ON
    qDebug("reduceResult:split.index(%d), checksum(0x%llX), offset(0x%llX), length(0x%llX), endaddress(0x%llX)", split.index, checksum, split.offset, split.length, (split.offset + split.length));
#endif
}

qint64 Checksumer::getElapsedTime()
{
    if (true == m_elapsedtime.isValid()){
        return m_elapsedtime.elapsed();
    }
    else{
        return -1;
    }
}

quint8 Checksumer::getChecksumerStatus(void)
{
    return m_status;
}

