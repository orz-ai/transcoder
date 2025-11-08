#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QApplication>
#include <QFile>

SettingDialog::SettingDialog(QWidget *parent) : QDialog(parent),
                                                ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
    connectSignals();
    loadSettings();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::connectSignals()
{
    // 重置按钮
    connect(ui->resetButton, &QPushButton::clicked, this, &SettingDialog::onResetButtonClicked);

    // 路径浏览按钮
    connect(ui->browseSourceButton, &QPushButton::clicked, this, &SettingDialog::onBrowseSourceButtonClicked);
    connect(ui->browseTargetButton, &QPushButton::clicked, this, &SettingDialog::onBrowseTargetButtonClicked);

    // 主题变更
    connect(ui->themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingDialog::onThemeChanged);
}

void SettingDialog::loadSettings()
{
    ConfigManager *config = ConfigManager::instance();

    // 加载转码设置
    setTranscodeSettingsToUI(config->getTranscodeSettings());

    // 加载系统设置
    setSystemSettingsToUI(config->getSystemSettings());
}

void SettingDialog::saveSettings()
{
    ConfigManager *config = ConfigManager::instance();

    // 保存转码设置
    config->setTranscodeSettings(getTranscodeSettingsFromUI());

    // 保存系统设置
    config->setSystemSettings(getSystemSettingsFromUI());

    // 保存到文件
    if (config->saveConfig())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("设置"), QString::fromLocal8Bit("设置已保存成功！"));
    }
    else
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("设置"), QString::fromLocal8Bit("设置保存失败！"));
    }
}

void SettingDialog::resetToDefaults()
{
    TranscodeSettings defaultTranscode;
    SystemSettings defaultSystem;

    setTranscodeSettingsToUI(defaultTranscode);
    setSystemSettingsToUI(defaultSystem);
}

TranscodeSettings SettingDialog::getTranscodeSettingsFromUI() const
{
    TranscodeSettings settings;

    // 编码器
    QString codecText = ui->codecComboBox->currentText();
    if (codecText.contains("libx264"))
    {
        settings.codec = "libx264";
    }
    else if (codecText.contains("libx265"))
    {
        settings.codec = "libx265";
    }
    else if (codecText.contains("libaom-av1"))
    {
        settings.codec = "libaom-av1";
    }

    // CRF质量
    settings.crf = ui->crfSpinBox->value();

    // 预设
    QString presetText = ui->presetComboBox->currentText();
    if (presetText.contains("ultrafast"))
        settings.preset = "ultrafast";
    else if (presetText.contains("superfast"))
        settings.preset = "superfast";
    else if (presetText.contains("veryfast"))
        settings.preset = "veryfast";
    else if (presetText.contains("faster"))
        settings.preset = "faster";
    else if (presetText.contains("fast"))
        settings.preset = "fast";
    else if (presetText.contains("medium"))
        settings.preset = "medium";
    else if (presetText.contains("slow"))
        settings.preset = "slow";
    else if (presetText.contains("slower"))
        settings.preset = "slower";
    else if (presetText.contains("veryslow"))
        settings.preset = "veryslow";

    // 分辨率
    QString resolutionText = ui->resolutionComboBox->currentText();
    if (resolutionText.contains("1920x1080"))
    {
        settings.resolution = "1920x1080";
    }
    else if (resolutionText.contains("1280x720"))
    {
        settings.resolution = "1280x720";
    }
    else if (resolutionText.contains("720x1280"))
    {
        settings.resolution = "720x1280";
    }
    else if (resolutionText.contains("1080x1920"))
    {
        settings.resolution = "1080x1920";
    }
    else if (resolutionText.contains("原始尺寸"))
    {
        settings.resolution = "original";
    }
    else
    {
        settings.resolution = resolutionText; // 自定义分辨率
    }

    // 帧率
    settings.framerate = ui->framerateSpinBox->value();

    // 像素格式
    QString pixelText = ui->pixelFormatComboBox->currentText();
    if (pixelText.contains("yuv420p"))
        settings.pixelFormat = "yuv420p";
    else if (pixelText.contains("yuv444p"))
        settings.pixelFormat = "yuv444p";
    else if (pixelText.contains("yuv422p"))
        settings.pixelFormat = "yuv422p";

    // 色彩空间
    QString colorspaceText = ui->colorspaceComboBox->currentText();
    if (colorspaceText.contains("bt709"))
        settings.colorspace = "bt709";
    else if (colorspaceText.contains("bt601"))
        settings.colorspace = "bt601";
    else if (colorspaceText.contains("bt2020"))
        settings.colorspace = "bt2020";

    // 编码档次
    QString profileText = ui->profileComboBox->currentText();
    if (profileText.contains("high"))
        settings.profile = "high";
    else if (profileText.contains("main"))
        settings.profile = "main";
    else if (profileText.contains("baseline"))
        settings.profile = "baseline";

    // 线程数
    settings.threads = ui->threadsSpinBox->value();

    // 快速启动
    settings.faststart = ui->faststartCheckBox->isChecked();

    return settings;
}

SystemSettings SettingDialog::getSystemSettingsFromUI() const
{
    SystemSettings settings;

    // 主题
    settings.theme = (ui->themeComboBox->currentIndex() == 0) ? "modern" : "dark";

    // 语言
    settings.language = (ui->languageComboBox->currentIndex() == 0) ? "zh_CN" : "en_US";

    // 默认路径
    settings.defaultSourcePath = ui->defaultSourceLineEdit->text();
    settings.defaultTargetPath = ui->defaultTargetLineEdit->text();

    // 任务设置
    settings.maxConcurrentJobs = ui->maxJobsSpinBox->value();
    settings.autoSaveProgress = ui->autoSaveProgressCheckBox->isChecked();
    settings.showNotifications = ui->showNotificationsCheckBox->isChecked();
    settings.autoStart = ui->autoStartCheckBox->isChecked();

    return settings;
}

void SettingDialog::setTranscodeSettingsToUI(const TranscodeSettings &settings)
{
    // 编码器
    if (settings.codec == "libx264")
    {
        ui->codecComboBox->setCurrentIndex(0);
    }
    else if (settings.codec == "libx265")
    {
        ui->codecComboBox->setCurrentIndex(1);
    }
    else if (settings.codec == "libaom-av1")
    {
        ui->codecComboBox->setCurrentIndex(2);
    }

    // CRF质量
    ui->crfSlider->setValue(settings.crf);
    ui->crfSpinBox->setValue(settings.crf);

    // 预设
    QStringList presets = {"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"};
    int presetIndex = presets.indexOf(settings.preset);
    if (presetIndex != -1)
    {
        ui->presetComboBox->setCurrentIndex(presetIndex);
    }

    // 分辨率
    if (settings.resolution == "1920x1080")
    {
        ui->resolutionComboBox->setCurrentIndex(0);
    }
    else if (settings.resolution == "1280x720")
    {
        ui->resolutionComboBox->setCurrentIndex(1);
    }
    else if (settings.resolution == "720x1280")
    {
        ui->resolutionComboBox->setCurrentIndex(2);
    }
    else if (settings.resolution == "1080x1920")
    {
        ui->resolutionComboBox->setCurrentIndex(3);
    }
    else if (settings.resolution == "original")
    {
        ui->resolutionComboBox->setCurrentIndex(4);
    }
    else
    {
        ui->resolutionComboBox->setEditText(settings.resolution);
    }

    // 帧率
    ui->framerateSpinBox->setValue(settings.framerate);

    // 像素格式
    if (settings.pixelFormat == "yuv420p")
    {
        ui->pixelFormatComboBox->setCurrentIndex(0);
    }
    else if (settings.pixelFormat == "yuv444p")
    {
        ui->pixelFormatComboBox->setCurrentIndex(1);
    }
    else if (settings.pixelFormat == "yuv422p")
    {
        ui->pixelFormatComboBox->setCurrentIndex(2);
    }

    // 色彩空间
    if (settings.colorspace == "bt709")
    {
        ui->colorspaceComboBox->setCurrentIndex(0);
    }
    else if (settings.colorspace == "bt601")
    {
        ui->colorspaceComboBox->setCurrentIndex(1);
    }
    else if (settings.colorspace == "bt2020")
    {
        ui->colorspaceComboBox->setCurrentIndex(2);
    }

    // 编码档次
    if (settings.profile == "high")
    {
        ui->profileComboBox->setCurrentIndex(0);
    }
    else if (settings.profile == "main")
    {
        ui->profileComboBox->setCurrentIndex(1);
    }
    else if (settings.profile == "baseline")
    {
        ui->profileComboBox->setCurrentIndex(2);
    }

    // 线程数
    ui->threadsSpinBox->setValue(settings.threads);

    // 快速启动
    ui->faststartCheckBox->setChecked(settings.faststart);
}

void SettingDialog::setSystemSettingsToUI(const SystemSettings &settings)
{
    // 主题
    ui->themeComboBox->setCurrentIndex((settings.theme == "modern") ? 0 : 1);

    // 语言
    ui->languageComboBox->setCurrentIndex((settings.language == "zh_CN") ? 0 : 1);

    // 默认路径
    ui->defaultSourceLineEdit->setText(settings.defaultSourcePath);
    ui->defaultTargetLineEdit->setText(settings.defaultTargetPath);

    // 任务设置
    ui->maxJobsSpinBox->setValue(settings.maxConcurrentJobs);
    ui->autoSaveProgressCheckBox->setChecked(settings.autoSaveProgress);
    ui->showNotificationsCheckBox->setChecked(settings.showNotifications);
    ui->autoStartCheckBox->setChecked(settings.autoStart);
}

void SettingDialog::onResetButtonClicked()
{
    int ret = QMessageBox::question(this, "确认重置",
                                    "确定要恢复所有设置为默认值吗？",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        resetToDefaults();
    }
}

void SettingDialog::onBrowseSourceButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择默认源目录",
                                                    ui->defaultSourceLineEdit->text());
    if (!dir.isEmpty())
    {
        ui->defaultSourceLineEdit->setText(dir);
    }
}

void SettingDialog::onBrowseTargetButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择默认输出目录",
                                                    ui->defaultTargetLineEdit->text());
    if (!dir.isEmpty())
    {
        ui->defaultTargetLineEdit->setText(dir);
    }
}

void SettingDialog::onThemeChanged()
{
    QString theme = (ui->themeComboBox->currentIndex() == 0) ? "modern" : "dark";
    applyTheme(theme);
}

void SettingDialog::applyTheme(const QString &theme)
{
    QString themeFile = (theme == "modern") ? ":/styles/modern.qss" : ":/styles/dark.qss";
    QFile qss(themeFile);
    if (qss.open(QFile::ReadOnly | QFile::Text))
    {
        qApp->setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }
}

void SettingDialog::accept()
{
    saveSettings();
    QDialog::accept();
}
