#pragma once

#include "BPMMessage.h"
#include "BPTruBPM200.h"
#include "Ack.h"
#include "Buttons.h"
#include "Data.h"
#include "Notification.h"

class BPMResponse
{
public:
	static void Interpret(BPMMessage* msg, Ui::BPTruBPM200Class* bpm200) {
		if (msg->CheckCRCValid()) {
			switch (msg->GetMsgId()) {
			// Ack
			case 6:
				Ack::Interpret(msg, bpm200);
				break;
			// Button
			case 85:
				Buttons::Interpret(msg, bpm200);
				break;
			// Notification
			case 33:
				Notification::Interpret(msg, bpm200);
				break;
			// Data
			case 65:// Bp Average
			case 66:// Bp Result
			case 68:// Deflating Cuff Pressure
			case 73:// Inflating Cuff Pressure
			case 76:// Review
				Data::Interpret(msg, bpm200);
				break;
			}
		}
	}
};

