#include "selecteddirsdialog.h"
#include "ui_selecteddirsdialog.h"

SelectedDirsDialog::SelectedDirsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectedDirsDialog)
{
    ui->setupUi(this);
}

SelectedDirsDialog::~SelectedDirsDialog()
{
    delete ui;
}


void SelectedDirsDialog::setSelectedDirsListView(QStringList const &selectedDirs)
{
    auto itemsModel = new QStandardItemModel(this);
    for (const auto &dir: selectedDirs)
    {
        auto *item = new QStandardItem(dir);
        item->setEditable(false);
        itemsModel->appendRow(item);
    }

    ui->listView->setModel(itemsModel);
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listView->setUniformItemSizes(true);
}
