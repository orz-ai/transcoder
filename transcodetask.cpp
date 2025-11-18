#include "transcodetask.h"
#include "transcodetaskmanager.h"
#include "utils/ffmpegutils.h"
#include <QDebug>
#include <QProcess>

TranscodeTask::TranscodeTask(const QString &inputPath, const QString &outputPath,
                             const QString &fileName, const TranscodeSettings &settings,
                             TranscodeTaskManager *manager)
    : m_inputPath(inputPath), m_outputPath(outputPath), m_fileName(fileName), m_settings(settings), m_manager(manager)
{
    setAutoDelete(true); // 任务完成后自动删除
}

void TranscodeTask::run()
{
    qDebug() << QString::fromLocal8Bit("开始转码: %1").arg(m_fileName);

    // 通知管理器任务开始
    if (m_manager)
    {
        m_manager->onTaskStarted(m_fileName);
    }

    QString command = buildFFmpegCommand(m_inputPath, m_outputPath);
    QProcess process;

    process.start(command);
    bool success = process.waitForFinished(-1) &&
                   process.exitCode() == 0 &&
                   process.exitStatus() == QProcess::NormalExit;

    qDebug() << QString::fromLocal8Bit("转码%1: %2").arg(success ? QString::fromLocal8Bit("成功") : QString::fromLocal8Bit("失败")).arg(m_fileName);

    // 调用管理器的回调函数
    if (m_manager)
    {
        m_manager->onTaskCompleted(m_fileName, success, m_outputPath);
    }
}

QString TranscodeTask::buildFFmpegCommand(const QString &inputPath, const QString &outputPath)
{
    FFmpegUtils::TranscodeParams params;

    params.crf = m_settings.crf;
    params.frameRate = m_settings.framerate;
    params.colorSpace = m_settings.colorspace;
    params.pixelFormat = m_settings.pixelFormat;
    params.profile = m_settings.profile;
    params.fastStart = m_settings.faststart;
    params.audioBitrate = 128;

    // 设置编码器
    if (m_settings.codec == "libx264")
    {
        params.videoCodec = FFmpegUtils::H264;
    }
    else if (m_settings.codec == "libx265")
    {
        params.videoCodec = FFmpegUtils::H265;
    }
    else
    {
        params.videoCodec = FFmpegUtils::H264; // 默认H264
    }

    // 解析分辨率
    QStringList resParts = m_settings.resolution.split('x');
    if (resParts.size() == 2)
    {
        int width = resParts[0].toInt();
        int height = resParts[1].toInt();
        params.customResolution = QSize(width, height);
        params.resolutionPreset = FFmpegUtils::RESOLUTION_CUSTOM;
    }
    else
    {
        params.customResolution = QSize(720, 1280);
        params.resolutionPreset = FFmpegUtils::RESOLUTION_CUSTOM;
    }

    // 设置预设
    if (m_settings.preset == "ultrafast")
    {
        params.preset = FFmpegUtils::ULTRA_FAST;
    }
    else if (m_settings.preset == "superfast")
    {
        params.preset = FFmpegUtils::SUPER_FAST;
    }
    else if (m_settings.preset == "veryfast")
    {
        params.preset = FFmpegUtils::VERY_FAST;
    }
    else if (m_settings.preset == "faster")
    {
        params.preset = FFmpegUtils::FASTER;
    }
    else if (m_settings.preset == "fast")
    {
        params.preset = FFmpegUtils::FAST;
    }
    else if (m_settings.preset == "medium")
    {
        params.preset = FFmpegUtils::MEDIUM;
    }
    else if (m_settings.preset == "slow")
    {
        params.preset = FFmpegUtils::SLOW;
    }
    else if (m_settings.preset == "slower")
    {
        params.preset = FFmpegUtils::SLOWER;
    }
    else if (m_settings.preset == "veryslow")
    {
        params.preset = FFmpegUtils::VERY_SLOW;
    }
    else
    {
        params.preset = FFmpegUtils::MEDIUM;
    }

    return FFmpegUtils::buildTranscodeCommand(inputPath, outputPath, params);
}
