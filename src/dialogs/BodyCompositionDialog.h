#ifndef BODYCOMPOSITIONDIALOG_H
#define BODYCOMPOSITIONDIALOG_H

#include "DialogBase.h"
#include "ui_bodycompositiondialog.h"

QT_FORWARD_DECLARE_CLASS(BodyCompositionAnalyzerManager)

class BodyCompositionDialog : public DialogBase, public Ui::BodyCompositionDialog
{
    Q_OBJECT

public:
    BodyCompositionDialog(QWidget *parent = Q_NULLPTR);
    ~BodyCompositionDialog();
 
    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::BodyCompositionDialog *ui { Q_NULLPTR };
    BodyCompositionAnalyzerManager *m_child { Q_NULLPTR };
};

#endif // BODYCOMPOSITIONDIALOG_H
