#include "renamedialog.h"
#include "ui_renamedialog.h"
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

RenameDialog::RenameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog),
    currentMode(PREFIX)
{
    ui->setupUi(this);

    // 手动连接信号槽
    connect(ui->pushButton, &QPushButton::clicked, this, &RenameDialog::onPushButtonClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &RenameDialog::onCancelBtnClicked);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &RenameDialog::onConfirmBtnClicked);
    connect(ui->prefixRadioButton, &QRadioButton::toggled, this, &RenameDialog::onPrefixRadioButtonToggled);
    connect(ui->suffixRadioButton, &QRadioButton::toggled, this, &RenameDialog::onSuffixRadioButtonToggled);
    connect(ui->arbitraryRadioButton, &QRadioButton::toggled, this, &RenameDialog::onArbitraryRadioButtonToggled);
}

RenameDialog::~RenameDialog()
{
    delete ui;
}

void RenameDialog::onPushButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择目录"),
                                                   QDir::homePath());
    if (!dir.isEmpty()) {
        ui->dirInput->setText(dir);
    }
}

void RenameDialog::onCancelBtnClicked()
{
    this->reject();
}

void RenameDialog::onConfirmBtnClicked()
{
    QString dirPath = getDirPath();
    QString content = getContent();

    if (dirPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"),
                           QString::fromLocal8Bit("请选择目录"));
        return;
    }

    if (content.isEmpty()) {
        QMessageBox::warning(this, QString::fromLocal8Bit("警告"),
                           QString::fromLocal8Bit("请输入重命名内容"));
        return;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        QMessageBox::critical(this, QString::fromLocal8Bit("错误"),
                            QString::fromLocal8Bit("目录不存在"));
        return;
    }

    QStringList filters;
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);

    if (fileList.isEmpty()) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
                                QString::fromLocal8Bit("目录中没有文件"));
        return;
    }

    int successCount = 0;
    int failCount = 0;
    QStringList failedFiles;

    for (const QFileInfo &fileInfo : fileList) {
        QString oldFileName = fileInfo.fileName();
        QString newFileName;

        switch (currentMode) {
            case PREFIX: {
                if (!oldFileName.startsWith(content)) {
                    continue;
                }

                newFileName = oldFileName.mid(content.length());
                if (newFileName.isEmpty()) {
                    continue;
                }
                break;
            }
            case SUFFIX: {
                QString baseName = fileInfo.baseName();
                QString suffix = fileInfo.completeSuffix();
                if (!baseName.endsWith(content)) {
                    continue;
                }

                QString newBaseName = baseName.left(baseName.length() - content.length());
                if (newBaseName.isEmpty()) {
                    continue;
                }
                newFileName = newBaseName + "." + suffix;
                break;
            }
            case ARBITRARY: {
                QString baseName = fileInfo.baseName();
                QString suffix = fileInfo.completeSuffix();

                QString newBaseName = baseName;
                newBaseName.remove(content);
                if (newBaseName.isEmpty()) {
                    continue;
                }

                if (suffix.isEmpty()) {
                    newFileName = newBaseName;
                } else {
                    newFileName = newBaseName + "." + suffix;
                }
                break;
            }
        }

        QString oldFilePath = fileInfo.absoluteFilePath();
        QString newFilePath = dir.absoluteFilePath(newFileName);

        if (QFile::exists(newFilePath) && oldFilePath != newFilePath) {
            qDebug() << QString::fromLocal8Bit("文件已存在，跳过：") << newFileName;
            failedFiles.append(oldFileName + QString::fromLocal8Bit(" -> 目标文件已存在"));
            failCount++;
            continue;
        }

        if (QFile::rename(oldFilePath, newFilePath)) {
            qDebug() << QString::fromLocal8Bit("重命名成功：") << oldFileName << " -> " << newFileName;
            successCount++;
        } else {
            qDebug() << QString::fromLocal8Bit("重命名失败：") << oldFileName;
            failedFiles.append(oldFileName + QString::fromLocal8Bit(" -> 重命名失败"));
            failCount++;
        }
    }

    // 显示结果
    QString resultMessage = QString::fromLocal8Bit("重命名完成！\n成功：%1 个文件\n失败：%2 个文件")
                          .arg(successCount).arg(failCount);

    if (!failedFiles.isEmpty()) {
        resultMessage += QString::fromLocal8Bit("\n\n失败的文件：\n") + failedFiles.join("\n");
    }

    if (failCount > 0) {
        QMessageBox::warning(this, QString::fromLocal8Bit("重命名结果"), resultMessage);
    } else {
        QMessageBox::information(this, QString::fromLocal8Bit("重命名结果"), resultMessage);
    }
}

void RenameDialog::onPrefixRadioButtonToggled(bool checked)
{
    if (checked) {
        currentMode = PREFIX;
        ui->label_2->setText(QString::fromLocal8Bit("要删除的前缀："));
    }
}

void RenameDialog::onSuffixRadioButtonToggled(bool checked)
{
    if (checked) {
        currentMode = SUFFIX;
        ui->label_2->setText(QString::fromLocal8Bit("要删除的后缀："));
    }
}

void RenameDialog::onArbitraryRadioButtonToggled(bool checked)
{
    if (checked) {
        currentMode = ARBITRARY;
        ui->label_2->setText(QString::fromLocal8Bit("要删除的内容："));
    }
}

QString RenameDialog::getContent() const
{
    return ui->contentInput->text();
}

QString RenameDialog::getDirPath() const
{
    return ui->dirInput->text();
}

