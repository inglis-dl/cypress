#pragma once
#include "StateEnum.h"
#include "IState.h"
#include "ConnectingState.h"
#include "UiHelper.h"
#include "ReadyState.h"

class StateMachine
{
public:
	//StateMachine(UiHelper* helper);

	//void Transition(StateEnum stateEnum);
private:
	UiHelper* uiHelper;
	IState* state;
};

