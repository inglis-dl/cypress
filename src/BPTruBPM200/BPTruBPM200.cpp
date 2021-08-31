#include "BPTruBPM200.h"

BPTruBPM200::BPTruBPM200(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	bpmCommunication.StartBPMCommunication(isLive);
	SetupSlots();
}

void BPTruBPM200::closeEvent(QCloseEvent* event) {
	bpmCommunication.StopBPMCommunication();
	event->accept();
}

void BPTruBPM200::SetupSlots()
{
    // Connect ui buttons
    connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(OnStartClicked()));
    connect(ui.StopButton, SIGNAL(clicked()), this, SLOT(OnStopClicked()));
    connect(ui.ClearButton, SIGNAL(clicked()), this, SLOT(OnClearClicked()));
    connect(ui.ReviewButton, SIGNAL(clicked()), this, SLOT(OnReviewClicked()));
    connect(ui.CycleButton, SIGNAL(clicked()), this, SLOT(OnCycleClicked()));

    // Connect the communication thread to notify this window when a new message is read
    connect(&bpmCommunication.cThread, SIGNAL(BPMResponseRecieved()), this, SLOT(OnBPMResponse()));
}

void BPTruBPM200::OnStartClicked() {
    ui.ErrorsTextBrowser->setText("Start Pressed");
	bpmCommunication.Write(BPMCommands::NIBPStart());
}

void BPTruBPM200::OnStopClicked() {
    ui.ErrorsTextBrowser->setText("Stop Pressed");
	bpmCommunication.Write(BPMCommands::NIBPStop());
}

void BPTruBPM200::OnClearClicked() {
    ui.ErrorsTextBrowser->setText("Clear Pressed");
	bpmCommunication.Write(BPMCommands::NIBPClear());
}

void BPTruBPM200::OnReviewClicked() {
    ui.ErrorsTextBrowser->setText("Review Pressed");
	bpmCommunication.Write(BPMCommands::NIBPReview());
}

void BPTruBPM200::OnCycleClicked() {
    ui.ErrorsTextBrowser->setText("Cycle Pressed");
	bpmCommunication.Write(BPMCommands::NIBPCycle());
}

void BPTruBPM200::OnBPMResponse()
{
    while (bpmCommunication.MessagesAvailable()) {
        BPMMessage message = bpmCommunication.Read();
        BPMResponse::Interpret(&message, &ui);
    }
}
