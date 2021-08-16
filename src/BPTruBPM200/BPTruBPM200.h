#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BPTruBPM200.h"

#include <QQueue>

#include "BPMMessage.h"

class BPTruBPM200 : public QMainWindow
{
    Q_OBJECT

public:
    BPTruBPM200(QWidget *parent = Q_NULLPTR);
public slots:
    void OnStartClicked();
    void OnStopClicked();
    void OnClearClicked();
    void OnReviewClicked();
    void OnCycleClicked();
private:
    Ui::BPTruBPM200Class ui;
    void SetupSlots();
    QQueue<BPMMessage> writeQueue;
    QQueue<BPMMessage> readQueue;
};
