#ifndef FFMPEGUTILS_H
#define FFMPEGUTILS_H

#include <QString>
#include <QStringList>
#include <QSize>

/**
 * FFmpeg工具类
 * 提供视频转码、格式转换、参数设置等功能
 */
class FFmpegUtils
{
public:
    // 视频编码器枚举
    enum VideoCodec
    {
        H264, // libx264
        H265, // libx265
        VP9,  // libvpx-vp9
        AV1   // libaom-av1
    };

    // 音频编码器枚举
    enum AudioCodec
    {
        AAC,  // aac
        MP3,  // libmp3lame
        OPUS, // libopus
        FLAC  // flac
    };

    // 预设质量等级
    enum QualityPreset
    {
        ULTRA_FAST, // ultrafast
        SUPER_FAST, // superfast
        VERY_FAST,  // veryfast
        FASTER,     // faster
        FAST,       // fast
        MEDIUM,     // medium
        SLOW,       // slow
        SLOWER,     // slower
        VERY_SLOW   // veryslow
    };

    // 分辨率预设
    enum ResolutionPreset
    {
        RESOLUTION_480P,  // 854x480
        RESOLUTION_720P,  // 1280x720
        RESOLUTION_1080P, // 1920x1080
        RESOLUTION_1440P, // 2560x1440
        RESOLUTION_4K,    // 3840x2160
        RESOLUTION_CUSTOM // 自定义分辨率
    };

    /**
     * 转码参数结构体
     */
    struct TranscodeParams
    {
        VideoCodec videoCodec;
        AudioCodec audioCodec;
        QualityPreset preset;
        int crf;       // 质量因子 (0-51, 越小质量越好)
        int frameRate; // 帧率
        ResolutionPreset resolutionPreset;
        QSize customResolution; // 自定义分辨率
        int audioBitrate;       // 音频比特率 (kbps)
        bool fastStart;         // 启用 faststart
        QString colorSpace;     // 色彩空间
        QString pixelFormat;    // 像素格式
        QString profile;        // H.264 profile

        // 构造函数提供默认值
        TranscodeParams()
            : videoCodec(H264), audioCodec(AAC), preset(MEDIUM), crf(23), frameRate(30), resolutionPreset(RESOLUTION_720P), customResolution(QSize(720, 1280)), audioBitrate(128), fastStart(true), colorSpace("bt709"), pixelFormat("yuv420p"), profile("high")
        {
        }
    };

public:
    FFmpegUtils() = delete; // 工具类，禁止实例化

    /**
     * 构建基础转码命令
     * @param srcPath 源文件路径
     * @param targetPath 目标文件路径
     * @param params 转码参数
     * @return 完整的ffmpeg命令
     */
    static QString buildTranscodeCommand(const QString &srcPath,
                                         const QString &targetPath,
                                         const TranscodeParams &params = TranscodeParams());

    /**
     * 构建简单的转码命令（使用默认参数）
     * @param srcPath 源文件路径
     * @param targetPath 目标文件路径
     * @return ffmpeg命令
     */
    static QString buildSimpleTranscodeCommand(const QString &srcPath, const QString &targetPath);

    /**
     * 构建视频压缩命令
     * @param srcPath 源文件路径
     * @param targetPath 目标文件路径
     * @param crf 压缩质量因子 (18-28推荐)
     * @return ffmpeg命令
     */
    static QString buildCompressCommand(const QString &srcPath,
                                        const QString &targetPath,
                                        int crf = 23);

    /**
     * 检查ffmpeg是否可用
     * @return true 如果ffmpeg可执行
     */
    static bool isFFmpegAvailable();

private:
    // 辅助方法
    static QString videoCodecToString(VideoCodec codec);
    static QString audioCodecToString(AudioCodec codec);
    static QString qualityPresetToString(QualityPreset preset);
    static QSize resolutionPresetToSize(ResolutionPreset preset);
    static QString escapeFilePath(const QString &path);
};

#endif // FFMPEGUTILS_H