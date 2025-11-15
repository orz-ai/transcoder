#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QStandardPaths>
#include <QDir>

struct TranscodeSettings
{

    QString codec = "libx264";       // 编码器
    int crf = 23;                    // CRF质量
    QString preset = "medium";       // 编码预设
    QString resolution = "720x1280"; // 分辨率
    int framerate = 30;              // 帧率
    QString pixelFormat = "yuv420p"; // 像素格式
    QString colorspace = "bt709";    // 色彩空间
    bool faststart = true;           // 快速启动
    QString profile = "high";        // 编码档次
};

struct SystemSettings
{
    QString theme = "modern";       // 主题：modern/dark
    QString language = "zh_CN";     // 语言
    bool autoSaveProgress = true;   // 自动保存进度
    QString defaultSourcePath = ""; // 默认源目录
    QString defaultTargetPath = ""; // 默认输出目录
    bool showNotifications = true;  // 显示通知
    bool autoStart = false;         // 开机自启
    int threadCount = 0;            // 线程数（0=自动检测）
};

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager *instance();

    // 转码设置
    const TranscodeSettings &getTranscodeSettings() const { return m_transcodeSettings; }
    void setTranscodeSettings(const TranscodeSettings &settings);

    // 系统设置
    const SystemSettings &getSystemSettings() const { return m_systemSettings; }
    void setSystemSettings(const SystemSettings &settings);

    // 文件操作
    bool loadConfig();
    bool saveConfig();
    QString getConfigFilePath() const;

signals:
    void configChanged();
    void transcodeSettingsChanged();
    void systemSettingsChanged();

private:
    explicit ConfigManager(QObject *parent = nullptr);
    static ConfigManager *m_instance;

    TranscodeSettings m_transcodeSettings;
    SystemSettings m_systemSettings;

    void setDefaultValues();
    QJsonObject transcodeSettingsToJson() const;
    QJsonObject systemSettingsToJson() const;
    void transcodeSettingsFromJson(const QJsonObject &json);
    void systemSettingsFromJson(const QJsonObject &json);
};

#endif // CONFIGMANAGER_H
