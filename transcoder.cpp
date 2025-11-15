
#include "transcoder.h"
#include "ui_transcoder.h"
#include "configmanager.h"
#include <RenameDialog.h>
#include <selecteddirsdialog.h>
#include <settingdialog.h>
#include <qdebug.h>
#include <utils/HttpClient.h>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QShortcut>
#include <QThread>

Transcoder::Transcoder(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Transcoder)
{
    ui->setupUi(this);

    ui->progressLayout->hide();
    ui->progressBar->setValue(0);

    connect(ui->renameFileBtn, &QPushButton::clicked, this, &Transcoder::renameFile);
    // 选择
    connect(ui->transcodeBtn, &QPushButton::clicked, this, &Transcoder::startTranscode);
    // 选择转码目录
    connect(ui->sourceDirBtn, &QPushButton::clicked, this, &Transcoder::selectSourceDirs);
    // 选择转码结果保存目录
    connect(ui->targetDirButton, &QPushButton::clicked, this, &Transcoder::selectTargetDir);
    // 查看选中的目录
    connect(ui->checkSelectedBtn, &QPushButton::clicked, this, &Transcoder::showSelectedDirsDialog);
    connect(ui->action_settings, &QAction::triggered, this, &Transcoder::showSettingsDialog);

    // 主题切换连接 - 如果UI正确生成，取消注释以下行
    // connect(ui->action_modern_theme, &QAction::triggered, this, &Transcoder::switchToModernTheme);
    // connect(ui->action_dark_theme, &QAction::triggered, this, &Transcoder::switchToDarkTheme);

    // 临时添加快捷键支持主题切换
    QShortcut *modernThemeShortcut = new QShortcut(QKeySequence("Ctrl+1"), this);
    QShortcut *darkThemeShortcut = new QShortcut(QKeySequence("Ctrl+2"), this);
    connect(modernThemeShortcut, &QShortcut::activated, this, &Transcoder::switchToModernTheme);
    connect(darkThemeShortcut, &QShortcut::activated, this, &Transcoder::switchToDarkTheme);
}

Transcoder::~Transcoder()
{
    delete ui;
}

void Transcoder::renameFile()
{
    RenameDialog *dialog = new RenameDialog(this);
    Qt::WindowFlags flags = dialog->windowFlags();
    dialog->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);

    //    QString item = ui->lineEdit->text();
    //    ptr->SetValue(item);

    int ref = dialog->exec();
    if (ref == QDialog::Accepted)
    {
        //        QString the_value = ptr->GetValue();
        //        std::cout << "value = " << the_value.toStdString().data() << std::endl;
        //        ui->lineEdit->setText(the_value);
    }

    delete dialog;
}

void Transcoder::startTranscode()
{
    qDebug() << "start transcoding files: " << this->selectedPaths;
    if (targetPath.isEmpty())
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先选择目标保存目录！"));
        return;
    }

    workerThread = new QThread(this);
    worker = new TranscodeTaskManager(this->selectedPaths);
    worker->setTargetDirectory(targetPath);

    ConfigManager *config = ConfigManager::instance();
    const TranscodeSettings &settings = config->getTranscodeSettings();
    worker->setTranscodeSettings(settings);

    worker->moveToThread(workerThread);

    ui->progressBar->setValue(0);
    ui->transcodeBtn->setEnabled(false);
    ui->progressLayout->show();

    connect(worker, &TranscodeTaskManager::progressUpdated, this, &Transcoder::updateProgress);
    connect(worker, &TranscodeTaskManager::finished, this, &Transcoder::onTranscodeFinished);
    connect(worker, &TranscodeTaskManager::currentFileChanged, this, &Transcoder::onCurrentFileChanged);
    connect(worker, &TranscodeTaskManager::fileProcessed, this, &Transcoder::onFileProcessed);
    connect(worker, &TranscodeTaskManager::errorOccurred, this, &Transcoder::onTranscodeError);

    connect(workerThread, &QThread::started, worker, &TranscodeTaskManager::start);
    connect(worker, &TranscodeTaskManager::finished, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();

    QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("转码任务已开始！\n输出目录: %1").arg(targetPath));
}

void Transcoder::selectSourceDirs()
{

    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择转码目录"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    auto *listView = dialog.findChild<QListView *>("listView");
    if (listView)
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
    auto *treeView = dialog.findChild<QTreeView *>("treeView");
    if (treeView)
        treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList tempSelectedPaths = dialog.selectedFiles();
        QMap<QString, QStringList> validFiles = validatePaths(tempSelectedPaths);
        if (validFiles.isEmpty())
        {
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("没有检测到视频文件！"));
            return;
        }

        this->selectedPaths = validFiles;
    }
}

void Transcoder::selectTargetDir()
{
    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择保存目录"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    auto *listView = dialog.findChild<QListView *>("listView");
    if (listView)
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
    auto *treeView = dialog.findChild<QTreeView *>("treeView");
    if (treeView)
        treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList selectedPaths = dialog.selectedFiles();
        this->targetPath = selectedPaths.first();
    }
}

QMap<QString, QStringList> Transcoder::validatePaths(const QStringList &paths)
{
    QMap<QString, QStringList> result;
    for (const QString &path : paths)
    {
        QFileInfo info(path);

        if (!info.isDir())
        {
            continue;
        }

        QDir dir(path);
        QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);
        QStringList names;
        for (const QFileInfo &entry : entries)
        {
            if (this->supportedExtensions.contains(entry.suffix().toLower()))
                names.append(entry.fileName());
        }
        if (names.isEmpty())
        {
            continue;
        }
        result.insert(dir.absolutePath(), names);
    }
    return result;
}

void Transcoder::updateProgress(int value)
{
    ui->progressBar->setValue(value);
}

void Transcoder::onTranscodeFinished()
{
    ui->transcodeBtn->setEnabled(true);
    worker->deleteLater();
    worker = nullptr;

    QMessageBox::information(this, QString::fromLocal8Bit("成功"), QString::fromLocal8Bit("所有视频文件转码成功！"));
}

void Transcoder::onCurrentFileChanged(const QString &fileName)
{
    ui->statusbar->showMessage(QString::fromLocal8Bit("正在转码: %1").arg(fileName));
    qDebug() << QString::fromLocal8Bit("当前处理文件:") << fileName;
}

void Transcoder::onFileProcessed(const QString &fileName, bool success)
{
    if (success)
    {
        qDebug() << QString::fromLocal8Bit("文件转码成功:") << fileName;
    }
    else
    {
        qDebug() << QString::fromLocal8Bit("文件转码失败:") << fileName;
    }
}

void Transcoder::onTranscodeError(const QString &errorMessage)
{
    qDebug() << QString::fromLocal8Bit("转码错误:") << errorMessage;
    QMessageBox::warning(this, QString::fromLocal8Bit("转码错误"), errorMessage);
}

void Transcoder::showSelectedDirsDialog()
{
    auto *dialog = new SelectedDirsDialog(this);
    if (!selectedPaths.isEmpty())
    {
        QStringList dirList;
        for (auto it = selectedPaths.begin(); it != selectedPaths.end(); ++it)
        {
            dirList << QString::fromLocal8Bit("%1 (文件数: %2)").arg(it.key()).arg(it.value().size());
        }
        dialog->setSelectedDirsListView(dirList);
    }

    dialog->exec();
    dialog->deleteLater();
}

void Transcoder::showSettingsDialog()
{
    SettingDialog *dialog = new SettingDialog(this);
    if (dialog->exec() == QDialog::Accepted)
    {
        qDebug() << "转码设置已更新";
    }

    dialog->deleteLater();
}

void Transcoder::switchToModernTheme()
{
    applyTheme(":/styles/modern.qss");
}

void Transcoder::switchToDarkTheme()
{
    applyTheme(":/styles/dark.qss");
}

void Transcoder::applyTheme(const QString &themePath)
{
    QFile qss(themePath);
    if (qss.open(QFile::ReadOnly | QFile::Text))
    {
        qApp->setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }
}
