#pragma once
#include "BPMMessage.h"

class IBPMCommands {
public:
	virtual BPMMessage Reset() = 0;
	virtual BPMMessage HandShake() = 0;
	virtual BPMMessage NIBPStop() = 0;
	virtual BPMMessage NIBPReview() = 0;
	virtual BPMMessage NIBPCycle() = 0;
	virtual BPMMessage NIBPStart() = 0;
	virtual BPMMessage NIBPClear() = 0;
	virtual BPMMessage DisablePressures() = 0;
	virtual BPMMessage EnablePressures() = 0;
	virtual BPMMessage RetrieveLastResult() = 0;
};