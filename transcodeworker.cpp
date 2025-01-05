#include "transcodeWorker.h"
#include <QThread>
#include <QDebug>

TranscodeWorker::TranscodeWorker(const QStringList &files, QObject *parent)
    : QThread(parent), filesToTranscode(files) {}

void TranscodeWorker::run() {
    int totalFiles = filesToTranscode.size();
    for (int i = 0; i < totalFiles; ++i) {
        QString file = filesToTranscode.at(i);
        qDebug() << "current proccessing file: " << file;
        QThread::sleep(1);

        // update progress
        int progress = static_cast<int>((i + 1) * 100.0 / totalFiles);
        emit progressUpdated(progress);
    }

    emit finished();
}
