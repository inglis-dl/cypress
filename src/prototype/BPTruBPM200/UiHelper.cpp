#include "UiHelper.h"

UiHelper::UiHelper(Ui::BPTruBPM200Class* ui)
	: bpm200Ui(ui)
{
}

void UiHelper::UpdateStateText(QString text)
{
	bpm200Ui->StateVal->setText(text);
}

void UiHelper::UpdateReadingText(int reading)
{
	currentReading = reading;
	bpm200Ui->ReadingVal->setText(QString::number(reading));
}

void UiHelper::IncrementReading()
{
	UpdateReadingText(currentReading + 1);
}

void UiHelper::UpdateCycleText(int cycle)
{
	cycleVal = cycle;
	bpm200Ui->CycleVal->setText(QString::number(cycle));
}

void UiHelper::UpdateFirmwareText(QString text)
{
	bpm200Ui->FirmwareVal->setText(text);
}

void UiHelper::UpdateCuffPressure(int pressure, bool inflating)
{
	bpm200Ui->PressureVal->setText(QString::number(pressure) + (inflating ? " (+)" : " (-)"));
}

void UiHelper::UpdateCuffPressure(int pressure)
{
	bpm200Ui->PressureVal->setText(QString::number(pressure));
}

void UiHelper::ClearResults()
{
}

void UiHelper::AddResult(long startTime, long EndTime, int systolic, int diastolic, int pulse)
{
}

void UiHelper::AddAverage(int systolic, int diastolic, int pulse)
{
}
