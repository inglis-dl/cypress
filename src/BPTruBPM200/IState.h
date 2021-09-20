#pragma once

#include "StateEnum.h"
#include "BPMMessage.h"

class IState {
public:
	virtual StateEnum GetState() = 0;
	virtual StateEnum ProcessMessage(BPMMessage* msg) = 0;
};