#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BPTruBPM200.h"

#include <QCloseEvent>

#include "hidapi.h"
#include "CRC8.h"
#include "BPMCommunicationHelper.h"
#include "BPMResponse.h"
#include "Wait.h"

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

    void OnBPMResponse();
private:
    Ui::BPTruBPM200Class ui;
    bool isLive = true;
    void closeEvent(QCloseEvent* event);
    void SetupSlots();
    BPMCommunicationHelper bpmCommunication;
};
