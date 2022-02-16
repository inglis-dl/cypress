#ifndef FRAXDIALOG_H
#define FRAXDIALOG_H

#include "DialogBase.h"
#include "ui_runnabledialog.h"

class FraxDialog : public DialogBase, public Ui::RunnableDialog
{
    Q_OBJECT

public:
    FraxDialog(QWidget *parent = Q_NULLPTR);
    ~FraxDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::RunnableDialog *ui { Q_NULLPTR };
};

#endif // FRAXDIALOG_H
