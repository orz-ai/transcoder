
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
#include <QRegularExpression>
#include <QHeaderView>
#include <QDir>

Transcoder::Transcoder(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Transcoder)
{
    ui->setupUi(this);

    // 设置应用程序图标
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    // 初始化转码模型
    transcodeModel = new TranscodeModel(this);

    // 初始化代理模型用于筛选
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(transcodeModel);
    proxyModel->setFilterKeyColumn(1); // 按状态列筛选（序号列后状态列变为索引1）

    ui->tableView->setModel(proxyModel);

    // 设置表格属性
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setAlternatingRowColors(true);

    // 设置header高度
    ui->tableView->horizontalHeader()->setMaximumHeight(40);
    ui->tableView->horizontalHeader()->setMinimumHeight(30);

    // 设置各列宽度
    ui->tableView->setColumnWidth(0, 60);  // 序号列
    ui->tableView->setColumnWidth(1, 100); // 状态列
    ui->tableView->setColumnWidth(2, 450); // 源路径列 - 加宽
    ui->tableView->setColumnWidth(3, 450); // 目标路径列 - 加宽
    ui->tableView->setColumnWidth(4, 60);  // 进度列 - 适中

    // 设置列的调整策略 - 允许手动调整
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // 序号列固定
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed); // 进度列固定    ui->progressLayout->hide();
    ui->progressBar->setValue(0);

    connect(ui->renameFileBtn, &QPushButton::clicked, this, &Transcoder::renameFile);
    connect(ui->videoInfoBtn, &QPushButton::clicked, this, &Transcoder::showVideoInfoDialog);
    // 选择
    connect(ui->transcodeBtn, &QPushButton::clicked, this, &Transcoder::startTranscode);
    // 选择转码目录
    connect(ui->sourceDirBtn, &QPushButton::clicked, this, &Transcoder::selectSourceDirs);
    // 选择转码结果保存目录
    connect(ui->targetDirButton, &QPushButton::clicked, this, &Transcoder::selectTargetDir);
    // 查看选中的目录
    connect(ui->checkSelectedBtn, &QPushButton::clicked, this, &Transcoder::showSelectedDirsDialog);
    connect(ui->action_settings, &QAction::triggered, this, &Transcoder::showSettingsDialog);

    // 连接筛选器
    connect(ui->statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Transcoder::onFilterStatusChanged);

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

    // 在开始转码前再次更新已存在文件的状态
    updateExistingFilesStatus();

    workerThread = new QThread(this);
    worker = new TranscodeTaskManager(this->selectedPaths);
    worker->setTargetDirectory(targetPath);

    ConfigManager *config = ConfigManager::instance();
    const TranscodeSettings &settings = config->getTranscodeSettings();
    worker->setTranscodeSettings(settings);

    worker->moveToThread(workerThread);

    ui->progressBar->setValue(0);
    ui->transcodeBtn->setText(QString::fromLocal8Bit("停止转码"));
    ui->transcodeBtn->disconnect(); // 断开所有连接
    connect(ui->transcodeBtn, &QPushButton::clicked, this, &Transcoder::stopTranscode);
    ui->progressLayout->show();

    connect(worker, &TranscodeTaskManager::progressUpdated, this, &Transcoder::updateProgress, Qt::QueuedConnection);
    connect(worker, &TranscodeTaskManager::finished, this, &Transcoder::onTranscodeFinished, Qt::QueuedConnection);
    connect(worker, &TranscodeTaskManager::currentFileChanged, this, &Transcoder::onCurrentFileChanged, Qt::QueuedConnection);
    connect(worker, &TranscodeTaskManager::fileProcessed, this, &Transcoder::onFileProcessed, Qt::QueuedConnection);
    connect(worker, &TranscodeTaskManager::errorOccurred, this, &Transcoder::onTranscodeError, Qt::QueuedConnection);

    connect(workerThread, &QThread::started, worker, &TranscodeTaskManager::start);
    connect(worker, &TranscodeTaskManager::finished, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    qDebug() << QString::fromLocal8Bit("准备启动转码线程...");
    workerThread->start();
    qDebug() << QString::fromLocal8Bit("转码线程已启动");

    QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("转码任务已开始！\n输出目录: %1").arg(targetPath));
}

void Transcoder::stopTranscode()
{
    if (worker && workerThread && workerThread->isRunning())
    {
        qDebug() << QString::fromLocal8Bit("用户请求停止转码");

        // 停止转码任务
        worker->stop();

        // 重新设置按钮状态
        ui->transcodeBtn->setText(QString::fromLocal8Bit("开始转码"));
        ui->transcodeBtn->disconnect();                                                      // 断开停止连接
        connect(ui->transcodeBtn, &QPushButton::clicked, this, &Transcoder::startTranscode); // 重新连接开始转码
        ui->progressLayout->hide();

        QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("转码任务已停止"));
    }
}

void Transcoder::selectSourceDirs()
{
    QFileDialog dialog(nullptr, QString::fromLocal8Bit("选择转码目录"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setOption(QFileDialog::ShowDirsOnly, true); // 只显示目录
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

        // 立即加载文件到表格
        loadFilesToTable();
    }
}

void Transcoder::selectTargetDir()
{
    QString selectedDir = QFileDialog::getExistingDirectory(
        this,
        QString::fromLocal8Bit("选择保存目录"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!selectedDir.isEmpty())
    {
        this->targetPath = selectedDir;
        qDebug() << QString::fromLocal8Bit("选中的目标目录:") << this->targetPath;

        // 更新表格中已存在文件的状态
        updateExistingFilesStatus();
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
    qDebug() << QString::fromLocal8Bit("转码任务完成，重新启用界面");
    ui->transcodeBtn->setText(QString::fromLocal8Bit("开始转码"));
    ui->transcodeBtn->disconnect();                                                      // 断开停止连接
    connect(ui->transcodeBtn, &QPushButton::clicked, this, &Transcoder::startTranscode); // 重新连接开始转码
    ui->progressLayout->hide();

    if (worker)
    {
        worker->deleteLater();
        worker = nullptr;
    }

    QMessageBox::information(this, QString::fromLocal8Bit("成功"), QString::fromLocal8Bit("所有视频文件转码完成！"));
}

void Transcoder::onCurrentFileChanged(const QString &fileName)
{
    qDebug() << QString::fromLocal8Bit("当前处理文件:") << fileName;
    transcodeModel->updateRecordStatus(fileName, TranscodeStatus::Processing);
}

void Transcoder::onFileProcessed(const QString &fileName, bool success)
{
    if (success)
    {
        qDebug() << QString::fromLocal8Bit("文件转码成功:") << fileName;
        transcodeModel->updateRecordStatus(fileName, TranscodeStatus::Success);
    }
    else
    {
        qDebug() << QString::fromLocal8Bit("文件转码失败:") << fileName;
        transcodeModel->updateRecordStatus(fileName, TranscodeStatus::Failed, QString::fromLocal8Bit("转码失败"));
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
        qDebug() << QString::fromLocal8Bit("转码设置已更新");
    }

    dialog->deleteLater();
}

void Transcoder::showVideoInfoDialog()
{
    VideoInfoDialog *dialog = new VideoInfoDialog(this);
    dialog->exec();
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

void Transcoder::onFilterStatusChanged()
{
    int currentIndex = ui->statusFilterCombo->currentIndex();
    QString filterText;

    switch (currentIndex)
    {
    case 0: // 全部
        filterText = "";
        break;
    case 1: // 等待中
        filterText = QString::fromLocal8Bit("等待中");
        break;
    case 2: // 转码中
        filterText = QString::fromLocal8Bit("转码中");
        break;
    case 3: // 成功
        filterText = QString::fromLocal8Bit("成功");
        break;
    case 4: // 失败
        filterText = QString::fromLocal8Bit("失败");
        break;
    }

    if (filterText.isEmpty())
    {
        proxyModel->setFilterRegularExpression(QRegularExpression());
    }
    else
    {
        proxyModel->setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(filterText)));
    }
}

void Transcoder::loadFilesToTable()
{
    if (selectedPaths.isEmpty())
    {
        return;
    }

    // 清空之前的记录
    transcodeModel->clearRecords();

    // 添加所有文件到模型
    for (auto it = selectedPaths.begin(); it != selectedPaths.end(); ++it)
    {
        const QString &dirPath = it.key();
        const QStringList &files = it.value();
        for (const QString &file : files)
        {
            QString sourcePath = QDir(dirPath).absoluteFilePath(file);
            QString targetFilePath; // 目标路径稍后设置
            if (!targetPath.isEmpty())
            {
                targetFilePath = QDir(targetPath).absoluteFilePath(file);
            }
            transcodeModel->addRecord(file, sourcePath, targetFilePath);
        }
    }

    // 如果目标路径已经设置，则更新已存在文件的状态
    if (!targetPath.isEmpty())
    {
        updateExistingFilesStatus();
    }
}

void Transcoder::updateExistingFilesStatus()
{
    if (targetPath.isEmpty())
    {
        qDebug() << QString::fromLocal8Bit("目标路径为空，跳过更新");
        return;
    }

    // 更新所有记录的目标路径并检查文件是否已存在
    int rowCount = transcodeModel->rowCount();
    qDebug() << QString::fromLocal8Bit("准备更新文件状态，表格行数：") << rowCount;

    for (int i = 0; i < rowCount; ++i)
    {
        QString sourcePath = transcodeModel->data(transcodeModel->index(i, TranscodeModel::SourcePath), Qt::DisplayRole).toString();

        // 从源路径中提取文件名
        QFileInfo sourceFileInfo(sourcePath);
        QString fileName = sourceFileInfo.fileName();

        qDebug() << QString::fromLocal8Bit("处理文件：") << fileName << QString::fromLocal8Bit("，源路径：") << sourcePath;

        // 获取源文件所在目录名（剧集名）
        QString sourceDir = sourceFileInfo.absolutePath();
        QDir sourceQDir(sourceDir);
        QString dramaName = sourceQDir.dirName(); // 获取最后一级目录名

        // 构建最终输出路径，与TranscodeTaskManager::start()中的逻辑一致
        QString dramaTargetDir = QDir(targetPath).absoluteFilePath(dramaName);
        QString baseName = sourceFileInfo.baseName();
        QString finalOutputName = baseName + ".mp4";
        QString finalOutputPath = QDir(dramaTargetDir).absoluteFilePath(finalOutputName);

        qDebug() << QString::fromLocal8Bit("最终输出路径：") << finalOutputPath;

        // 更新目标路径
        transcodeModel->setData(transcodeModel->index(i, TranscodeModel::TargetPath), finalOutputPath, Qt::EditRole);

        // 检查目标文件是否已存在，如果存在则标记为成功
        QFileInfo targetFileInfo(finalOutputPath);
        if (targetFileInfo.exists() && targetFileInfo.isFile())
        {
            qDebug() << QString::fromLocal8Bit("文件已存在，标记为成功：") << finalOutputPath;
            transcodeModel->updateRecordStatus(fileName, TranscodeStatus::Success);
        }
        else
        {
            qDebug() << QString::fromLocal8Bit("文件不存在，标记为等待：") << finalOutputPath;
            // 如果文件不存在，确保状态为等待
            transcodeModel->updateRecordStatus(fileName, TranscodeStatus::Pending);
        }
    }
}
