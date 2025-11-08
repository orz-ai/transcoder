
#include "transcoder.h"
#include "ui_transcoder.h"
#include <RenameDialog.h>
#include <selecteddirsdialog.h>
#include <settingdialog.h>
#include <qdebug.h>
#include <utils/HttpClient.h>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QShortcut>

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
    if (this->selecedPaths.length() == 0) {
        QMessageBox::warning(nullptr, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("请先选择转码目录！"));
		return;
    }

    transcoding(this->selecedPaths);
}

void Transcoder::selectSourceDirs()
{

    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择转码目录"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    QListView *listView = dialog.findChild<QListView *>("listView");
    if (listView)
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *treeView = dialog.findChild<QTreeView *>("treeView");
    if (treeView)
        treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList selectedPaths = dialog.selectedFiles();
        qDebug() << "选择了：" << selectedPaths;

        QStringList validFiles = validatePaths(selectedPaths);
        if (validFiles.isEmpty())
        {
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("没有检测到视频文件！"));
            return;
        }

        this->selecedPaths = validFiles;
    }
}

void Transcoder::selectTargetDir()
{
    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择保存目录"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    QListView *listView = dialog.findChild<QListView *>("listView");
    if (listView)
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *treeView = dialog.findChild<QTreeView *>("treeView");
    if (treeView)
        treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted)
    {
        QStringList selectedPaths = dialog.selectedFiles();
        this->targetPath = selectedPaths.first();
    }
}

void Transcoder::transcoding(const QStringList &files)
{
    qDebug() << "start transcoding files: " << files;
    worker = new TranscodeWorker(files, this);
    ui->progressBar->setValue(0);
    ui->transcodeBtn->setEnabled(false);
    ui->progressLayout->show();

    connect(worker, &TranscodeWorker::progressUpdated, this, &Transcoder::updateProgress);
    connect(worker, &TranscodeWorker::finished, this, &Transcoder::onTranscodeFinished);

    worker->start();
    QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("转码任务已开始！"));
}

bool Transcoder::isValidFile(const QFileInfo &fileInfo)
{
    QStringList supportedExtensions = {"mp4", "mkv", "avi", "mov"};
    return supportedExtensions.contains(fileInfo.suffix().toLower());
}

QStringList Transcoder::validatePaths(const QStringList &paths)
{
    QStringList validFiles;
    for (const QString &path : paths)
    {
        QFileInfo info(path);

        if (info.isDir())
        {

            QDir dir(path);
            if (dir.isEmpty())
            {
                qDebug() << "skip empty dir." << path;
                continue;
            }
            validFiles.append(path);
        }
        else if (info.isFile())
        {

            if (isValidFile(info))
            {
                validFiles.append(path);
            }
            else
            {
                qDebug() << "invalid file: " << path;
            }
        }
        else
        {
            qDebug() << "unknown dir." << path;
        }
    }
    return validFiles;
}

QString Transcoder::buildTranscodeCommand(QString srcPath, QString targetPath)
{
    QString cmd = QString("ffmpeg -i %1 -c:v libx264 -s 720x1280 -r 30 -crf %2 -pix_fmt yuv420p -colorspace bt709 -color_primaries bt709 -color_trc bt709 -color_range tv -movflags faststart -profile:v high %3")
                      .arg(srcPath)
                      .arg(23) // CRF 值
                      .arg(targetPath);

    return cmd;
}


/**
 * 显示当前选中的所有目录
 */
void Transcoder::showSelectedDirsDialog()
{
    SelectedDirsDialog *dialog = new SelectedDirsDialog();
    Qt::WindowFlags flags = dialog->windowFlags();
    dialog->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);

    dialog->setSelectedDirsListView(this->selecedPaths);

    int ref = dialog->exec();
    if (ref == QDialog::Accepted)
    {

    }

    delete dialog;
    return;
}

void Transcoder::showSettingsDialog()
{
    SettingDialog *dialog = new SettingDialog;
    Qt::WindowFlags flags = dialog->windowFlags();
    dialog->setWindowFlags(flags);
    int ref = dialog->exec();
    if (ref == QDialog::Accepted)
    {

    }

    delete dialog;
    return;
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
