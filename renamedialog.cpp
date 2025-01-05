#include "renamedialog.h"
#include "ui_renamedialog.h"

RenameDialog::RenameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog)
{
    ui->setupUi(this);
}

RenameDialog::~RenameDialog()
{
    delete ui;
}
