#ifndef CDTTDIALOG_H
#define CDTTDIALOG_H

#include "DialogBase.h"
#include "ui_runnabledialog.h"

class CDTTDialog : public DialogBase, public Ui::RunnableDialog
{
    Q_OBJECT

public:
    CDTTDialog(QWidget *parent = Q_NULLPTR);
    ~CDTTDialog();

    QString getVerificationBarcode() const override;
    void setVerificationBarcode(const QString&) override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::RunnableDialog *ui { Q_NULLPTR };
};

#endif // CDTTDIALOG_H
