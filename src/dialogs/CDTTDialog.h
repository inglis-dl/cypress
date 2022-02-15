#ifndef CDTTDIALOG_H
#define CDTTDIALOG_H

#include "CypressDialog.h"
#include "ui_cdttdialog.h"

QT_FORWARD_DECLARE_CLASS(CDTTManager)

class CDTTDialog : public CypressDialog, public Ui::CDTTDialog
{
    Q_OBJECT

public:
    CDTTDialog(QWidget *parent = Q_NULLPTR);
    ~CDTTDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::CDTTDialog *ui { Q_NULLPTR };
    CDTTManager *m_child { Q_NULLPTR };
};

#endif // CDTTDIALOG_H
