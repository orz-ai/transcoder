#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = nullptr);
    ~RenameDialog();

    enum RenameMode {
        PREFIX,
        SUFFIX,
        ARBITRARY
    };

    // 获取当前选择的重命名模式
    RenameMode getCurrentMode() const { return currentMode; }

    // 获取用户输入的内容
    QString getContent() const;

    // 获取选择的目录路径
    QString getDirPath() const;

private slots:
    void onPushButtonClicked();
    void onCancelBtnClicked();
    void onConfirmBtnClicked();
    void onPrefixRadioButtonToggled(bool checked);
    void onSuffixRadioButtonToggled(bool checked);
    void onArbitraryRadioButtonToggled(bool checked);

private:
    Ui::RenameDialog *ui;

    RenameMode currentMode;
};

#endif // RENAMEDIALOG_H
