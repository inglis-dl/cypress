#ifndef CHOICEREACTIONDIALOG_H
#define CHOICEREACTIONDIALOG_H

#include "CypressDialog.h"
#include "ui_choicereactiondialog.h"

QT_FORWARD_DECLARE_CLASS(ChoiceReactionManager)

class ChoiceReactionDialog : public CypressDialog, public Ui::ChoiceReactionDialog
{
    Q_OBJECT

public:
    ChoiceReactionDialog(QWidget* parent = Q_NULLPTR);
    ~ChoiceReactionDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::ChoiceReactionDialog *ui { Q_NULLPTR };
    ChoiceReactionManager *m_child { Q_NULLPTR };
};

#endif // CHOICEREACTIONDIALOG_H
