#pragma once

#include "IState.h"
#include "StateEnum.h"
#include "BPMMessage.h"
#include "BPMCommunicationHelper.h"
//#include "BPMCommands.h"
#include "UiHelper.h"
#include "Logger.h"

#include "ResponseTypeEnum.h"

#include "Ack.h"
#include "AckEnum.h"
#include "Buttons.h"
#include "ButtonsEnum.h"
#include "Data.h"
#include "DataEnum.h"
#include "Notification.h"
#include "NotificationEnum.h"

#include <QString>
#include <QDateTime>

class MeasuringState : public IState
{
public:
	MeasuringState(UiHelper* helper, BPMCommunicationHelper* communicationHelper);

	// Inherited via IState
	StateEnum GetState() override;
	// Inherited via IState
	StateEnum ProcessMessage(BPMMessage* msg) override;
private:
	StateEnum OnAck(BPMMessage* msg);
	StateEnum OnNotification(BPMMessage* msg);
	StateEnum OnButton(BPMMessage* msg);
	StateEnum OnData(BPMMessage* msg);
	long startTime = 0;
	void EndReading(int systolic, int diastolic, int pulse);
	void EndMeasures();
	UiHelper* uiHelper;
	BPMCommunicationHelper* commHelper;
	void LogUnexpectedResponse(QString responseType, QString category);
	bool complete = false;
};

