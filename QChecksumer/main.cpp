#include "checksumermain.h"
#include "checksumer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);

    Checksumer * const checksumer = new Checksumer();

    // Move Checksumer to a sub thread
    QThread * const workerThread = new QThread();
    checksumer->moveToThread(workerThread);
    workerThread->setObjectName("Checksumer");
    static_cast<void>(QObject::connect(workerThread, SIGNAL(started()), checksumer, SLOT(threadStarted())));
    workerThread->start();

    ChecksumerMain w(checksumer);

    // Remove "?" Button from QDialog
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    w.setWindowFlags(flags);

    w.show();

    return a.exec();
}
