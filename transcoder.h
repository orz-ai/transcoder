
#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFileInfo>
#include <QListView>
#include <QTreeView>
#include "transcodetaskmanager.h"
#include "videoinfodialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Transcoder;
}
QT_END_NAMESPACE

class Transcoder : public QMainWindow

{
    Q_OBJECT

public:
    const QStringList supportedExtensions = {"mp4", "mkv", "avi", "mov"};

    Transcoder(QWidget *parent = nullptr);
    ~Transcoder();

public slots:
    void renameFile();
    void startTranscode();
    void selectSourceDirs();
    void selectTargetDir();
    void updateProgress(int value);
    void onTranscodeFinished();
    void onCurrentFileChanged(const QString &fileName);
    void onFileProcessed(const QString &fileName, bool success);
    void onTranscodeError(const QString &errorMessage);
    void switchToModernTheme();
    void switchToDarkTheme();
    void showSelectedDirsDialog();
    void showSettingsDialog();
    void showVideoInfoDialog();

private:
    Ui::Transcoder *ui;
    QMap<QString, QStringList> validatePaths(const QStringList &paths);
    void applyTheme(const QString &themePath);

    TranscodeTaskManager *worker;
    QThread *workerThread;
    QMap<QString, QStringList> selectedPaths;
    QString targetPath;
};

#endif // TRANSCODER_H
