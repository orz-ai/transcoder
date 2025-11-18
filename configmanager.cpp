#include "configmanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QApplication>

ConfigManager *ConfigManager::m_instance = nullptr;

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    setDefaultValues();
    loadConfig();
}

ConfigManager *ConfigManager::instance()
{
    if (!m_instance)
    {
        m_instance = new ConfigManager();
    }
    return m_instance;
}

void ConfigManager::setDefaultValues()
{
    // 转码设置默认值
    m_transcodeSettings = TranscodeSettings();

    // 系统设置默认值
    m_systemSettings = SystemSettings();
    m_systemSettings.defaultSourcePath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    m_systemSettings.defaultTargetPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/Transcoded";
}

QString ConfigManager::getConfigFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists())
    {
        dir.mkpath(configDir);
    }
    return dir.filePath("transcoder_config.json");
}

bool ConfigManager::loadConfig()
{
    QString configPath = getConfigFilePath();
    QFile file(configPath);

    if (!file.exists())
    {
        qDebug() << QString::fromLocal8Bit("配置文件不存在，使用默认设置:") << configPath;
        return saveConfig(); // 创建默认配置文件
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << QString::fromLocal8Bit("无法打开配置文件:") << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON解析错误:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    // 加载转码设置
    if (root.contains("transcode"))
    {
        transcodeSettingsFromJson(root["transcode"].toObject());
    }

    // 加载系统设置
    if (root.contains("system"))
    {
        systemSettingsFromJson(root["system"].toObject());
    }

    qDebug() << QString::fromLocal8Bit("配置文件加载成功:") << configPath;
    return true;
}

bool ConfigManager::saveConfig()
{
    QString configPath = getConfigFilePath();
    QFile file(configPath);

    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << QString::fromLocal8Bit("无法创建配置文件:") << file.errorString();
        return false;
    }

    QJsonObject root;
    root["version"] = "1.0";
    root["transcode"] = transcodeSettingsToJson();
    root["system"] = systemSettingsToJson();

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();

    qDebug() << QString::fromLocal8Bit("配置文件保存成功:") << configPath;
    return true;
}

void ConfigManager::setTranscodeSettings(const TranscodeSettings &settings)
{
    m_transcodeSettings = settings;
    saveConfig(); // 保存到文件
    emit transcodeSettingsChanged();
    emit configChanged();
}

void ConfigManager::setSystemSettings(const SystemSettings &settings)
{
    m_systemSettings = settings;
    saveConfig();
    emit systemSettingsChanged();
    emit configChanged();
}

QJsonObject ConfigManager::transcodeSettingsToJson() const
{
    QJsonObject json;
    json["codec"] = m_transcodeSettings.codec;
    json["crf"] = m_transcodeSettings.crf;
    json["preset"] = m_transcodeSettings.preset;
    json["resolution"] = m_transcodeSettings.resolution;
    json["framerate"] = m_transcodeSettings.framerate;
    json["pixelFormat"] = m_transcodeSettings.pixelFormat;
    json["colorspace"] = m_transcodeSettings.colorspace;
    json["faststart"] = m_transcodeSettings.faststart;
    json["profile"] = m_transcodeSettings.profile;
    return json;
}

QJsonObject ConfigManager::systemSettingsToJson() const
{
    QJsonObject json;
    json["theme"] = m_systemSettings.theme;
    json["language"] = m_systemSettings.language;
    json["autoSaveProgress"] = m_systemSettings.autoSaveProgress;
    json["defaultSourcePath"] = m_systemSettings.defaultSourcePath;
    json["defaultTargetPath"] = m_systemSettings.defaultTargetPath;
    json["showNotifications"] = m_systemSettings.showNotifications;
    json["autoStart"] = m_systemSettings.autoStart;
    json["threadCount"] = m_systemSettings.threadCount;
    return json;
}

void ConfigManager::transcodeSettingsFromJson(const QJsonObject &json)
{
    if (json.contains("codec"))
        m_transcodeSettings.codec = json["codec"].toString();
    if (json.contains("crf"))
        m_transcodeSettings.crf = json["crf"].toInt();
    if (json.contains("preset"))
        m_transcodeSettings.preset = json["preset"].toString();
    if (json.contains("resolution"))
        m_transcodeSettings.resolution = json["resolution"].toString();
    if (json.contains("framerate"))
        m_transcodeSettings.framerate = json["framerate"].toInt();
    if (json.contains("pixelFormat"))
        m_transcodeSettings.pixelFormat = json["pixelFormat"].toString();
    if (json.contains("colorspace"))
        m_transcodeSettings.colorspace = json["colorspace"].toString();
    if (json.contains("faststart"))
        m_transcodeSettings.faststart = json["faststart"].toBool();
    if (json.contains("profile"))
        m_transcodeSettings.profile = json["profile"].toString();
}

void ConfigManager::systemSettingsFromJson(const QJsonObject &json)
{
    if (json.contains("theme"))
        m_systemSettings.theme = json["theme"].toString();
    if (json.contains("language"))
        m_systemSettings.language = json["language"].toString();
    if (json.contains("autoSaveProgress"))
        m_systemSettings.autoSaveProgress = json["autoSaveProgress"].toBool();
    if (json.contains("defaultSourcePath"))
        m_systemSettings.defaultSourcePath = json["defaultSourcePath"].toString();
    if (json.contains("defaultTargetPath"))
        m_systemSettings.defaultTargetPath = json["defaultTargetPath"].toString();
    if (json.contains("showNotifications"))
        m_systemSettings.showNotifications = json["showNotifications"].toBool();
    if (json.contains("autoStart"))
        m_systemSettings.autoStart = json["autoStart"].toBool();
    if (json.contains("threadCount"))
        m_systemSettings.threadCount = json["threadCount"].toInt();
}
