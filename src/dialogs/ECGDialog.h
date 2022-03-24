#ifndef ECGDIALOG_H
#define ECGDIALOG_H

#include "DialogBase.h"
#include "ui_runnabledialog.h"

class ECGDialog : public DialogBase, public Ui::RunnableDialog
{
    Q_OBJECT

public:
    ECGDialog(QWidget *parent = Q_NULLPTR);
    ~ECGDialog();

    QString getVerificationBarcode() const override;
    void setVerificationBarcode(const QString&) override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::RunnableDialog *ui { Q_NULLPTR };
};

#endif // ECGDIALOG_H
