#include <QtWidgets>
#include <QtConcurrent>
#include "checksumer.h"

QString Checksumer::m_filepath = QString();
QList<Split_st> Checksumer::m_splitlist = QList<Split_st>();
quint8 Checksumer::m_status = Checksumer::CHECKSUMER_IDLE;

static const quint32 EVERY_SPLIT_SIZE_SMALL = (1024 * 1024 * 2);
static const quint32 EVERY_SPLIT_SIZE_LARGE = (1024 * 1024 * 10);

#define split_roundup(x,n) ((x+n-1)/n)

Checksumer::Checksumer(QObject *parent) :
    QObject(parent)
{
    QObject::connect(this, SIGNAL(OpenFileButtonClicked(QString)), this, SLOT(OpenFileProcesser(QString)), Qt::AutoConnection);
    QObject::connect(this, SIGNAL(ChecksumButtonClicked()), this, SLOT(ChecksumProcesser()), Qt::AutoConnection);
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
        m_status = CHECKSUMER_CHECKSUMMING;

        // Prepare the QList
        m_splitlist.clear();


#ifdef DEBUG_LOGOUT_ON
        // Get System ideal ThreadProcess Number
        int threadNumber = QThread::idealThreadCount();
        qDebug("ChecksumProcesser::file size : %lld, thread number : %d", filesize, threadNumber);
#endif
        qint64 filesize = fileInfo.size();
        Split_st tempSplit;
        tempSplit.index = 0;
        tempSplit.length = 0;
        tempSplit.checksum = 0;
        tempSplit.offset = 0;
        tempSplit.checksumer_ptr = this;

        int splitcount = split_roundup(filesize, EVERY_SPLIT_SIZE_SMALL);
#ifdef DEBUG_LOGOUT_ON
        qDebug("splitcount:%d", splitcount);
#endif

        for (int loop = 0; loop < splitcount - 1; loop++){
            tempSplit.index = loop;
            tempSplit.offset = EVERY_SPLIT_SIZE_SMALL * loop;
            tempSplit.length = EVERY_SPLIT_SIZE_SMALL;
            m_splitlist.append(tempSplit);
        }

        /* last split */
        tempSplit.index = splitcount - 1;
        tempSplit.offset = EVERY_SPLIT_SIZE_SMALL * (splitcount - 1);

        int lastsplitsize = filesize%EVERY_SPLIT_SIZE_SMALL;
        if(0 == lastsplitsize){
            tempSplit.length = EVERY_SPLIT_SIZE_SMALL;
        }
        else{
            tempSplit.length = lastsplitsize;
        }
        m_splitlist.append(tempSplit);

        emit Checksumer_RangeChangedSignal(0, (splitcount - 1));

//        for (int loop = 0; loop < m_splitlist.size(); loop++){
//            qDebug("splitlist(%d):offset(%lld), length(%lld)", loop, m_splitlist[loop].offset, m_splitlist[loop].length);
//        }

        int mapReduceTime = 0;
        QTime time;
        time.start();

        quint64 final_checksum = QtConcurrent::mappedReduced(m_splitlist, Checksumer::splitChecksum, Checksumer::reduceResult, QtConcurrent::ReduceOptions(QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce));
        emit Checksumer_ChecksumResultSignal(final_checksum);

        mapReduceTime = time.elapsed();
        qDebug() << "MapReduce" << mapReduceTime;
#ifdef DEBUG_LOGOUT_ON
        qDebug("Final result.checksum(0x%X)", final_checksum);
#endif
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
    //qDebug("reduceResult:split.index(%d), result.checksum(0x%X), in thread(0x%08X)", split.index, result.checksum, (quint32)(QThread::currentThreadId()) );
}

