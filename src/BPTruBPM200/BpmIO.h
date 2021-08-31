#pragma once

#include "BPMMessage.h"
#include "Wait.h"
#include "hidapi.h"

#include <QList>

class BpmIO
{
public:
	void Setup(bool mIsLive);
	void Finish();
	QList<BPMMessage> Read();
	void Write(BPMMessage message);
private: 
	bool isLive;
	QList<BPMMessage> LiveRead();
	QList<BPMMessage> TestRead();
	void LiveWrite(BPMMessage message);
	void TestWrite(BPMMessage message);
	hid_device* bpm;
};

