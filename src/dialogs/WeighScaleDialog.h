#ifndef WEIGHSCALEDIALOG_H
#define WEIGHSCALEDIALOG_H

#include "CypressDialog.h"
#include "ui_weighscaledialog.h"

QT_FORWARD_DECLARE_CLASS(WeighScaleManager)

class WeighScaleDialog : public CypressDialog, public Ui::WeighScaleDialog
{
    Q_OBJECT

public:
    WeighScaleDialog(QWidget *parent = Q_NULLPTR);
    ~WeighScaleDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::WeighScaleDialog *ui { Q_NULLPTR };
    WeighScaleManager* m_child { Q_NULLPTR };
};

#endif // WEIGHSCALEDIALOG_H
