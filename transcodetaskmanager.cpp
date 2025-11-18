#include "transcodetaskmanager.h"
#include "transcodetask.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>
#include <QThreadPool>

TranscodeTaskManager::TranscodeTaskManager(const QMap<QString, QStringList> &files, QObject *parent)
    : QObject(parent), m_filesToTranscode(files)
{
    m_targetDirectory = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/TranscodeOutput";

    m_threadPool = QThreadPool::globalInstance();

    m_totalFiles = 0;
    m_completedFiles = 0;
    m_failedFiles = 0;
    m_stopped = 0;
}

TranscodeTaskManager::~TranscodeTaskManager()
{
    if (m_threadPool)
    {
        m_threadPool->waitForDone();
    }
}

void TranscodeTaskManager::setTargetDirectory(const QString &targetDir)
{
    m_targetDirectory = targetDir;
}

void TranscodeTaskManager::setTranscodeParams(int crf, const QString &resolution, int frameRate)
{
    m_settings.crf = crf;
    m_settings.resolution = resolution;
    m_settings.framerate = frameRate;
    m_settings.codec = "libx264";
    m_settings.preset = "medium";
    m_settings.pixelFormat = "yuv420p";
    m_settings.colorspace = "bt709";
    m_settings.profile = "high";
    m_settings.faststart = true;
}

void TranscodeTaskManager::setTranscodeSettings(const TranscodeSettings &settings)
{
    m_settings = settings;
}

void TranscodeTaskManager::start()
{
    qDebug() << QString::fromLocal8Bit("开始转码任务...");

    if (!createTargetDirectory(m_targetDirectory))
    {
        emit errorOccurred(QString::fromLocal8Bit("无法创建目标目录: %1").arg(m_targetDirectory));
        emit finished();
        return;
    }

    // 设置线程池大小
    ConfigManager *config = ConfigManager::instance();
    SystemSettings systemSettings = config->getSystemSettings();
    int maxConcurrent = systemSettings.threadCount;
    if (maxConcurrent == 0)
    {
        maxConcurrent = qMax(1, QThread::idealThreadCount() - 1);
    }
    m_threadPool->setMaxThreadCount(maxConcurrent);

    for (auto it = m_filesToTranscode.begin(); it != m_filesToTranscode.end(); ++it)
    {
        QString sourceDir = it.key();
        QStringList files = it.value();

        QDir sourceQDir(sourceDir);
        QString dramaName = sourceQDir.dirName(); // 获取最后一级目录名

        // 目标目录/
        // └── 彩礼加了8万8
        //    ├── 第1集_temp.mp4
        //    ├── 第2集_temp.mp4
        //    └── 第3集_temp.mp4

        QString dramaTargetDir = QDir(m_targetDirectory).absoluteFilePath(dramaName);
        QDir().mkpath(dramaTargetDir);

        for (const QString &fileName : files)
        {
            QString inputPath = QDir(sourceDir).absoluteFilePath(fileName);

            QFileInfo fileInfo(fileName);
            QString baseName = fileInfo.baseName();
            QString finalOutputName = baseName + ".mp4";
            QString tempOutputName = baseName + "_temp.mp4";

            QString finalOutputPath = QDir(dramaTargetDir).absoluteFilePath(finalOutputName);
            QString tempOutputPath = QDir(dramaTargetDir).absoluteFilePath(tempOutputName);
            if (QFile::exists(finalOutputPath))
            {
                qDebug() << QString::fromLocal8Bit("文件已存在，跳过:") << finalOutputPath;
                continue;
            }

            if (QFile::exists(tempOutputPath))
            {
                if (QFile::remove(tempOutputPath))
                {
                    qDebug() << QString::fromLocal8Bit("删除已存在的临时文件:") << tempOutputPath;
                }
                else
                {
                    qDebug() << QString::fromLocal8Bit("删除临时文件失败:") << tempOutputPath;
                }
            }

            TranscodeTask *task = new TranscodeTask(inputPath, tempOutputPath, fileName, m_settings, this);
            m_threadPool->start(task);
            m_totalFiles++;
        }
    }

    qDebug() << QString::fromLocal8Bit("提交了 %1 个任务到线程池，最大并发: %2")
                    .arg(m_totalFiles.loadAcquire())
                    .arg(maxConcurrent);

    // 如果没有文件需要转码，立即发射完成信号
    if (m_totalFiles.loadAcquire() == 0)
    {
        qDebug() << QString::fromLocal8Bit("没有文件需要转码，直接完成");
        emit finished();
        return;
    }
}

void TranscodeTaskManager::onTaskCompleted(const QString &fileName, bool success, const QString &outputPath)
{
    QMutexLocker locker(&m_mutex);

    // 如果已经停止，忽略后续任务结果
    if (m_stopped.loadAcquire())
    {
        return;
    }

    if (success)
    {
        m_completedFiles++;

        // 重命名文件，去掉_temp后缀
        QString finalFilePath = outputPath;
        finalFilePath.replace("_temp", "");

        QFile::rename(outputPath, finalFilePath);
        emit fileProcessed(fileName, true);
    }
    else
    {
        m_failedFiles++;
        emit fileProcessed(fileName, false);
        qDebug() << QString::fromLocal8Bit("转码失败: %1").arg(fileName);
    }

    // 更新进度
    int completed = m_completedFiles.loadAcquire();
    int failed = m_failedFiles.loadAcquire();
    int total = m_totalFiles.loadAcquire();

    if (total > 0)
    {
        int progress = ((completed + failed) * 100) / total;
        emit progressUpdated(progress);
    }

    // 检查是否全部完成
    if (completed + failed >= total)
    {
        qDebug() << QString::fromLocal8Bit("所有任务完成！成功: %1，失败: %2").arg(completed).arg(failed);
        emit finished();
    }
}

bool TranscodeTaskManager::createTargetDirectory(const QString &dirPath)
{
    QDir dir;
    if (!dir.exists(dirPath))
    {
        if (!dir.mkpath(dirPath))
        {
            qDebug() << QString::fromLocal8Bit("无法创建目录: %1").arg(dirPath);
            return false;
        }
    }
    return true;
}

void TranscodeTaskManager::stop()
{
    qDebug() << QString::fromLocal8Bit("停止转码任务...");
    m_stopped.store(1);

    if (m_threadPool)
    {
        m_threadPool->clear(); // 清空等待中的任务
        // 注意：正在运行的任务无法立即停止，但会在完成后被忽略
    }

    emit finished(); // 发出完成信号，结束转码过程
}

void TranscodeTaskManager::onTaskStarted(const QString &fileName)
{
    // 发射当前文件变更信号，将文件状态标记为"转码中"
    emit currentFileChanged(fileName);
}

QString TranscodeTaskManager::generateOutputFileName(const QString &inputFileName, const QString &extension)
{
    QFileInfo fileInfo(inputFileName);
    QString baseName = fileInfo.completeBaseName();
    return QString("%1_temp.%2").arg(baseName).arg(extension);
}
