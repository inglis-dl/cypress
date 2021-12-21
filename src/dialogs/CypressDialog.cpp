#include "CypressDialog.h"
#include "ui_CypressDialog.h"

CypressDialog::CypressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CypressDialog)
{
    ui->setupUi(this);
}

CypressDialog::~CypressDialog()
{
    delete ui;
}
