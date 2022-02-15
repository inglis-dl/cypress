#ifndef AUDIOMETERDIALOG_H
#define AUDIOMETERDIALOG_H

#include "CypressDialog.h"
#include "ui_audiometerdialog.h"

QT_FORWARD_DECLARE_CLASS(AudiometerManager)

class AudiometerDialog : public CypressDialog, public Ui::AudiometerDialog
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
    AudiometerManager* m_child { Q_NULLPTR };
};

#endif // AUDIOMETERDIALOG_H
