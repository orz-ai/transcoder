// 配置系统测试示例
// 这个文件展示如何使用ConfigManager

#include "configmanager.h"
#include <QDebug>

void testConfigManager()
{
    // 获取配置管理器实例
    ConfigManager *config = ConfigManager::instance();

    // 测试转码设置
    qDebug() << "=== 转码设置测试 ===";
    TranscodeSettings transcodeSettings = config->getTranscodeSettings();
    qDebug() << "编码器:" << transcodeSettings.codec;
    qDebug() << "CRF质量:" << transcodeSettings.crf;
    qDebug() << "分辨率:" << transcodeSettings.resolution;
    qDebug() << "帧率:" << transcodeSettings.framerate;

    // 修改设置
    transcodeSettings.codec = "libx265";
    transcodeSettings.crf = 20;
    config->setTranscodeSettings(transcodeSettings);

    // 测试系统设置
    qDebug() << "=== 系统设置测试 ===";
    SystemSettings systemSettings = config->getSystemSettings();
    qDebug() << "主题:" << systemSettings.theme;
    qDebug() << "语言:" << systemSettings.language;
    qDebug() << "最大任务数:" << systemSettings.maxConcurrentJobs;

    // 保存配置
    if (config->saveConfig())
    {
        qDebug() << QString::fromLocal8Bit("配置保存成功，文件位置:") << config->getConfigFilePath();
    }

    // 测试配置加载
    if (config->loadConfig())
    {
        qDebug() << QString::fromLocal8Bit("配置加载成功");
        TranscodeSettings reloadedSettings = config->getTranscodeSettings();
        qDebug() << "重新加载的编码器:" << reloadedSettings.codec;
        qDebug() << "重新加载的CRF:" << reloadedSettings.crf;
    }
}

/*
使用方法：
在主程序的任何地方调用 testConfigManager() 即可测试配置系统

预期输出示例：
=== 转码设置测试 ===
编码器: "libx264"
CRF质量: 23
分辨率: "720x1280"
帧率: 30
=== 系统设置测试 ===
主题: "modern"
语言: "zh_CN"
最大任务数: 2
配置保存成功，文件位置: "C:/Users/.../AppData/Roaming/Transcoder/transcoder_config.json"
配置加载成功
重新加载的编码器: "libx265"
重新加载的CRF: 20
*/