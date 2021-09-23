#include "BPTruBPM200.h"

BPTruBPM200::BPTruBPM200(QWidget *parent)
    : QMainWindow(parent), uiHelper(UiHelper(&ui))//, stateMachine(new StateMachine(&uiHelper))
{
    ui.setupUi(this);
	bpmCommunication.StartBPMCommunication(isLive);
	SetupSlots();
    SetState(StateEnum::CONNECTING);
}

void BPTruBPM200::closeEvent(QCloseEvent* event) {
	bpmCommunication.StopBPMCommunication();
	event->accept();
}

void BPTruBPM200::SetupSlots()
{
    // Connect ui buttons
    connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(OnStartClicked()));
    connect(ui.AddButton, SIGNAL(clicked()), this, SLOT(OnAddClicked()));
    connect(ui.StopButton, SIGNAL(clicked()), this, SLOT(OnStopClicked()));
    connect(ui.ExitButton, SIGNAL(clicked()), this, SLOT(OnExitClicked()));

    // Connect the communication thread to notify this window when a new message is read
    connect(&bpmCommunication.cThread, SIGNAL(BPMResponseRecieved()), this, SLOT(OnBPMResponse()));
}

void BPTruBPM200::OnStartClicked() {
    if (stateEnum == StateEnum::READY) {
        // this will cause the bpm to start after finishing clearing
        bpmCommunication.Write(BPMCommands::NIBPClear());
    }
}

void BPTruBPM200::OnStopClicked() {
    if (stateEnum == StateEnum::MEASURING) {
        bpmCommunication.Write(BPMCommands::NIBPStop());
    }
}

void BPTruBPM200::OnAddClicked() {
    if (stateEnum == StateEnum::READY) {
        // this will set cycle to manual
        bpmCommunication.Write(BPMCommands::NIBPCycle());
    }
}

void BPTruBPM200::OnExitClicked() {
    // TODO: Exit app when clicked
}

void BPTruBPM200::OnBPMResponse()
{
    while (bpmCommunication.MessagesAvailable()) {
        BPMMessage message = bpmCommunication.Read();
        if (message.CheckCRCValid()) {
            switch (stateEnum) {
            case StateEnum::CONNECTING:
                UseConnectingState(&message);
                break;
            case StateEnum::READY:
                UseReadyState(&message);
                break;
            case StateEnum::MEASURING:
                UseMeasuringState(&message);
                break;
            }
        }
    }
}

void BPTruBPM200::SetState(StateEnum newState)
{
    if (stateEnum != newState) {
        stateEnum = newState;
        switch (newState) {
        case StateEnum::CONNECTING:
            uiHelper.UpdateStateText("Connecting");
            ui.StartButton->setEnabled(false);
            ui.AddButton->setEnabled(false);
            ui.StopButton->setEnabled(false);
            bpmCommunication.Write(BPMCommands::HandShake());
            break;
        case StateEnum::READY:
            uiHelper.UpdateStateText("Ready");
            ui.StartButton->setEnabled(true);
            ui.AddButton->setEnabled(true);
            ui.StopButton->setEnabled(false);
            break;
        case StateEnum::MEASURING:
            uiHelper.UpdateStateText("Measuring");
            ui.StartButton->setEnabled(false);
            ui.AddButton->setEnabled(false);
            ui.StopButton->setEnabled(true);
            break;
        }
    }
   
}

void BPTruBPM200::UseConnectingState(BPMMessage* bpmMessage)
{
    ConnectingState connectingState(&uiHelper, &bpmCommunication);
    SetState(connectingState.ProcessMessage(bpmMessage));
}

void BPTruBPM200::UseReadyState(BPMMessage* bpmMessage)
{
    ReadyState readyState(&uiHelper, &bpmCommunication);
    SetState(readyState.ProcessMessage(bpmMessage));
}

void BPTruBPM200::UseMeasuringState(BPMMessage* bpmMessage)
{
    MeasuringState measuringState(&uiHelper, &bpmCommunication);
    SetState(measuringState.ProcessMessage(bpmMessage));
}
