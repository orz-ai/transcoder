#ifndef TRANSCODETASK_H
#define TRANSCODETASK_H

#include <QRunnable>
#include <QString>
#include <QProcess>
#include <configmanager.h>

// 前置声明
class TranscodeTaskManager;

/**
 * 单个转码任务类
 * 继承自QRunnable，用于在线程池中执行
 */
class TranscodeTask : public QRunnable
{
public:
    TranscodeTask(const QString &inputPath, const QString &outputPath,
                  const QString &fileName, const TranscodeSettings &settings,
                  TranscodeTaskManager *manager);

    void run() override;

private:
    QString m_inputPath;
    QString m_outputPath;
    QString m_fileName;
    TranscodeSettings m_settings;
    TranscodeTaskManager *m_manager;

    QString buildFFmpegCommand(const QString &inputPath, const QString &outputPath);
};

#endif // TRANSCODETASK_H