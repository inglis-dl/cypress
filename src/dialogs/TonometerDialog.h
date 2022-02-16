#ifndef TONOMETERDIALOG_H
#define TONOMETERDIALOG_H

#include "DialogBase.h"
#include "ui_tonometerdialog.h"

QT_FORWARD_DECLARE_CLASS(TonometerManager)

class TonometerDialog : public DialogBase, public Ui::TonometerDialog
{
    Q_OBJECT

public:
    TonometerDialog(QWidget *parent = Q_NULLPTR);
    ~TonometerDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::TonometerDialog *ui { Q_NULLPTR };
    TonometerManager *m_child { Q_NULLPTR };
};

#endif // TONOMETERDIALOG_H
