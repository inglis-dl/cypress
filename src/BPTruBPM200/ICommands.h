#pragma once
class ICommands {
public:
	virtual void Reset() = 0;
	virtual void HandShake() = 0;
	virtual void NIBPStop() = 0;
	virtual void NIBPReview() = 0;
	virtual void NIBPCycle() = 0;
	virtual void NIBPStart() = 0;
	virtual void NIBPClear() = 0;
	virtual void DisablePressures() = 0;
	virtual void EnablePressures() = 0;
	virtual void RetrieveLastResult() = 0;
};