#ifndef THERMOMETERDIALOG_H
#define THERMOMETERDIALOG_H

#include "DialogBase.h"
#include "ui_thermometerdialog.h"

class ThermometerDialog : public DialogBase, public Ui::ThermometerDialog
{
    Q_OBJECT

public:
    ThermometerDialog(QWidget *parent = Q_NULLPTR);
    ~ThermometerDialog();

    QString getVerificationBarcode() const override;
    void setVerificationBarcode(const QString&) override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::ThermometerDialog *ui { Q_NULLPTR };
};

#endif // THERMOMETERDIALOG_H
