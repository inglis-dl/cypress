#ifndef CDTTDIALOG_H
#define CDTTDIALOG_H

#include "DialogBase.h"
#include "ui_runnabledialog.h"

QT_FORWARD_DECLARE_CLASS(CDTTManager)

class CDTTDialog : public DialogBase, public Ui::RunnableDialog
{
    Q_OBJECT

public:
    CDTTDialog(QWidget *parent = Q_NULLPTR);
    ~CDTTDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::RunnableDialog *ui { Q_NULLPTR };
    CDTTManager *m_child { Q_NULLPTR };
};

#endif // CDTTDIALOG_H
