#include "BPTruBPM200.h"
#include "CRC8.h"

BPTruBPM200::BPTruBPM200(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    SetupSlots();
}

void BPTruBPM200::SetupSlots()
{
    connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(OnStartClicked()));
    connect(ui.StopButton, SIGNAL(clicked()), this, SLOT(OnStopClicked()));
    connect(ui.ClearButton, SIGNAL(clicked()), this, SLOT(OnClearClicked()));
    connect(ui.ReviewButton, SIGNAL(clicked()), this, SLOT(OnReviewClicked()));
    connect(ui.CycleButton, SIGNAL(clicked()), this, SLOT(OnCycleClicked()));
}

void BPTruBPM200::OnStartClicked() {
    ui.ErrorsTextBrowser->setText("Start Pressed");
}

void BPTruBPM200::OnStopClicked() {
    ui.ErrorsTextBrowser->setText("Stop Pressed");
}

void BPTruBPM200::OnClearClicked() {
    ui.ErrorsTextBrowser->setText("Clear Pressed");
}

void BPTruBPM200::OnReviewClicked() {
    ui.ErrorsTextBrowser->setText("Review Pressed");
}

void BPTruBPM200::OnCycleClicked() {
    ui.ErrorsTextBrowser->setText("Cycle Pressed");
}
