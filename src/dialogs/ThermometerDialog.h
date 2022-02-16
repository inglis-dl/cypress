#ifndef THERMOMETERDIALOG_H
#define THERMOMETERDIALOG_H

#include "DialogBase.h"
#include "ui_thermometerdialog.h"

QT_FORWARD_DECLARE_CLASS(BluetoothLEManager)

class ThermometerDialog : public DialogBase, public Ui::ThermometerDialog
{
    Q_OBJECT

public:
    ThermometerDialog(QWidget *parent = Q_NULLPTR);
    ~ThermometerDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::ThermometerDialog *ui { Q_NULLPTR };
    BluetoothLEManager *m_child { Q_NULLPTR };
};

#endif // THERMOMETERDIALOG_H
