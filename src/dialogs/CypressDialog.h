#ifndef CYPRESSDIALOG_H
#define CYPRESSDIALOG_H

#include <QDialog>

namespace Ui {
class CypressDialog;
}

class CypressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CypressDialog(QWidget *parent = nullptr);
    ~CypressDialog();

private:
    Ui::CypressDialog *ui;

    friend class CypressApplication;
};

#endif // CYPRESSDIALOG_H
