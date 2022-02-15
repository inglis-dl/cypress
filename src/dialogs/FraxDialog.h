#ifndef FRAXDIALOG_H
#define FRAXDIALOG_H

#include "CypressDialog.h"
#include "ui_fraxdialog.h"

QT_FORWARD_DECLARE_CLASS(FraxManager)

class FraxDialog : public CypressDialog, public Ui::FraxDialog
{
    Q_OBJECT

public:
    FraxDialog(QWidget *parent = Q_NULLPTR);
    ~FraxDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::FraxDialog *ui = { Q_NULLPTR };
    FraxManager *m_child = { Q_NULLPTR };
};

#endif // FRAXDIALOG_H
