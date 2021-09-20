#pragma once
#include "ui_BPTruBPM200.h"

class UiHelper
{
public:
	UiHelper(Ui::BPTruBPM200Class* ui);
	void UpdateStateText(QString text);
	void UpdateReadingText(int reading);
	void IncrementReading();
	void UpdateCycleText(int cycle);
	void UpdateFirmwareText(QString text);
	void UpdateCuffPressure(int pressure, bool inflating);
	void UpdateCuffPressure(int pressure);

	// TODO: Need to finihs
	void ClearResults();
	void AddResult(long startTime, long EndTime, int systolic, int diastolic, int pulse);
	void AddAverage(int systolic, int diastolic, int pulse);

	int GetCycleVal() { return cycleVal; }
private:
	Ui::BPTruBPM200Class* bpm200Ui;
	int currentReading = 0;
	int cycleVal = 0;
};

