#pragma once

#include "BPMMessage.h"
#include "BPTruBPM200.h"

class Buttons
{
public:
	static void Interpret(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		switch (msg->GetData0()) {
		case 1:
			Stopped(msg, bpm200);
			break;
		case 2:
			Review(msg, bpm200);
			break;
		case 3:
			Cycled(msg, bpm200);
			break;
		case 4:
			Started(msg, bpm200);
			break;
		case 5:
			Cleared(msg, bpm200);
			break;
		}
	}
private:
	static void Stopped(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		qDebug() << "Stop button clicked" << endl;
	}
	static void Review(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		qDebug() << "Review button clicked. Result code = " << msg->GetData3() << endl;
	}
	static void Cycled(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		int cycleTime = msg->GetData1();
		bpm200->CycleVal->setText(QString::number(cycleTime));
		qDebug() << "Cycle button clicked" << endl;
	}
	static void Started(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		int readingNumber = msg->GetData2();
		bpm200->ReadingVal->setText(QString::number(readingNumber));
		qDebug() << "Start button clicked" << endl;
	}
	static void Cleared(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		qDebug() << "Clear button clicked" << endl;
	}
};


