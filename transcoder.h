
#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFileInfo>
#include <QListView>
#include <QTreeView>
#include "transcodeworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Transcoder; }
QT_END_NAMESPACE

class Transcoder : public QMainWindow

{
    Q_OBJECT

public:
    Transcoder(QWidget *parent = nullptr);
    ~Transcoder();

public slots:
    void renameFile();
    void startTranscode();
    void selectSourceDirs();
    void selectTargetDir();
    void updateProgress(int value);
    void onTranscodeFinished();

private:
    Ui::Transcoder *ui;
    void transcoding(const QStringList &files);
    QStringList validatePaths(const QStringList &paths);
    bool isValidFile(const QFileInfo &file);
    QString buildTranscodeCommand(QString srcPath, QString targetPath);

    TranscodeWorker *worker;
    QStringList selecedPaths;
    QString targetPath;
};

#endif // TRANSCODER_H
