#include "videoinfodialog.h"
#include <QApplication>
#include <QFileInfo>
#include <QTextStream>
#include <QScrollBar>
#include <QFont>

VideoInfoDialog::VideoInfoDialog(QWidget *parent)
    : QDialog(parent), m_ffprobeProcess(nullptr)
{
    setupUI();
    setWindowTitle(QString::fromLocal8Bit("视频信息分析器"));
    setMinimumSize(800, 600);
    resize(900, 700);
}

void VideoInfoDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // 文件选择区域
    m_buttonLayout = new QHBoxLayout();
    m_selectButton = new QPushButton(QString::fromLocal8Bit("选择视频文件"), this);
    m_selectButton->setMinimumHeight(35);
    m_closeButton = new QPushButton(QString::fromLocal8Bit("关闭"), this);
    m_closeButton->setMinimumHeight(35);

    m_buttonLayout->addWidget(m_selectButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_closeButton);

    // 文件路径显示
    m_fileLabel = new QLabel(QString::fromLocal8Bit("请选择视频文件进行分析"), this);
    m_fileLabel->setWordWrap(true);
    m_fileLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 10px; }");

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setRange(0, 0); // 无限进度条

    // 信息显示区域 - 使用滚动区域
    m_scrollArea = new QScrollArea(this);
    m_infoWidget = new QWidget();
    m_infoLayout = new QVBoxLayout(m_infoWidget);
    m_infoLayout->setAlignment(Qt::AlignTop);
    m_infoLayout->setSpacing(5);

    m_scrollArea->setWidget(m_infoWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setMinimumHeight(400);

    // 添加提示标签
    QLabel *hintLabel = new QLabel(QString::fromLocal8Bit("请选择视频文件查看详细信息"), m_infoWidget);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("QLabel { color: #888; font-style: italic; padding: 20px; }");
    m_infoLayout->addWidget(hintLabel);

    // 布局
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_fileLabel);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addWidget(m_scrollArea, 1);

    // 连接信号
    connect(m_selectButton, &QPushButton::clicked, this, &VideoInfoDialog::onSelectVideo);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void VideoInfoDialog::onSelectVideo()
{
    QString videoPath = QFileDialog::getOpenFileName(
        this,
        QString::fromLocal8Bit("选择视频文件"),
        "",
        QString::fromLocal8Bit("视频文件 (*.mp4 *.avi *.mkv *.mov *.wmv *.flv *.webm *.m4v *.3gp *.ts *.mts);;所有文件 (*.*)"));

    if (!videoPath.isEmpty())
    {
        analyzeVideo(videoPath);
    }
}

void VideoInfoDialog::analyzeVideo(const QString &videoPath)
{
    if (videoPath.isEmpty())
    {
        return;
    }

    m_currentVideoPath = videoPath;
    QFileInfo fileInfo(videoPath);
    m_fileLabel->setText(QString::fromLocal8Bit("正在分析: %1").arg(fileInfo.fileName()));

    // 清空之前的信息
    clearInfoDisplay();

    // 开始分析
    setAnalyzing(true);

    // 创建ffprobe进程
    if (m_ffprobeProcess)
    {
        m_ffprobeProcess->kill();
        m_ffprobeProcess->deleteLater();
    }

    m_ffprobeProcess = new QProcess(this);
    connect(m_ffprobeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VideoInfoDialog::onProcessFinished);
    connect(m_ffprobeProcess, &QProcess::errorOccurred,
            this, &VideoInfoDialog::onProcessError);

    // 构建ffprobe命令
    QStringList arguments;
    arguments << "-v" << "quiet"
              << "-print_format" << "json"
              << "-show_format"
              << "-show_streams"
              << "-show_chapters"
              << videoPath;

    // 启动ffprobe
    m_ffprobeProcess->start("ffprobe", arguments);
}

void VideoInfoDialog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    setAnalyzing(false);

    if (exitStatus == QProcess::CrashExit || exitCode != 0)
    {
        QString error = m_ffprobeProcess->readAllStandardError();
        clearInfoDisplay();
        addInfoRow(m_infoLayout, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("分析失败: %1").arg(error));
        return;
    }

    QString output = m_ffprobeProcess->readAllStandardOutput();
    parseAndDisplayVideoInfo(output);

    QFileInfo fileInfo(m_currentVideoPath);
    m_fileLabel->setText(QString::fromLocal8Bit("文件: %1").arg(fileInfo.fileName()));
}

void VideoInfoDialog::onProcessError(QProcess::ProcessError error)
{
    setAnalyzing(false);

    QString errorMsg;
    switch (error)
    {
    case QProcess::FailedToStart:
        errorMsg = QString::fromLocal8Bit("错误: 无法启动ffprobe。请确保FFmpeg已安装并在系统PATH中。");
        break;
    case QProcess::Crashed:
        errorMsg = QString::fromLocal8Bit("错误: ffprobe进程崩溃。");
        break;
    case QProcess::Timedout:
        errorMsg = QString::fromLocal8Bit("错误: ffprobe进程超时。");
        break;
    default:
        errorMsg = QString::fromLocal8Bit("错误: 未知的进程错误。");
        break;
    }

    clearInfoDisplay();
    addInfoRow(m_infoLayout, QString::fromLocal8Bit("错误"), errorMsg);
    QMessageBox::warning(this, QString::fromLocal8Bit("分析错误"), errorMsg);
}

void VideoInfoDialog::setAnalyzing(bool analyzing)
{
    m_progressBar->setVisible(analyzing);
    m_selectButton->setEnabled(!analyzing);
    if (analyzing)
    {
        clearInfoDisplay();
        addInfoRow(m_infoLayout, QString::fromLocal8Bit("状态"), QString::fromLocal8Bit("正在分析视频信息，请稍候..."));
    }
}

void VideoInfoDialog::parseAndDisplayVideoInfo(const QString &jsonOutput)
{
    clearInfoDisplay();

    if (jsonOutput.trimmed().isEmpty())
    {
        addInfoRow(m_infoLayout, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("无法获取视频信息"));
        return;
    }

    // 简单解析JSON (这里可以用QJsonDocument做更精确的解析)
    QStringList lines = jsonOutput.split('\n');
    QString currentSection;

    for (const QString &line : lines)
    {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        // 检查是否是新的段落
        if (trimmed.contains("\"streams\""))
        {
            currentSection = QString::fromLocal8Bit("流信息");
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("═══════════"), currentSection);
        }
        else if (trimmed.contains("\"format\""))
        {
            currentSection = QString::fromLocal8Bit("格式信息");
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("═══════════"), currentSection);
        }

        // 解析具体字段
        if (trimmed.contains("\"codec_name\""))
        {
            QString codec = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("编码格式"), codec);
        }
        else if (trimmed.contains("\"codec_type\""))
        {
            QString type = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("流类型"), type);
        }
        else if (trimmed.contains("\"width\""))
        {
            QString width = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("宽度"), width + " px");
        }
        else if (trimmed.contains("\"height\""))
        {
            QString height = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("高度"), height + " px");
        }
        else if (trimmed.contains("\"duration\""))
        {
            QString duration = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("时长"), formatDuration(duration));
        }
        else if (trimmed.contains("\"bit_rate\""))
        {
            QString bitrate = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("比特率"), formatBitrate(bitrate));
        }
        else if (trimmed.contains("\"r_frame_rate\""))
        {
            QString framerate = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("帧率"), formatFrameRate(framerate));
        }
        else if (trimmed.contains("\"sample_rate\""))
        {
            QString sampleRate = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("采样率"), sampleRate + " Hz");
        }
        else if (trimmed.contains("\"channels\""))
        {
            QString channels = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("声道数"), channels);
        }
        else if (trimmed.contains("\"filename\""))
        {
            QString filename = extractJsonValue(trimmed);
            QFileInfo fileInfo(filename);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("文件名"), fileInfo.fileName());
        }
        else if (trimmed.contains("\"size\""))
        {
            QString size = extractJsonValue(trimmed);
            addInfoRow(m_infoLayout, QString::fromLocal8Bit("文件大小"), formatFileSize(size));
        }
    }
}

void VideoInfoDialog::addInfoRow(QVBoxLayout *layout, const QString &label, const QString &value)
{
    QWidget *rowWidget = new QWidget();
    QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(10, 5, 10, 5);

    QLabel *labelWidget = new QLabel(label + ":");
    labelWidget->setMinimumWidth(100);
    labelWidget->setStyleSheet("QLabel { font-weight: bold; color: #2c3e50; }");

    QLabel *valueWidget = new QLabel(value);
    valueWidget->setWordWrap(true);
    valueWidget->setStyleSheet("QLabel { color: #34495e; }");

    rowLayout->addWidget(labelWidget);
    rowLayout->addWidget(valueWidget, 1);

    layout->addWidget(rowWidget);
}

void VideoInfoDialog::clearInfoDisplay()
{
    // 清除所有子控件
    QLayoutItem *item;
    while ((item = m_infoLayout->takeAt(0)))
    {
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
}

QString VideoInfoDialog::extractJsonValue(const QString &jsonLine)
{
    // 简单提取JSON值 "key": "value"
    int colonPos = jsonLine.indexOf(':');
    if (colonPos == -1)
        return jsonLine.trimmed();

    QString value = jsonLine.mid(colonPos + 1).trimmed();
    value.remove('"').remove(',');
    return value.trimmed();
}

QString VideoInfoDialog::formatDuration(const QString &seconds)
{
    bool ok;
    double duration = seconds.toDouble(&ok);
    if (!ok)
        return seconds;

    int hours = duration / 3600;
    int minutes = (duration - hours * 3600) / 60;
    int secs = duration - hours * 3600 - minutes * 60;

    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

QString VideoInfoDialog::formatBitrate(const QString &bitrate)
{
    bool ok;
    int rate = bitrate.toInt(&ok);
    if (!ok)
        return bitrate;

    if (rate >= 1000000)
    {
        return QString::number(rate / 1000000.0, 'f', 1) + " Mbps";
    }
    else if (rate >= 1000)
    {
        return QString::number(rate / 1000.0, 'f', 1) + " Kbps";
    }
    return bitrate + " bps";
}

QString VideoInfoDialog::formatFrameRate(const QString &frameRate)
{
    if (frameRate.contains('/'))
    {
        QStringList parts = frameRate.split('/');
        if (parts.size() == 2)
        {
            bool ok1, ok2;
            double num = parts[0].toDouble(&ok1);
            double den = parts[1].toDouble(&ok2);
            if (ok1 && ok2 && den != 0)
            {
                return QString::number(num / den, 'f', 2) + " fps";
            }
        }
    }
    return frameRate + " fps";
}

QString VideoInfoDialog::formatFileSize(const QString &bytes)
{
    bool ok;
    qint64 size = bytes.toLongLong(&ok);
    if (!ok)
        return bytes;

    if (size >= 1024 * 1024 * 1024)
    {
        return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
    else if (size >= 1024 * 1024)
    {
        return QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
    }
    else if (size >= 1024)
    {
        return QString::number(size / 1024.0, 'f', 2) + " KB";
    }
    return QString::number(size) + " B";
}