#ifndef WEIGHSCALEDIALOG_H
#define WEIGHSCALEDIALOG_H

#include "DialogBase.h"
#include "ui_weighscaledialog.h"

class WeighScaleDialog : public DialogBase, public Ui::WeighScaleDialog
{
    Q_OBJECT

public:
    WeighScaleDialog(QWidget *parent = Q_NULLPTR);
    ~WeighScaleDialog();

    QString getVerificationBarcode() const override;
    void setVerificationBarcode(const QString&) override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::WeighScaleDialog *ui { Q_NULLPTR };
};

#endif // WEIGHSCALEDIALOG_H
