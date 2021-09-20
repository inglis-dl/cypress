#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BPTruBPM200.h"

#include <QCloseEvent>

#include "hidapi.h"
#include "CRC8.h"
#include "BPMCommunicationHelper.h"
#include "Wait.h"
#include "UiHelper.h"
#include "StateMachine.h"
#include "StateEnum.h"
#include "MeasuringState.h"

class BPTruBPM200 : public QMainWindow
{
    Q_OBJECT

public:
    BPTruBPM200(QWidget *parent = Q_NULLPTR);
public slots:
    void OnStartClicked();
    void OnAddClicked();
    void OnStopClicked();
    void OnExitClicked();

    void OnBPMResponse();
private:
    Ui::BPTruBPM200Class ui;
    UiHelper uiHelper; // ORDER DEPENDENCY
    //StateMachine* stateMachine; // ORDER DEPENDENCY
    bool isLive = true;
    BPMCommunicationHelper bpmCommunication;
    StateEnum stateEnum = StateEnum::Uninitialized;

    void closeEvent(QCloseEvent* event);
    void SetupSlots();
    void SetState(StateEnum newState);

    void UseConnectingState(BPMMessage* bpmMessage);
    void UseReadyState(BPMMessage* bpmMessage);
    void UseMeasuringState(BPMMessage* bpmMessage);
};
