
#include "transcoder.h"
#include "ui_transcoder.h"
#include <RenameDialog.h>
#include <qdebug.h>
#include <utils/HttpClient.h>
#include <QMessageBox>


Transcoder::Transcoder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Transcoder)
{
    ui->setupUi(this);


    ui->progressLayout->hide();
    ui->progressBar->setValue(0);

    connect(ui->renameFileBtn, &QPushButton::clicked, this, &Transcoder::renameFile);
    // 开始转码
    connect(ui->transcodeBtn, &QPushButton::clicked, this, &Transcoder::startTranscode);
    // 选择需要转码的目录
    connect(ui->sourceDirBtn, &QPushButton::clicked, this, &Transcoder::selectSourceDirs);
    // 选择转码后文件存放的目录
    connect(ui->targetDirButton, &QPushButton::clicked, this, &Transcoder::selectTargetDir);
}

Transcoder::~Transcoder()
{
    delete ui;
}

void Transcoder::renameFile()
{
    RenameDialog *dialog = new RenameDialog(this);
    Qt::WindowFlags flags = dialog->windowFlags();                     // 需要获取返回值
    dialog->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);  // 设置对话框固定大小

//    QString item = ui->lineEdit->text();
//    ptr->SetValue(item);

    int ref = dialog->exec();             // 以模态方式显示对话框
    if (ref==QDialog::Accepted)        // OK键被按下,对话框关闭
    {
        // 当BtnOk被按下时,则设置对话框中的数据
//        QString the_value = ptr->GetValue();
//        std::cout << "value = " << the_value.toStdString().data() << std::endl;
//        ui->lineEdit->setText(the_value);
    }

    // 删除释放对话框句柄
    delete dialog;
}

void Transcoder::startTranscode()
{
//    HttpClient client;
//    QString response = client.get("https://orz.ai/dailynews/?platform=baidu");
//    qDebug() << "Response:" << response;
    QFileDialog dialog(nullptr, QString::fromLocal8Bit("select dirs or files"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    // 添加目录选择功能
    QListView *listView = dialog.findChild<QListView*>("listView");
    if (listView) listView->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *treeView = dialog.findChild<QTreeView*>("treeView");
    if (treeView) treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedPaths = dialog.selectedFiles();
        qDebug() << "selected paths: " << selectedPaths;

        // check dirs and files
        QStringList validFiles = validatePaths(selectedPaths);
        if (validFiles.isEmpty()) {
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("ERROR"), QString::fromLocal8Bit("no effect files!"));
            return;
        }


        transcoding(validFiles);
    }

}

void Transcoder::selectSourceDirs()
{

    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择需要转码的目录(支持多选)"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    // 添加目录选择功能
    QListView *listView = dialog.findChild<QListView*>("listView");
    if (listView) listView->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *treeView = dialog.findChild<QTreeView*>("treeView");
    if (treeView) treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedPaths = dialog.selectedFiles();
        qDebug() << "selected paths: " << selectedPaths;

        // check dirs and files
        QStringList validFiles = validatePaths(selectedPaths);
        if (validFiles.isEmpty()) {
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("ERROR"), QString::fromLocal8Bit("no effect files!"));
            return;
        }


        transcoding(validFiles);
    }

}

void Transcoder::selectTargetDir()
{
    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择转码结果保存目录"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    QListView *listView = dialog.findChild<QListView*>("listView");
    if (listView) listView->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *treeView = dialog.findChild<QTreeView*>("treeView");
    if (treeView) treeView->setSelectionMode(QAbstractItemView::MultiSelection);

    if (dialog.exec() == QDialog::Accepted) {

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
    QMessageBox::information(nullptr, QString::fromLocal8Bit("成功"), QString::fromLocal8Bit("task is proccessing!"));
}

bool Transcoder::isValidFile(const QFileInfo &fileInfo) {
    // 支持的后缀
    QStringList supportedExtensions = {"mp4", "mkv", "avi", "mov"};
    return supportedExtensions.contains(fileInfo.suffix().toLower());
}

QStringList Transcoder::validatePaths(const QStringList &paths)
{
    QStringList validFiles;
    for (const QString &path : paths) {
        QFileInfo info(path);

        if (info.isDir()) {

            QDir dir(path);
            if (dir.isEmpty()) {
                qDebug() << "skip empty dir." << path;
                continue;
            }
            validFiles.append(path);
        } else if (info.isFile()) {

            if (isValidFile(info)) {
                validFiles.append(path);
            } else {
                qDebug() << "invalid file: " << path;
            }
        } else {
            qDebug() << "unknown dir." << path;
        }
    }
    return validFiles;
}

void Transcoder::updateProgress(int value) {
    ui->progressBar->setValue(value);
}

void Transcoder::onTranscodeFinished() {

    ui->transcodeBtn->setEnabled(true);
    worker->deleteLater();
    worker = nullptr;

    QMessageBox::information(this, "Success", "All files transcode completely!");
}


