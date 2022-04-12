#ifndef SPIROMETERDIALOG_H
#define SPIROMETERDIALOG_H

#include "DialogBase.h"
#include "ui_runnabledialog.h"

class SpirometerDialog : public DialogBase, public Ui::RunnableDialog
{
    Q_OBJECT

public:
    SpirometerDialog(QWidget *parent = Q_NULLPTR);
    ~SpirometerDialog();

    QString getVerificationBarcode() const override;
    void setVerificationBarcode(const QString&) override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::RunnableDialog *ui { Q_NULLPTR };
};

#endif // SPIROMETERDIALOG_H
