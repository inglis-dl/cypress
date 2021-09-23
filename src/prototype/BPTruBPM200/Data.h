#pragma once

#include "BPMMessage.h"
#include "BPTruBPM200.h"
#include "DataEnum.h"

class Data
{
public:
	static QString GetPrintableType(BPMMessage* msg) {
		switch (msg->GetMsgId()) {
		case (int)DataEnum::BPAverage:
			return "BPAverage";
		case (int)DataEnum::BPResult:
			return "BPResult";
		case (int)DataEnum::DeflatingCuffPressure:
			return "DeflatingCuffPressure";
		case (int)DataEnum::InflatingCuffPressure:
			return "InflatingCuffPressure";
		case (int)DataEnum::Review:
			return "Review";
		default:
			return msg->GetFullMsg();
		}
	}

	static void Interpret(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		switch (msg->GetMsgId()) {
		case 65:// Bp Average
			BPAverage(msg, bpm200, false);
			break;
		case 66:// Bp Result
			BPResult(msg, bpm200, false);
			break;
		case 68:// Deflating Cuff Pressure
			CuffPressure(msg, bpm200, false);
			break;
		case 73:// Inflating Cuff Pressure
			CuffPressure(msg, bpm200, true);
			break;
		case 76: // Review
			Review(msg, bpm200);
			break;
		}
	}

	static void CuffPressure(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200, bool inflating) {
		int pressure = msg->GetData0() + msg->GetData1();
		bpm200->PressureVal->setText(QString::number(pressure) + (inflating?" (+)":" (-)"));
	}

	static void BPAverage(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200, bool isReview) {
		int sbp = msg->GetData1();
		//bpm200->AvgSystolicVal->setText(QString::number(sbp));
		int dbp = msg->GetData2();
		//bpm200->AvgDiastolicVal->setText(QString::number(dbp));

		int pulse = msg->GetData3();
		/*if (isReview) {
			bpm200->AvgPulseVal->setText(QString::number(pulse) + " (R)");
		}
		else {
			int count = msg->GetData0();
			bpm200->AvgPulseVal->setText(QString::number(pulse) + " (" + QString::number(count) + ")");
		}*/
	}

	static void BPResult(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200, bool isReview) {
		if (isReview == false) {
			int errorCode = msg->GetData0();
			qDebug() << "Error Code: " << errorCode << endl;
		}

		int sbp = msg->GetData1();
		//bpm200->SystolicVal->setText(QString::number(sbp));
		int dbp = msg->GetData2();
		//bpm200->DiastolicVal->setText(QString::number(dbp));
		int pulse = msg->GetData3();
		//bpm200->PulseVal->setText(QString::number(pulse));
	}

	static void Review(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		if (msg->GetData0() == 0) {
			BPAverage(msg, bpm200, true);
		}
		else {
			BPResult(msg, bpm200, true);
		}
	}
};

