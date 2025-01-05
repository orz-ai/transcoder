#ifndef TRANSCODEWORKER_H
#define TRANSCODEWORKER_H

#include <QThread>
#include <QStringList>

class TranscodeWorker : public QThread {
    Q_OBJECT
public:
    explicit TranscodeWorker(const QStringList &files, QObject *parent = nullptr);
    void run() override;

signals:
    void progressUpdated(int value);
    void finished();

private:
    QStringList filesToTranscode;
};

#endif // TRANSCODEWORKER_H
