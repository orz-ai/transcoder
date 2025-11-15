#include "ffmpegutils.h"
#include <QProcess>
#include <QFileInfo>
#include <QDebug>

QString FFmpegUtils::buildTranscodeCommand(const QString &srcPath,
                                           const QString &targetPath,
                                           const TranscodeParams &params)
{
    QStringList args;
    args << "ffmpeg" << "-i" << escapeFilePath(srcPath);

    // 视频编码器设置
    args << "-c:v" << videoCodecToString(params.videoCodec);

    // 分辨率设置
    QSize resolution;
    if (params.resolutionPreset == RESOLUTION_CUSTOM)
    {
        resolution = params.customResolution;
    }
    else
    {
        resolution = resolutionPresetToSize(params.resolutionPreset);
    }
    args << "-s" << QString("%1x%2").arg(resolution.width()).arg(resolution.height());

    // 帧率设置
    if (params.frameRate > 0)
    {
        args << "-r" << QString::number(params.frameRate);
    }

    // 质量设置
    if (params.videoCodec == H264 || params.videoCodec == H265)
    {
        args << "-crf" << QString::number(params.crf);
        args << "-preset" << qualityPresetToString(params.preset);
    }

    // 像素格式
    if (!params.pixelFormat.isEmpty())
    {
        args << "-pix_fmt" << params.pixelFormat;
    }

    // 色彩空间设置
    if (!params.colorSpace.isEmpty())
    {
        args << "-colorspace" << params.colorSpace;
        args << "-color_primaries" << params.colorSpace;
        args << "-color_trc" << params.colorSpace;
        args << "-color_range" << "tv";
    }

    // H.264 profile设置
    if (params.videoCodec == H264 && !params.profile.isEmpty())
    {
        args << "-profile:v" << params.profile;
    }

    // 音频编码器设置
    args << "-c:a" << audioCodecToString(params.audioCodec);

    // 音频比特率
    if (params.audioBitrate > 0)
    {
        args << "-b:a" << QString("%1k").arg(params.audioBitrate);
    }

    // FastStart优化
    if (params.fastStart)
    {
        args << "-movflags" << "faststart";
    }

    // 输出文件
    args << escapeFilePath(targetPath);

    return args.join(" ");
}

QString FFmpegUtils::buildSimpleTranscodeCommand(const QString &srcPath, const QString &targetPath)
{
    TranscodeParams params;
    params.crf = 23;
    params.frameRate = 30;
    params.customResolution = QSize(720, 1280);
    params.resolutionPreset = RESOLUTION_CUSTOM;

    return buildTranscodeCommand(srcPath, targetPath, params);
}

QString FFmpegUtils::buildCompressCommand(const QString &srcPath,
                                          const QString &targetPath,
                                          int crf)
{
    QStringList args;
    args << "ffmpeg" << "-i" << escapeFilePath(srcPath);
    args << "-c:v" << "libx264";
    args << "-crf" << QString::number(qBound(0, crf, 51));
    args << "-preset" << "medium";
    args << "-c:a" << "aac";
    args << "-b:a" << "128k";
    args << "-movflags" << "faststart";
    args << escapeFilePath(targetPath);

    return args.join(" ");
}

bool FFmpegUtils::isFFmpegAvailable()
{
    QProcess process;
    process.start("ffmpeg", QStringList() << "-version");
    process.waitForFinished(3000); // 等待3秒

    return process.exitCode() == 0;
}

QString FFmpegUtils::videoCodecToString(VideoCodec codec)
{
    switch (codec)
    {
    case H264:
        return "libx264";
    case H265:
        return "libx265";
    case VP9:
        return "libvpx-vp9";
    case AV1:
        return "libaom-av1";
    default:
        return "libx264";
    }
}

QString FFmpegUtils::audioCodecToString(AudioCodec codec)
{
    switch (codec)
    {
    case AAC:
        return "aac";
    case MP3:
        return "libmp3lame";
    case OPUS:
        return "libopus";
    case FLAC:
        return "flac";
    default:
        return "aac";
    }
}

QString FFmpegUtils::qualityPresetToString(QualityPreset preset)
{
    switch (preset)
    {
    case ULTRA_FAST:
        return "ultrafast";
    case SUPER_FAST:
        return "superfast";
    case VERY_FAST:
        return "veryfast";
    case FASTER:
        return "faster";
    case FAST:
        return "fast";
    case MEDIUM:
        return "medium";
    case SLOW:
        return "slow";
    case SLOWER:
        return "slower";
    case VERY_SLOW:
        return "veryslow";
    default:
        return "medium";
    }
}

QSize FFmpegUtils::resolutionPresetToSize(ResolutionPreset preset)
{
    switch (preset)
    {
    case RESOLUTION_480P:
        return QSize(854, 480);
    case RESOLUTION_720P:
        return QSize(1280, 720);
    case RESOLUTION_1080P:
        return QSize(1920, 1080);
    case RESOLUTION_1440P:
        return QSize(2560, 1440);
    case RESOLUTION_4K:
        return QSize(3840, 2160);
    default:
        return QSize(1280, 720);
    }
}

QString FFmpegUtils::escapeFilePath(const QString &path)
{
    // 在Windows上，如果路径包含空格，需要用引号包围
    if (path.contains(' ') || path.contains('&') || path.contains('(') || path.contains(')'))
    {
        return QString("\"%1\"").arg(path);
    }
    return path;
}