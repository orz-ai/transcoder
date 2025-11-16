#ifndef VIDEOINFODIALOG_H
#define VIDEOINFODIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>

class VideoInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoInfoDialog(QWidget *parent = nullptr);
    void analyzeVideo(const QString &videoPath);

private slots:
    void onSelectVideo();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void setupUI();
    void setAnalyzing(bool analyzing);
    void parseAndDisplayVideoInfo(const QString &jsonOutput);
    void addInfoRow(QVBoxLayout *layout, const QString &label, const QString &value);
    void clearInfoDisplay();
    QString extractJsonValue(const QString &jsonLine);
    QString formatDuration(const QString &seconds);
    QString formatBitrate(const QString &bitrate);
    QString formatFrameRate(const QString &frameRate);
    QString formatFileSize(const QString &bytes);

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_selectButton;
    QPushButton *m_closeButton;
    QLabel *m_fileLabel;
    QProgressBar *m_progressBar;
    QScrollArea *m_scrollArea;
    QWidget *m_infoWidget;
    QVBoxLayout *m_infoLayout;
    QProcess *m_ffprobeProcess;
    QString m_currentVideoPath;
};

#endif // VIDEOINFODIALOG_H