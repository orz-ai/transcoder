#ifndef SELECTEDDIRSDIALOG_H
#define SELECTEDDIRSDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>

namespace Ui {
class SelectedDirsDialog;
}

class SelectedDirsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectedDirsDialog(QWidget *parent = nullptr);
    ~SelectedDirsDialog();

    void setSelectedDirsListView(QStringList const &selectedDirs);

private:
    Ui::SelectedDirsDialog *ui;
};

#endif // SELECTEDDIRSDIALOG_H
