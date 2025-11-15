#ifndef TRANSCODETASKMANAGER_H
#define TRANSCODETASKMANAGER_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QProcess>
#include <QMutex>
#include <QThreadPool>
#include <QRunnable>
#include <QAtomicInt>
#include <configmanager.h>
#include "transcodetask.h"

/**
 * 转码任务管理器
 * 使用Qt线程池管理转码任务的并发执行
 */
class TranscodeTaskManager : public QObject
{
    Q_OBJECT

public:
    explicit TranscodeTaskManager(const QMap<QString, QStringList> &files, QObject *parent = nullptr);
    ~TranscodeTaskManager();

    // 任务完成回调（供TranscodeTask调用）
    void onTaskCompleted(const QString &fileName, bool success, const QString &outputPath);

public slots:
    void start();

    // 设置目标输出目录
    void setTargetDirectory(const QString &targetDir);

    // 设置转码参数（简化版）
    void setTranscodeParams(int crf = 23, const QString &resolution = "720x1280", int frameRate = 30);

    // 设置完整的转码设置
    void setTranscodeSettings(const TranscodeSettings &settings);

signals:
    void progressUpdated(int value);
    void finished();
    void fileProcessed(const QString &fileName, bool success);
    void currentFileChanged(const QString &fileName);
    void errorOccurred(const QString &errorMessage);

private:
    QMap<QString, QStringList> m_filesToTranscode;
    QString m_targetDirectory;
    TranscodeSettings m_settings;
    QThreadPool *m_threadPool;
    QMutex m_mutex;

    // 进度跟踪
    QAtomicInt m_totalFiles;
    QAtomicInt m_completedFiles;
    QAtomicInt m_failedFiles;

    // 私有方法
    bool createTargetDirectory(const QString &dirPath);
    QString generateOutputFileName(const QString &inputFileName, const QString &extension = "mp4");
};

#endif // TRANSCODETASKMANAGER_H