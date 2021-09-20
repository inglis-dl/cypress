#pragma once

#include "BPMMessage.h"
#include "BPTruBPM200.h"
#include "AckEnum.h"

class Ack
{
public:
	static QString GetPrintableType(BPMMessage* msg) {
		switch (msg->GetData0()) {
		case (int)AckEnum::HandShake:
			return "Handshake";
		case (int)AckEnum::Clear:
			return "Clear";
		case (int)AckEnum::Start:
			return "Start";
		case (int)AckEnum::Stop:
			return "Stop";
		case (int)AckEnum::Review:
			return "Review";
		case (int)AckEnum::Cycle:
			return "Cycle";
		default:
			return msg->GetFullMsg();
		}
	}

	static void Interpret(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		switch (msg->GetData0()) {
		case 0:
			Handshake(msg, bpm200);
			break;
		case 1:
			Stop(msg, bpm200);
			break;
		case 2:
			Review(msg, bpm200);
			break;
		case 3:
			Cycle(msg, bpm200);
			break;
		case 4:
			Start(msg, bpm200);
			break;
		case 5:
			Clear(msg, bpm200);
			break;
		}
	}
private:
	static void Handshake(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200)
	{
		int f1 = msg->GetData1();
		int f2 = msg->GetData2();
		int f3 = msg->GetData3();
		QString firmwareVersion = QString::number(f1) + "." + QString::number(f2) + QString::number(f3);
		qDebug() << "Handshake ack. Firmware version = " << firmwareVersion << endl;
	}

	static void Stop(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200)
	{
		qDebug() << "Stop ack "<< endl;
	}

	static void Review(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200)
	{
		qDebug() << "Review ack. Result code = " << msg->GetData3() << endl;
	}

	static void Cycle(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200)
	{
		int cycleTime = msg->GetData1();
		bpm200->CycleVal->setText(QString::number(cycleTime));
	}

	static void Start(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200)
	{
		int readingNumber = msg->GetData2();
		bpm200->ReadingVal->setText(QString::number(readingNumber));
	}

	static void Clear(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200)
	{
		qDebug() << "Clear ack " << endl;
	}
};

