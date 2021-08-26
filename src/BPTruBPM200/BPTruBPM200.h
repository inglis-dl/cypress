#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BPTruBPM200.h"

#include <QCloseEvent>

#include "hidapi.h"
#include "CRC8.h"
#include "BPMCommunicationHelper.h"

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
    void closeEvent(QCloseEvent* event);
    int testHidApi();
    void SetupSlots();
    BPMCommunicationHelper bpmCommunication;
};
