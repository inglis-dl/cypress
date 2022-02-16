#ifndef AUDIOMETERDIALOG_H
#define AUDIOMETERDIALOG_H

#include "DialogBase.h"
#include "ui_audiometerdialog.h"

class AudiometerDialog : public DialogBase, public Ui::AudiometerDialog
{
    Q_OBJECT

public:
    AudiometerDialog(QWidget *parent = Q_NULLPTR);
    ~AudiometerDialog();

    QString getVerificationBarcode() const override;

private:
    void initializeModel() override;
    void initializeConnections() override;

    Ui::AudiometerDialog *ui { Q_NULLPTR };
};

#endif // AUDIOMETERDIALOG_H
