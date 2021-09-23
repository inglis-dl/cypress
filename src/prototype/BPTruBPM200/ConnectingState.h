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
#include "Data.h"
#include "Notification.h"
#include "NotificationEnum.h"

#include <QString>

class ConnectingState : public IState
{
public:
	ConnectingState(UiHelper* helper, BPMCommunicationHelper* communicationHelper);

	// Inherited via IState
	StateEnum GetState() override;
	// Inherited via IState
	StateEnum ProcessMessage(BPMMessage* msg) override;
private:
	StateEnum OnAck(BPMMessage* msg);
	void OnNotification(BPMMessage* msg);
	UiHelper* uiHelper;
	BPMCommunicationHelper* commHelper;
	void LogUnexpectedResponse(QString responseType, QString category);
};

