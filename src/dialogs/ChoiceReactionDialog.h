#ifndef CHOICEREACTIONDIALOG_H
#define CHOICEREACTIONDIALOG_H

#include "DialogBase.h"
#include "ui_runnabledialog.h"

class ChoiceReactionDialog : public DialogBase, public Ui::RunnableDialog
{
    Q_OBJECT

public:
    ChoiceReactionDialog(QWidget* parent = Q_NULLPTR);
    ~ChoiceReactionDialog();

    QString getVerificationBarcode() const override;
    void setVerificationBarcode(const QString&) override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::RunnableDialog *ui { Q_NULLPTR };
};

#endif // CHOICEREACTIONDIALOG_H
